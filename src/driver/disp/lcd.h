/**
 * SPDX-License-Identifier: GPLv3
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) 2013 Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#ifndef LCD_H
#define LCD_H

#include <inttypes.h>

#include <arch/interface.h>

struct lcd {
	pio_dev_t port;
	uint8_t dataport;
};

extern void lcd_init(struct lcd *self, pio_dev_t port, uint8_t dispAttr);
extern void lcd_clrscr(struct lcd *self);
extern void lcd_home(struct lcd *self);
extern void lcd_gotoxy(struct lcd *self, uint8_t x, uint8_t y);
extern void lcd_led(struct lcd *self, uint8_t onoff);
extern void lcd_putc(struct lcd *self, char c);
extern void lcd_puts(struct lcd *self, const char *s);
extern void lcd_puts_p(struct lcd *self, const char *progmem_s);
extern void lcd_command(struct lcd *self, uint8_t cmd);
extern void lcd_data(struct lcd *self, uint8_t data);

/*@}*/
#endif //LCD_H
