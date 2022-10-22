/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

/**
 * Device driver for HC-SR04 ultrasound module
 */

#include <stdio.h>
#include <inttypes.h>

#include <arch/soc.h>

#include "hcsr04.h"

//values for state
#define ST_IDLE 0
#define ST_PULSE_SENT 1
#define HCSR04_PULSE_TIMEOUT 1000000UL

static uint8_t hcsr04_send_pulse(struct hcsr04 *self)
{
	pio_write_pin(self->gpio, self->trigger_pin, 1);
	delay_us(10);
	pio_write_pin(self->gpio, self->trigger_pin, 0);

	self->state = ST_PULSE_SENT;
	self->pulse_timeout = timestamp_from_now_us(HCSR04_PULSE_TIMEOUT);

	return 1;
}

void hcsr04_init(struct hcsr04 *self, pio_dev_t gpio, gpio_pin_t trigger_pin, gpio_pin_t echo_pin)
{
	self->gpio = gpio;
	self->trigger_pin = trigger_pin;
	self->echo_pin = echo_pin;

	pio_configure_pin(self->gpio, self->trigger_pin, GP_OUTPUT | GP_PULLUP);
	pio_configure_pin(self->gpio, self->echo_pin, GP_INPUT | GP_PCINT);

	self->state = ST_IDLE;
	self->distance = -1;

	hcsr04_send_pulse(self);
}

static uint8_t hcsr04_check_response(struct hcsr04 *self)
{
	timestamp_t t_up, t_down;
	uint8_t status = pio_get_pin_status(self->gpio, self->echo_pin, &t_up, &t_down);
	if (status == GP_WENT_LOW) {
		timestamp_t us = timestamp_ticks_to_us(t_down - t_up);
		// convert to cm
		self->distance = (uint16_t)(us * 0.000001 * 34029.0f);
		self->state = ST_IDLE;
		return 1;
	}
	return 0;
}

int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self)
{
	if (hcsr04_check_response(self) || self->state == ST_IDLE ||
	    timestamp_expired(self->pulse_timeout)) {
		hcsr04_send_pulse(self);
	}
	return self->distance;
}
