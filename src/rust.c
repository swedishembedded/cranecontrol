/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2019-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libfirmware/console.h>

extern void rust_function(console_device_t dev);

static int _rust_probe(void *fdt, int fdt_node)
{
	console_device_t con = console_find_by_ref(fdt, fdt_node, "console");
	rust_function(con);
	return 0;
}

static int _rust_remove(void *fdt, int fdt_node)
{
	// TODO
	return -1;
}

DEVICE_DRIVER(rust, "app,rust", _rust_probe, _rust_remove)
