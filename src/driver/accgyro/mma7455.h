/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015, Martin K. Schröder, All Rights Reserved
 * Copyright (c) Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/
#ifndef MMA7455_H_
#define MMA7455_H_

#include <libfirmware/i2c.h>

//definitions
#define MMA7455_ADDR (0x1D << 1) //device address

struct mma7455 {
	i2c_device_t i2c;
	uint8_t addr;
	uint8_t mode;
};

//functions declarations
void mma7455_init(struct mma7455 *self, i2c_device_t i2c, uint8_t addr, uint8_t mode);
void mma7455_getdata(struct mma7455 *self, float *ax, float *ay, float *az);
#if MMA7455_GETATTITUDE == 1
void mma7455_getpitchroll(struct mma7455 *self, float ax, float ay, float az, float *pitch,
			  float *roll);
#endif

#endif
