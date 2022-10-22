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

#ifdef __cplusplus
extern "C" {
#endif

#include <disp/interface.h>

#define VT100_CHAR_WIDTH 6
#define VT100_CHAR_HEIGHT 8

#define VT100_MAX_COMMAND_ARGS 4

struct vt100 {
	union flags {
		uint8_t val;
		struct {
			// 0 = cursor remains on last column when it gets there
			// 1 = lines wrap after last column to next line
			uint8_t cursor_wrap : 1;
			uint8_t scroll_mode : 1;
			uint8_t origin_mode : 1;
		};
	} flags;

	tty_dev_t display;

	//uint16_t screen_width, screen_height;
	// cursor position on the screen (0, 0) = top left corner.
	uint16_t cursor_x, cursor_y;
	uint16_t saved_cursor_x, saved_cursor_y; // used for cursor save restore
	uint16_t scroll_start_row, scroll_end_row;
	// character width and height
	//uint8_t char_width, char_height;
	// colors used for rendering current characters
	uint16_t back_color, front_color;
	// the starting y-position of the screen scroll
	uint16_t scroll_value;
	// command arguments that get parsed as they appear in the terminal
	uint8_t narg;
	uint16_t args[VT100_MAX_COMMAND_ARGS];
	// current arg pointer (we use it for parsing)
	uint8_t carg;
	uint16_t screen_height, screen_width;

	void (*state)(struct vt100 *term, uint8_t ev, uint16_t arg);
	//void (*send_response)(char *str);
	void (*ret_state)(struct vt100 *term, uint8_t ev, uint16_t arg);

	struct serial_if *serial;
};

#include "../arch/interface.h"

//void vt100_init(void (*send_response)(char *str));
void vt100_init(struct vt100 *self, tty_dev_t display);
void vt100_putc(struct vt100 *self, uint8_t ch);
void vt100_puts(struct vt100 *self, const char *str);

serial_dev_t vt100_get_serial_interface(struct vt100 *self);

#ifdef __cplusplus
}
#endif
