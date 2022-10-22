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

typedef enum { FB_PIXEL_FORMAT_1BIT, FB_PIXEL_FORMAT_RGB8, FB_PIXEL_FORMAT_RGB16 } fb_pixel_type_t;

typedef struct fb_image {
	uint16_t w;
	uint16_t h;
	uint8_t *data;
	fb_pixel_type_t format;
} fb_image_t;

typedef struct {
	uint8_t r, g, b, a;
} fb_color_t;

#define RGBA(r, g, b, a)                                                                           \
	(fb_color_t)                                                                               \
	{                                                                                          \
		r, g, b, a                                                                         \
	}

typedef void (*fb_fill)(fb_color_t color, size_t count);
typedef void (*fb_set_region)(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
typedef size_t (*fb_write)(const uint8_t *data, size_t sz, fb_pixel_type_t type_of_supplied_data);
typedef size_t (*fb_read)(uint8_t *data, size_t sz, fb_pixel_type_t encode_in_format);
typedef void (*fb_get_size)(uint16_t *width, uint16_t *height);

struct fb_device {
	fb_fill fill;
	fb_set_region set_region;
	fb_write write;
	fb_read read;
	fb_get_size get_size;
};

typedef struct tty_device **tty_dev_t;
typedef uint16_t color_t;

struct tty_device {
	void (*put)(tty_dev_t self, uint8_t ch, color_t fg, color_t bg);
	void (*move_cursor)(tty_dev_t self, uint16_t x, uint16_t y);

	//void (*draw_fill_rect)(tty_dev_t *disp, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color);

	void (*get_size)(tty_dev_t self, uint16_t *w, uint16_t *h);
	//void (*set_scroll_region)(tty_dev_t *disp, uint16_t start, uint16_t end);
	//void (*set_top_line)(tty_dev_t *disp, uint16_t idx);
	void (*clear)(tty_dev_t self);
};

static inline void tty_put(tty_dev_t self, uint8_t ch, color_t fg, color_t bg)
{
	(*self)->put(self, ch, fg, bg);
}

static inline void tty_move_cursor(tty_dev_t self, uint16_t x, uint16_t y)
{
	(*self)->move_cursor(self, x, y);
}

static inline void tty_get_size(tty_dev_t self, uint16_t *w, uint16_t *h)
{
	(*self)->get_size(self, w, h);
}
