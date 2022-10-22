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

#pragma once

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_128_64
// #define SSD1306_128_32

typedef struct ssd1306 {
	i2c_dev_t i2c;
	uint8_t r_row_start, r_row_end;
} ssd1306_t;

void ssd1306_init(ssd1306_t *dev, i2c_dev_t i2c);
void ssd1306_set_region(ssd1306_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ssd1306_fill(ssd1306_t *dev, fb_color_t color, size_t len);
void ssd1306_draw(ssd1306_t *dev, uint16_t x, uint16_t y, fb_image_t img);
void ssd1306_gotoChar(ssd1306_t *dev, uint8_t x, uint8_t y);
void ssd1306_clear(ssd1306_t *dev);
int16_t ssd1306_puts(ssd1306_t *dev, const char *str, uint8_t col);
int16_t ssd1306_printf(ssd1306_t *dev, uint8_t col, const char *str, ...);
void ssd1306_reset(ssd1306_t *dev);

#ifdef __cplusplus
}
#endif
