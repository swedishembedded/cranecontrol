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

#ifndef HCSR04_H
#define HCSR04_H

struct hcsr04 {
	pio_dev_t gpio;
	gpio_pin_t trigger_pin, echo_pin;
	uint8_t state;
	timestamp_t pulse_timeout;
	int16_t distance;
};

void hcsr04_init(struct hcsr04 *self, pio_dev_t gpio, gpio_pin_t trigger_pin, gpio_pin_t echo_pin);
int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self);

#endif
