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

#ifndef L74HC165_H_
#define L74HC165_H_

struct l74hc165 {
	pio_dev_t port;
	gpio_pin_t clock_pin;
	gpio_pin_t load_pin;
	gpio_pin_t data_pin;
};

void l74hc165_init(struct l74hc165 *self, pio_dev_t port, gpio_pin_t clock_pin, gpio_pin_t load_pin,
		   gpio_pin_t data_pin);
void l74hc165_read(struct l74hc165 *self, uint8_t *data, uint8_t count);

#endif
