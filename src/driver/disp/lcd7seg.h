/**
 * SPDX-License-Identifier: GPLv3
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

#ifndef SEVSEG_H_
#define SEVSEG_H_

#include <arch/interface.h>

//definitions
typedef enum {
	SEVSEG_TYPECC, //common cathode
	SEVSEG_TYPECA, //common anode type
	SEVSEG_TYPECCT, //common cathode / full transistor type
	SEVSEG_TYPECAT //common anode type / full transistor type
} lcd7seg_type;

struct led7seg {
	pio_dev_t port;
	uint8_t type;
	uint8_t cs_pin;
	uint8_t data_port;
	uint8_t data;
};

//functions
void led7seg_init(struct led7seg *self, pio_dev_t port, uint8_t data_port, gpio_pin_t cs_pin,
		  lcd7seg_type type);
void led7seg_putc(struct led7seg *self, uint8_t c, uint8_t dot);

void led7seg_on(struct led7seg *self);
void led7seg_off(struct led7seg *self);

#endif
