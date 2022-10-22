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

#include "fb.h"
#include "fb_state.h"

void _fb_state_operational_enter(struct fb *self)
{
	fb_leds_reset(&self->button_leds);
	self->control_mode = FB_CONTROL_MODE_MANUAL;
}

void _fb_state_operational(struct fb *self, float dt)
{
	struct fb_switch_state *sw[] = { [FB_PRESET_HOME] = &self->inputs.sw[FB_SW_HOME],
					 [FB_PRESET_1] = &self->inputs.sw[FB_SW_PRESET_1],
					 [FB_PRESET_2] = &self->inputs.sw[FB_SW_PRESET_2],
					 [FB_PRESET_3] = &self->inputs.sw[FB_SW_PRESET_3],
					 [FB_PRESET_4] = &self->inputs.sw[FB_SW_PRESET_4] };

	for (unsigned preset = 0; preset < FB_PRESET_COUNT; preset++) {
		if (sw[preset]->long_pressed) {
			if (preset == FB_PRESET_HOME) {
				fb_config_set_preset(&self->config, preset, self->measured.pitch,
						     self->measured.yaw);
			} else {
				struct fb_config_preset *home =
					&self->config.presets[FB_PRESET_HOME];
				fb_config_set_preset(&self->config, preset,
						     self->measured.pitch - home->pitch,
						     self->measured.yaw - home->yaw);
			}
		} else if (!sw[preset]->pressed && sw[preset]->toggled) {
			// if button is released
			fb_try_load_preset(self, preset);
		}
	}
}
