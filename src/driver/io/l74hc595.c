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

#include <arch/soc.h>

#include "l74hc595.h"

#ifndef CONFIG_L74HC595_STC_PIN
#define CONFIG_L74HC595_STC_PIN GPIO_NONE
#endif

#define spi_writereadbyte(b) (serial_putc(self->spi, b), serial_getc(self->spi))

#define L74HC595_CELo pio_write_pin(self->gpio, self->ce_pin, 0)
#define L74HC595_CEHi pio_write_pin(self->gpio, self->ce_pin, 1)

#define L74HC595_STCLo pio_write_pin(self->gpio, self->stc_pin, 0)
#define L74HC595_STCHi pio_write_pin(self->gpio, self->stc_pin, 1)
/*
 * init the shift register
 */
void l74hc595_init(struct l74hc595 *self, serial_dev_t spi, pio_dev_t gpio, gpio_pin_t ce_pin,
		   gpio_pin_t stc_pin)
{
	self->spi = spi;
	self->gpio = gpio;
	self->stc_pin = stc_pin;
	self->ce_pin = ce_pin;

	pio_configure_pin(gpio, ce_pin, GP_OUTPUT);
	pio_configure_pin(gpio, stc_pin, GP_OUTPUT);

	L74HC595_STCLo;
}

void l74hc595_write(struct l74hc595 *self, uint8_t data)
{
	L74HC595_CEHi;
	spi_writereadbyte(data);
	L74HC595_STCHi;
	delay_us(1); // not needed but still for safety (16ns is minimum high period)
	L74HC595_STCLo;
	L74HC595_CELo;
}
