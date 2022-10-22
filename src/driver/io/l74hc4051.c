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

#include <stdio.h>
#include <inttypes.h>

#include <arch/soc.h>
#include "l74hc4051.h"

/*
 * init the shift register
 */
void l74hc4051_init(struct l74hc4051 *self, pio_dev_t port, gpio_pin_t s0_pin, gpio_pin_t s1_pin,
		    gpio_pin_t s2_pin)
{
	self->port = port;
	self->s0_pin = s0_pin;
	self->s1_pin = s1_pin;
	self->s2_pin = s2_pin;

	pio_configure_pin(port, s0_pin, GP_OUTPUT);
	pio_configure_pin(port, s1_pin, GP_OUTPUT);
	pio_configure_pin(port, s2_pin, GP_OUTPUT);
	pio_write_pin(port, s0_pin, 0);
	pio_write_pin(port, s1_pin, 0);
	pio_write_pin(port, s2_pin, 0);
}

void l74hc4051_set_channel(struct l74hc4051 *self, uint8_t channel)
{
	//bit 1
	if ((channel & (1 << 0)) >> 0)
		pio_write_pin(self->port, self->s0_pin, 1);
	else
		pio_write_pin(self->port, self->s0_pin, 0);
	//bit 2
	if ((channel & (1 << 1)) >> 1)
		pio_write_pin(self->port, self->s1_pin, 1);
	else
		pio_write_pin(self->port, self->s1_pin, 0);
	//bit 3
	if ((channel & (1 << 2)) >> 2)
		pio_write_pin(self->port, self->s2_pin, 1);
	else
		pio_write_pin(self->port, self->s2_pin, 0);
}
