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

#include <math.h>
#include <libfirmware/debug.h>
#include <libfirmware/leds.h>
#include <libfdt/libfdt.h>

struct blinky {
	led_controller_t leds;
};

static void _blinky_thread(void *ptr)
{
	struct blinky *self = (struct blinky *)ptr;
	uint32_t t = thread_ticks_count();
	bool blink_state = false;
	while (1) {
		blink_state = !blink_state;
		if (blink_state) {
			led_on(self->leds, 0);
		} else {
			led_off(self->leds, 0);
		}
		thread_sleep_ms_until(&t, 1000 / 25);
	}
}

static int _blinky_probe(void *fdt, int fdt_node)
{
	led_controller_t leds = leds_find_by_ref(fdt, fdt_node, "leds");

	if (!leds) {
		printk("blinky: no leds\n");
		return -1;
	}

	struct blinky *self = (struct blinky *)kzmalloc(sizeof(struct blinky));
	self->leds = leds;

	thread_create(_blinky_thread, "blinky", 250, self, 1, NULL);

	return 0;
}

static int _blinky_remove(void *fdt, int fdt_node)
{
	// TODO
	return -1;
}

DEVICE_DRIVER(blinky, "app,blinky", _blinky_probe, _blinky_remove)
