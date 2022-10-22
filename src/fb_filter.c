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

#include <string.h>

#include "fb_config.h"
#include "fb_filter.h"

static const struct fb_config_filter _default_conf = { .b0 = 1, .b1 = 0, .a0 = 0, .a1 = 0 };

void fb_filter_init(struct fb_filter *self, const struct fb_config_filter *conf)
{
	memset(self, 0, sizeof(*self));
	self->conf = conf;
}

void fb_filter_init_default(struct fb_filter *self)
{
	fb_filter_init(self, &_default_conf);
}

/**
   Execute a filter specified as conf
 */
float fb_filter_update(struct fb_filter *self, float in)
{
	float out = self->conf->b0 * in + self->conf->b1 * self->in[0] -
		    self->conf->a0 * self->out[0] - self->conf->a1 * self->out[1];
	self->in[0] = in;
	self->out[1] = self->out[0];
	self->out[0] = out;
	return out;
}
