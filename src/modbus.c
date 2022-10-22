/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2019-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#include "bus/modbus.h"
#include "driver.h"
#include "serial.h"

#define MODBUS_FC_READ_COILS 0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS 0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS 0x03
#define MODBUS_FC_READ_INPUT_REGISTERS 0x04
#define MODBUS_FC_WRITE_SINGLE_COIL 0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS 0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS 0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS 0x10
#define MODBUS_FC_READ_SLAVE_ID 0x11
#define MODBUS_FC_MASK_WRITE_REGISTER 0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS 0x17

#define MODBUS_BROADCAST_ADDRESS 0

struct modbus_server {
	uint8_t addr;
	struct regmap *regmap;
	serial_device_t serial;
	uint8_t slave_id;
	uint8_t rx_data[16];
	uint8_t tx_data[16];
};

struct modbus_client {
	serial_device_t serial;
};

/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40
};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
	0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
	0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
	0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
	0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
	0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
	0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
	0x40
};

static uint16_t _modbus_crc16(uint8_t *buffer, uint16_t buffer_length)
{
	uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
	uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
	unsigned int i; /* will index into CRC lookup */

	/* pass through message buffer */
	while (buffer_length--) {
		i = crc_hi ^ *buffer++; /* calculate the CRC  */
		crc_hi = (uint8_t)(crc_lo ^ table_crc_hi[i]);
		crc_lo = (uint8_t)(table_crc_lo[i]);
	}

	return (uint16_t)(crc_hi << 8 | crc_lo);
}
/*
static void _modbus_response_u16(struct modbus_server *self, uint8_t function,
                                 uint16_t value) {
  uint8_t buf[16];
  unsigned c = 0;

  buf[c++] = self->addr;
  buf[c++] = function;
  buf[c++] = (uint8_t)(value >> 8);
  buf[c++] = (uint8_t)(value & 0x00ff);

  uint16_t crc = _modbus_crc16(buf, 4);
  buf[c++] = (uint8_t)(crc >> 8);
  buf[c++] = (uint8_t)(crc & 0x00FF);

  serial_write(self->serial, buf, c, 100);
}
*/
void modbus_server_set_serial(struct modbus_server *self, serial_device_t dev)
{
	self->serial = dev;
}

void modbus_server_set_regmap(struct modbus_server *self, struct regmap *regs)
{
	self->regmap = regs;
}

void modbus_server_set_address(struct modbus_server *self, uint8_t addr)
{
	self->addr = addr;
}

int modbus_server_do_request(struct modbus_server *self, const uint8_t *rx, unsigned rx_size,
			     uint8_t *tx, unsigned tx_max_size)
{
	printk("modbus: req %02x %02x\n", rx[0], rx[1]);
	if (rx[0] != self->addr) {
		return -EINVAL;
	}
	switch (rx[1]) {
	case MODBUS_FC_READ_INPUT_REGISTERS:
	case MODBUS_FC_READ_HOLDING_REGISTERS: {
		uint16_t start = (uint16_t)(rx[2] << 8 | rx[3]);
		// uint16_t num = rx[4] << 8 | rx[5];
		uint32_t val = 0;
		int txi = 0;
		if (regmap_read_u32(self->regmap, start, &val) >= 0) {
			tx[txi++] = self->addr;
			tx[txi++] = rx[1];
			tx[txi++] = 2;
			tx[txi++] = (val >> 8) & 0xff;
			tx[txi++] = val & 0xff;
			uint16_t crc = _modbus_crc16(tx, 5);
			tx[txi++] = (uint8_t)(crc >> 8);
			tx[txi++] = (uint8_t)(crc & 0x00FF);
			// crc
			return txi;
		}
		return -1;
	} break;
	case MODBUS_FC_WRITE_SINGLE_REGISTER: {
		uint16_t start = (uint16_t)(rx[2] << 8 | rx[3]);
		uint16_t val = (uint16_t)(rx[4] << 8 | rx[5]);
		int txi = 0;
		if (regmap_write_u32(self->regmap, start, val) >= 0) {
			tx[txi++] = self->addr;
			tx[txi++] = rx[1];
			tx[txi++] = rx[2];
			tx[txi++] = rx[3];
			tx[txi++] = rx[4];
			tx[txi++] = rx[5];
			uint16_t crc = _modbus_crc16(tx, 6);
			tx[txi++] = (uint8_t)(crc >> 8);
			tx[txi++] = (uint8_t)(crc & 0x00FF);
			// crc
			return txi;
		}
		return -1;
	} break;
	case MODBUS_FC_READ_SLAVE_ID: {
		int txi = 0;
		tx[txi++] = self->addr;
		tx[txi++] = rx[1];
		tx[txi++] = 1;
		tx[txi++] = self->slave_id;
		uint16_t crc = _modbus_crc16(tx, 4);
		tx[txi++] = (uint8_t)(crc >> 8);
		tx[txi++] = (uint8_t)(crc & 0xff);
		return txi;
	} break;
	}
	return 0;
}

