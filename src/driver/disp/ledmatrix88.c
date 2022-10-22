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

#include <stdio.h>
#include <string.h>

#include "ledmatrix88.h"

//define led matrix columns and rows
#define LEDMATRIX88_COLS 8
#define LEDMATRIX88_ROWS 8

volatile uint8_t ledmatrix88_col = 0; //contains column data
volatile uint8_t ledmatrix88_row = 0; //contains row data

// clear the display
void ledmatrix88_clear(struct ledmatrix88 *self)
{
	memset(self->data, 0, sizeof(self->data));
}

void ledmatrix88_init(struct ledmatrix88 *self, pio_dev_t port, uint8_t x_bank, uint8_t y_bank)
{
	self->port = port;
	self->x_bank = x_bank;
	self->y_bank = y_bank;
	self->cur_x = self->cur_y = 0;
	memset(self->data, 0, sizeof(self->data));
}

void ledmatrix88_write_row(struct ledmatrix88 *self, uint8_t row, uint8_t data)
{
	self->data[row & 0x07] = data;
}

uint8_t ledmatrix88_read_row(struct ledmatrix88 *self, uint8_t row)
{
	return self->data[row & 0x07];
}

void ledmatrix88_update(struct ledmatrix88 *self)
{
	//emit column data
	for (int row = 0; row < 8; row++) {
		pio_write_word(self->port, self->x_bank, self->data[row]);
		pio_write_word(self->port, self->y_bank, (1 << row));
	}
}
