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
/*
	Special thanks to:
	* this library is a porting of the bmp085driver 0.4 ardunio library
    http://code.google.com/p/bmp085driver/
	* Davide Gironi, original implementation
*/

#ifndef BMP085_H_
#define BMP085_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMP085_ADDR (0x77 << 1) //0x77 default I2C address

struct bmp085 {
	i2c_dev_t i2c;
	uint8_t addr;
	int regac1, regac2, regac3, regb1, regb2, regmb, regmc, regmd;
	unsigned int regac4, regac5, regac6;
};

//functions
/// inits the device over the interface supplied
void bmp085_init(struct bmp085 *self, i2c_dev_t i2c, uint8_t addr);
/// returns pressure
long bmp085_read_pressure(struct bmp085 *self);
/// returns altitude
float bmp085_read_altitude(struct bmp085 *self);
/// returns temperature
float bmp085_read_temperature(struct bmp085 *self);

#ifdef __cplusplus
}
#endif
#endif
