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
#ifndef L3G4200D_H_
#define L3G4200D_H_

#ifdef __cplusplus
"C"
{
#endif

#include <libfirmware/i2c.h>

//definitions
#define L3G4200D_ADDR (0x69 << 1) //device address

	struct l3g4200d {
		i2c_device_t i2c;
		uint8_t addr;
		int8_t temperatureref;
		float offsetx, offsety, offsetz;
	};

	//functions
	void l3g4200d_init(struct l3g4200d * self, i2c_device_t i2c, uint8_t addr);
	void l3g4200d_setoffset(struct l3g4200d * self, float offsetx, float offsety,
				float offsetz);
	void l3g4200d_read_raw(struct l3g4200d * self, int16_t * gxraw, int16_t * gyraw,
			       int16_t * gzraw);
	void l3g4200d_read_converted(struct l3g4200d * self, float *gx, float *gy, float *gz);
	void l3g4200d_settemperatureref(struct l3g4200d * self);
	int8_t l3g4200d_gettemperaturediff(struct l3g4200d * self);

#ifdef __cplusplus
}
#endif

#endif
