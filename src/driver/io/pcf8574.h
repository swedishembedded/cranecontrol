/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) 2012 Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#ifndef PCF8574_H_
#define PCF8574_H_

struct pcf8574 {
	i2c_dev_t i2c;
	uint8_t device_id;
	uint8_t in_reg, out_reg;
};

void pcf8574_init(struct pcf8574 *self, i2c_dev_t i2c, uint8_t device_id);

uint8_t pcf8574_write_word(struct pcf8574 *self, uint8_t data);
uint8_t pcf8574_write_pin(struct pcf8574 *self, uint8_t pin, uint8_t value);
uint8_t pcf8574_read_word(struct pcf8574 *self);
uint8_t pcf8574_read_pin(struct pcf8574 *self, uint8_t pin);

//extern int8_t pcf8574_getoutput(uint8_t deviceid);
//extern int8_t pcf8574_getoutputpin(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpinhigh(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpinlow(uint8_t deviceid, uint8_t pin);
//extern int8_t pcf8574_setoutputpins(uint8_t deviceid, uint8_t pinstart, uint8_t pinlength, int8_t data);

#endif
