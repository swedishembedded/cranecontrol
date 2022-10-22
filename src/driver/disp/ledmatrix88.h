/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#ifndef LEDMATRIX88_H_
#define LEDMATRIX88_H_

#include <arch/interface.h>

struct ledmatrix88 {
	pio_dev_t port;
	uint8_t x_bank, y_bank;
	uint8_t data[8]; // pixel data
	uint8_t cur_x, cur_y; // for updates
};

//functions
extern void ledmatrix88_init(struct ledmatrix88 *self, pio_dev_t port, uint8_t x_bank,
			     uint8_t y_bank);
extern void ledmatrix88_write_row(struct ledmatrix88 *self, uint8_t row, uint8_t data);
extern uint8_t ledmatrix88_read_row(struct ledmatrix88 *self, uint8_t row);
extern void ledmatrix88_clear(struct ledmatrix88 *self);
extern void ledmatrix88_update(struct ledmatrix88 *self);

#endif