#include <stdio.h>
static void _modbus_server_task(void *ptr)
{
	struct modbus_server *self = (struct modbus_server *)ptr;
	while (1) {
		if (self->serial) {
			int res = serial_read(self->serial, self->rx_data, sizeof(self->rx_data),
					      100000);
			if (res > 0) {
				if ((res = modbus_server_do_request(self, self->rx_data,
								    (unsigned)res, self->tx_data,
								    sizeof(self->tx_data))) > 0) {
					serial_write(self->serial, self->tx_data, (size_t)res, 100);
				} else {
					// error
				}
			}
		}
		thread_sleep_ms(1);
	}
}

struct modbus_server *modbus_server_new()
{
	struct modbus_server *self = kzmalloc(sizeof(struct modbus_server));

	self->addr = 1;
	self->serial = NULL;

	if (thread_create(_modbus_server_task, "modbus", 550, self, 1, NULL) < 0) {
		dbg_printk("modbus: fail!\n");
		return 0;
	} else {
		printk("modbus: started!\n");
	}
	return self;
}

void modbus_server_delete(struct modbus_server *self)
{
	kfree(self);
}

void modbus_server_set_slave_id(struct modbus_server *self, uint8_t id)
{
	self->slave_id = id;
}

int modbus_server_add_range(struct modbus_server *self, struct regmap_range *range)
{
	return -1;
}

struct modbus_client *modbus_client_new()
{
	return kzmalloc(sizeof(struct modbus_client));
}

void modbus_client_delete(struct modbus_client *self)
{
	kfree(self);
}

void modbus_client_set_serial(struct modbus_client *self, serial_device_t dev)
{
	self->serial = dev;
}

int modbus_client_read_u16(const struct modbus_client *self, uint8_t addr, uint16_t reg,
			   uint16_t *val)
{
	unsigned txi = 0;
	uint8_t tx[10];
	tx[txi++] = addr;
	tx[txi++] = MODBUS_FC_READ_HOLDING_REGISTERS;
	tx[txi++] = (uint8_t)(reg >> 8);
	tx[txi++] = (uint8_t)(reg & 0xff);
	tx[txi++] = 0;
	tx[txi++] = 1;
	uint16_t crc = _modbus_crc16(tx, 5);
	tx[txi++] = (uint8_t)(crc >> 8);
	tx[txi++] = (uint8_t)(crc & 0x00FF);
	serial_write(self->serial, tx, txi, 100);
	memset(tx, 0, sizeof(tx));
	int rx_size = serial_read(self->serial, tx, sizeof(tx), 100000);
	if (rx_size != 7)
		return -1;
	if (tx[0] == addr && tx[1] == MODBUS_FC_READ_HOLDING_REGISTERS) {
		*val = (uint16_t)((tx[3] << 8) | tx[4]);
		return 0;
	}
	return -EIO;
}

int modbus_client_read_slave_id(const struct modbus_client *self, uint8_t addr, uint8_t *id)
{
	unsigned txi = 0;
	uint8_t tx[10];
	tx[txi++] = addr;
	tx[txi++] = MODBUS_FC_READ_SLAVE_ID;
	uint16_t crc = _modbus_crc16(tx, 2);
	tx[txi++] = (uint8_t)(crc >> 8);
	tx[txi++] = (uint8_t)(crc & 0x00FF);
	serial_write(self->serial, tx, txi, 100000);
	memset(tx, 0, sizeof(tx));
	int rx_size = serial_read(self->serial, tx, sizeof(tx), 100000);
	if (rx_size >= 0 && tx[0] == addr && tx[1] == MODBUS_FC_READ_SLAVE_ID) {
		// uint8_t count = tx[2];
		*id = tx[3];
		return 0;
	}
	return -EIO;
}

int modbus_client_write_u16(const struct modbus_client *self, uint8_t addr, uint16_t reg,
			    uint16_t val)
{
	uint16_t txi = 0;
	uint8_t tx[10];
	tx[txi++] = addr;
	tx[txi++] = MODBUS_FC_WRITE_SINGLE_REGISTER;
	tx[txi++] = (uint8_t)((reg >> 8) & 0xff);
	tx[txi++] = (uint8_t)((reg)&0xff);
	tx[txi++] = (uint8_t)(val >> 8);
	tx[txi++] = (uint8_t)val;
	uint16_t crc = _modbus_crc16(tx, txi);
	tx[txi++] = (uint8_t)(crc >> 8);
	tx[txi++] = (uint8_t)(crc & 0x00FF);
	int sent = serial_write(self->serial, tx, txi, 100);
	serial_read(self->serial, tx, sizeof(tx), 100000);
	return sent;
}

static int _modbus_probe(void *fdt, int fdt_node)
{
	struct modbus_server *self = modbus_server_new();
	serial_device_t serial = serial_find_by_ref(fdt, fdt_node, "serial");
	regmap_device_t regmap = regmap_find_by_ref(fdt, fdt_node, "regmap");
	uint8_t addr = (uint8_t)fdt_get_int_or_default(fdt, fdt_node, "addr", 0);

	if (!serial) {
		printk("modbus: serial port missing\n");
		return -EINVAL;
	}

	modbus_server_set_serial(self, serial);
	modbus_server_set_regmap(self, regmap_device_to_regmap(regmap));
	modbus_server_set_address(self, addr);
	modbus_server_set_slave_id(self, 0xAB);

	printk("modbus: ready!\n");

	return 0;
}

static int _modbus_remove(void *fdt, int fdt_node)
{
	return 0;
}

DEVICE_DRIVER(modbus, "fw,modbus", _modbus_probe, _modbus_remove)
