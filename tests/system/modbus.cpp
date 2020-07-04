#include <stdio.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../mock/crc16.hpp"

extern "C" {
#include "bus/modbus.h"
#include "serial.h"
#include "regmap.h"
#include "linux/linux_uart.h"
#include "linux/linux_serial.h"
}

#define MODBUS_FC_READ_COILS                0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
#define MODBUS_FC_REPORT_SLAVE_ID           0x11
#define MODBUS_FC_MASK_WRITE_REGISTER       0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17

class ModbusTest : public ::testing::Test {
protected:
    virtual void SetUp() {

    }
};

static uint16_t _reg = 0;
ssize_t _regmap_read(regmap_range_t range, uint32_t id, regmap_value_type_t type, void *value, size_t size){
	printf("read: %02x\n", id);
	switch(id){
		case 0x10ff: {
			memcpy(value, &_reg, size);
			return size;
		}
	}
	return -EINVAL;
}

ssize_t _regmap_write(regmap_range_t range, uint32_t id, regmap_value_type_t type, const void *value, size_t size){
	printf("write: %02x\n", id);
	switch(id){
		case 0x10ff: {
			memcpy(&_reg, value, sizeof(_reg));
			return size;
		}
	}
	return -EINVAL;
}

TEST_F(ModbusTest, check_request){
	struct regmap *regmap = regmap_new();
	struct modbus_server *srv = modbus_server_new();

	modbus_server_set_regmap(srv, regmap);
	modbus_server_set_address(srv, 1);

	struct regmap_range rm;
	static struct regmap_range_ops ops = {
		.read = _regmap_read,
		.write = _regmap_write
	};

	regmap_range_init(&rm, 0x1000, 0x2000, &ops);

	EXPECT_EQ(regmap_add_range(regmap, &rm), 0);

	uint8_t data[16];
	uint8_t result[16];

	memset(data, 0xAA, sizeof(data));
	memset(result, 0xAA, sizeof(result));

	// make a read request
	data[0] = 1;
	data[1] = MODBUS_FC_READ_HOLDING_REGISTERS;
	data[2] = 0x10;
	data[3] = 0xff;
	data[4] = 0;
	data[5] = 1;
	uint16_t crc = crc16(0xffff, data, 6);
	data[6] = crc & 0xff;
	data[7] = crc >> 8;

	_reg = 0x1234;

	EXPECT_EQ(modbus_server_do_request(srv, data, sizeof(data), result, sizeof(result)), 7);
	EXPECT_EQ(result[0], 1); // server address
	EXPECT_EQ(result[1], MODBUS_FC_READ_HOLDING_REGISTERS); // function
	EXPECT_EQ(result[2], 2); // bytes of data
	EXPECT_EQ(result[3], 0x12); // hi
	EXPECT_EQ(result[4], 0x34); // lo
}

TEST_F(ModbusTest, check_read_slave_id){
	struct modbus_server *server = modbus_server_new();
	unsigned txi = 0;
	uint8_t tx[16];
	uint8_t rx[16];

	tx[txi++] = 0x05; // address
	tx[txi++] = 0x11; // read id
	uint16_t check = crc16(0xffff, tx, txi);
	tx[txi++] = (uint8_t)(check & 0xff);
	tx[txi++] = (uint8_t)(check >> 8);
	memset(rx, 0, sizeof(rx));

	modbus_server_set_address(server, 0x05);
	modbus_server_set_slave_id(server, 0xaa);
	EXPECT_EQ(modbus_server_do_request(server, tx, txi, rx, sizeof(rx)), 6); // return is return size or < 0

	EXPECT_EQ(rx[0], 0x05); // address
	EXPECT_EQ(rx[1], 0x11); // function
	EXPECT_EQ(rx[2], 0x1); // byte count
	EXPECT_EQ(rx[3], 0xaa); // slave id

	check = crc16(0xffff, rx, 4);
	EXPECT_EQ(check & 0xff, rx[4]);
	EXPECT_EQ(check >> 8, rx[5]);
}

TEST_F(ModbusTest, check_client){
	struct linux_uart *uart = linux_uart_new();
	struct linux_serial *serial = linux_serial_new();
	struct modbus_server *srv = modbus_server_new();
	struct modbus_client *cli = modbus_client_new();
	struct regmap *regmap = regmap_new();

	linux_serial_connect(serial, linux_uart_get_ptsname(uart), 115200);

	modbus_server_set_serial(srv, linux_uart_as_serial_device(uart));
	modbus_client_set_serial(cli, linux_serial_as_serial_device(serial));
	modbus_server_set_address(srv, 0x11);

	{
		uint8_t id = 0;
		modbus_server_set_slave_id(srv, 0xAB);
		EXPECT_EQ(modbus_client_read_slave_id(cli, 0x11, &id), 0);
		EXPECT_EQ(id, 0xAB);
	}

	// do it one more time to check that we can correctly handle multiple packets
	{
		uint8_t id = 0;
		modbus_server_set_slave_id(srv, 0x18);
		EXPECT_EQ(modbus_client_read_slave_id(cli, 0x11, &id), 0);
		EXPECT_EQ(id, 0x18);
	}

	modbus_server_set_address(srv, 0x01);
	struct regmap_range rm;
	static struct regmap_range_ops ops = {
		.read = _regmap_read,
		.write = _regmap_write
	};

	regmap_range_init(&rm, 0x1000, 0x2000, &ops);

	EXPECT_EQ(regmap_add_range(regmap, &rm), 0);
	modbus_server_set_regmap(srv, regmap);

	_reg = 0;
	EXPECT_EQ(modbus_client_write_u16(cli, 0x01, 0x10ff, 0x1234), 8);
	EXPECT_EQ(_reg, 0x1234);

	// now read it over modbus
	uint16_t val;
	EXPECT_EQ(modbus_client_read_u16(cli, 0x01, 0x10ff, &val), 0);

	EXPECT_EQ(val, 0x1234);
}

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
