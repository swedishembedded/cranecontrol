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
#define ADXL345_ADDR (0x53 << 1) //device address

#include "../i2c/i2c.h"

struct adxl345 {
	i2c_dev_t i2c;
	uint8_t addr;
};

void adxl345_init(struct adxl345 *self, i2c_dev_t i2c, uint8_t addr);
int8_t adxl345_read_raw(struct adxl345 *self, int16_t *ax, int16_t *ay, int16_t *az);
int8_t adxl345_read_adjusted(struct adxl345 *self, float *ax, float *ay, float *az);
