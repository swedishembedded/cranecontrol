/*
 * Copyright (C) 2017 Martin K. Schröder <mkschreder.uk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

#ifndef __cplusplus
//#include <cstdint>
//#include <cmath>
#define min(x, y)                                                                                  \
	(__extension__({                                                                           \
		typeof(x) _x = (x);                                                                \
		typeof(x) _y = (y);                                                                \
		(void)(&_x == &_y);                                                                \
		(typeof(x))(_x < _y ? _x : _y);                                                    \
	}))

#define max(x, y)                                                                                  \
	({                                                                                         \
		typeof(x) _x = (x);                                                                \
		typeof(y) _y = (y);                                                                \
		(void)(&_x == &_y);                                                                \
		_x > _y ? _x : _y;                                                                 \
	})
#endif

#define signf(x) (float)(((float)(x) > 0) - ((float)(x) < 0))

#define FLOAT_EPSILON (1e-6f)

void clamp_rad360(float *angle);

static inline int16_t u16_diff(uint16_t a, uint16_t b)
{
	return (int16_t)((int16_t)a - (int16_t)b);
}

static inline int32_t u32_diff(uint32_t a, uint32_t b)
{
	return ((int32_t)a - (int32_t)b);
}

int32_t lowpass_i32(int32_t x, int32_t prev, uint16_t num, uint16_t denom);
uint32_t lowpass_phase32(uint32_t x, uint32_t prev, uint16_t num, uint16_t denom);
float lowpass_f32(float x, float prev, float frac_new);

uint8_t majority_filter(uint8_t emf, uint8_t last_known_sector, uint8_t prev_out);
