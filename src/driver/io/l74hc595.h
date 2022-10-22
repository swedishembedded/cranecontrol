/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) 2011 Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#pragma once

//setup ports
#define L74HC595_DDR DDRB
#define L74HC595_PORT PORTB
#define L74HC595_STCPIN PB1
//#define L74HC595_CEPIN PB2
struct l74hc595 {
	serial_dev_t spi;
	pio_dev_t gpio;
	gpio_pin_t ce_pin;
	gpio_pin_t stc_pin;
};

void l74hc595_init(struct l74hc595 *self, serial_dev_t spi, pio_dev_t gpio, gpio_pin_t ce_pin,
		   gpio_pin_t stc_pin);
void l74hc595_write(struct l74hc595 *self, uint8_t val);
