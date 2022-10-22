/**
 * SPDX-License-Identifier: GPLv3
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#include <stdio.h>
#include <string.h>
#include <arch/soc.h>

#include "l74hc165.h"

/*
 * init the shift register
 */
void l74hc165_init(struct l74hc165 *self, pio_dev_t port, gpio_pin_t clock_pin, gpio_pin_t load_pin,
		   gpio_pin_t data_pin)
{
	self->port = port;
	self->clock_pin = clock_pin;
	self->load_pin = load_pin;
	self->data_pin = data_pin;

	pio_configure_pin(port, clock_pin, GP_OUTPUT);
	pio_configure_pin(port, load_pin, GP_OUTPUT);
	pio_configure_pin(port, data_pin, GP_INPUT | GP_PULLUP);
	pio_write_pin(port, clock_pin, 0);
	pio_write_pin(port, load_pin, 0);
}

/*
 * shift in data
 */
void l74hc165_read(struct l74hc165 *self, uint8_t *bytearray, uint8_t count)
{
	//parallel load to freeze the state of the data lines
	pio_write_pin(self->port, self->load_pin, 0);
	delay_us(5);
	pio_write_pin(self->port, self->load_pin, 1);
	for (uint8_t i = 0; i < count; i++) {
		//iterate through the bits in each registers
		uint8_t currentbyte = 0;
		for (uint8_t j = 0; j < 8; j++) {
			uint8_t data = (pio_read_pin(self->port, self->data_pin)) ? 1 : 0;
			currentbyte |= (data << (7 - j));
			//get next
			pio_write_pin(self->port, self->clock_pin, 0);
			delay_us(5);
			pio_write_pin(self->port, self->clock_pin, 1);
		}
		memcpy(&bytearray[i], &currentbyte, 1);
	}
}
