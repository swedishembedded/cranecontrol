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

#include "bh1750.h"

//resolution modes
#define BH1750_MODEH 0x10 //continuously h-resolution mode, 1lx resolution, 120ms
#define BH1750_MODEH2 0x11 //continuously h-resolution mode, 0.5lx resolution, 120ms
#define BH1750_MODEL 0x13 //continuously l-resolution mode, 4x resolution, 16ms
//define active resolution mode
#define BH1750_MODE BH1750_MODEH

void bh1750_init(struct bh1750 *self, i2c_dev_t i2c, uint8_t addr)
{
	self->i2c = i2c;
	self->addr = addr;
	uint8_t mode = BH1750_MODE;
	i2c_start_write(i2c, addr, &mode, 1);
	i2c_stop(i2c);
}

uint16_t bh1750_read_intensity_lux(struct bh1750 *self)
{
	uint16_t ret = 0;

	i2c_start_read(self->i2c, self->addr, (uint8_t *)&ret, 2);
	i2c_stop(self->i2c);
	return ret / 1.2f;
}
