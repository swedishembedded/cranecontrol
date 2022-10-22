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
#include <string.h>

#include <arch/soc.h>

#include "dht.h"

void dht_init(struct dht *self, pio_dev_t gpio, gpio_pin_t signal_pin, uint8_t sensor_type)
{
	self->gpio = gpio;
	self->signal_pin = signal_pin;

	if (sensor_type != DHT_DHT11 && sensor_type != DHT_DHT22)
		sensor_type = DHT_DHT11;

	self->sensor_type = sensor_type;
}

static int8_t dht_read_data(struct dht *self, int8_t *temperature, int8_t *humidity)
{
	uint8_t bits[5];

	memset(bits, 0, sizeof(bits));

	//reset port

	pio_configure_pin(self->gpio, self->signal_pin, GP_OUTPUT);
	pio_write_pin(self->gpio, self->signal_pin, 1);
	delay_us(100000L);

	// send request
	pio_write_pin(self->gpio, self->signal_pin, 0);
	if (self->sensor_type == DHT_DHT11)
		delay_us(18000L);
	else
		delay_us(500);

	pio_configure_pin(self->gpio, self->signal_pin, GP_INPUT | GP_PULLUP);
	delay_us(40);

#define read_pin() pio_read_pin(self->gpio, self->signal_pin)
	// start condition 1
	if (read_pin())
		return -1;
	delay_us(80);
	// start condition 2
	if (!read_pin())
		return -1;
	delay_us(80);

	//read the data
	uint16_t timeoutcounter = 0;
	for (int j = 0; j < 5; j++) { //read 5 byte
		uint8_t result = 0;
		for (int i = 0; i < 8; i++) { //read every bit
			timeoutcounter = 0;
			while (!read_pin()) { //wait for an high input (non blocking)
				timeoutcounter++;
				if (timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
			delay_us(30);
			if (read_pin()) //if input is high after 30 us, get result
				result |= (1 << (7 - i));
			timeoutcounter = 0;
			while (read_pin()) { //wait until input get low (blocking)
				timeoutcounter++;
				if (timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
		}
		bits[j] = result;
	}

	//reset port
	pio_configure_pin(self->gpio, self->signal_pin, GP_OUTPUT);
	pio_write_pin(self->gpio, self->signal_pin, 0);

	//check checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		//return temperature and humidity
		if (self->sensor_type == DHT_DHT11) {
			*temperature = bits[2];
			*humidity = bits[0];
		} else if (self->sensor_type == DHT_DHT22) {
			uint16_t rawhumidity = bits[0] << 8 | bits[1];
			uint16_t rawtemperature = bits[2] << 8 | bits[3];
			if (rawtemperature & 0x8000) {
				*temperature = (float)((rawtemperature & 0x7FFF) / 10.0) * -1.0;
			} else {
				*temperature = (float)(rawtemperature) / 10.0;
			}
			*humidity = (float)(rawhumidity) / 10.0;
		}
		return 0;
	}

	return -1;
}

int8_t dht_read(struct dht *self, int8_t *temperature, int8_t *humidity)
{
	return dht_read_data(self, temperature, humidity);
}
