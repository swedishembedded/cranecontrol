#pragma once

#include "../serial.h"
#include "../regmap.h"

struct modbus_server;
struct modbus_client;

struct modbus_server *modbus_server_new();
void modbus_server_delete(struct modbus_server *self);
void modbus_server_set_serial(struct modbus_server *self, serial_device_t dev);
void modbus_server_set_slave_id(struct modbus_server *self, uint8_t id);
void modbus_server_set_regmap(struct modbus_server *self, struct regmap *regs);
void modbus_server_set_address(struct modbus_server *self, uint8_t addr);
int modbus_server_add_range(struct modbus_server *self, struct regmap_range *range);
int modbus_server_do_request(struct modbus_server *self, const uint8_t *rx, unsigned rx_size,
			     uint8_t *tx, unsigned tx_max_size);

struct modbus_client *modbus_client_new();
void modbus_client_delete(struct modbus_client *self);
int modbus_client_read_u16(const struct modbus_client *self, uint8_t addr, uint16_t reg,
			   uint16_t *val);
int modbus_client_write_u16(const struct modbus_client *self, uint8_t addr, uint16_t reg,
			    uint16_t val);
void modbus_client_set_serial(struct modbus_client *self, serial_device_t dev);
int modbus_client_read_slave_id(const struct modbus_client *self, uint8_t addr, uint8_t *id);
