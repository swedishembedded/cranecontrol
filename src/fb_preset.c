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
#include <libfirmware/angle.h>

int fb_try_load_preset(struct fb *self, unsigned preset)
{
	struct fb_config_preset *p = &self->config.presets[preset];
	struct fb_config_preset *home = &self->config.presets[FB_PRESET_HOME];
	if (!p->valid || !home->valid)
		return -1;

	fb_control_set_input(&self->axis[FB_AXIS_UPDOWN], self->measured.pitch, 0.f);
	fb_control_set_input(&self->axis[FB_AXIS_LEFTRIGHT], self->measured.yaw, 0.f);

	// for home we want to rotate to closest halv circle
	float target_yaw = p->yaw;
	float target_pitch = p->pitch;
	if (preset != FB_PRESET_HOME) {
		target_yaw = normalize_angle(target_yaw + home->yaw);
		target_pitch += home->pitch;
	}

	float yaw_diff = normalize_angle(target_yaw - self->measured.yaw);
	if (preset == FB_PRESET_HOME) {
		if (fabsf(yaw_diff) > (M_PI / 2)) {
			target_yaw = normalize_angle(target_yaw + M_PI);
		}

		fb_control_set_target(&self->axis[FB_AXIS_UPDOWN], target_pitch);
		fb_control_set_target(&self->axis[FB_AXIS_LEFTRIGHT], target_yaw);

		fb_control_set_limits(&self->axis[FB_AXIS_UPDOWN], 1 * 0.5, 1 * 0.9, 1 * 0.5);
		fb_control_set_limits(&self->axis[FB_AXIS_LEFTRIGHT], 1 * 0.6, 1 * 0.5, 1 * 0.6);
	} else {
		fb_control_set_target(&self->axis[FB_AXIS_UPDOWN], target_pitch);
		fb_control_set_target(&self->axis[FB_AXIS_LEFTRIGHT], target_yaw);

		fb_control_set_limits(&self->axis[FB_AXIS_UPDOWN], self->measured.pitch_acc * 0.5,
				      self->measured.pitch_speed * 0.9,
				      self->measured.pitch_acc * 0.5);
		fb_control_set_limits(&self->axis[FB_AXIS_LEFTRIGHT], self->measured.yaw_acc * 0.6,
				      self->measured.yaw_speed * 0.5, self->measured.yaw_acc * 0.6);

		fb_control_clock(&self->axis[FB_AXIS_UPDOWN]);
		fb_control_clock(&self->axis[FB_AXIS_LEFTRIGHT]);

		// FIXME: this is a hack for now. Controller needs to be refactored.
		// Rescale the controller time so that both controllers finish at the same time
		float t_updown = fb_control_get_remaining_time(&self->axis[FB_AXIS_UPDOWN]);
		float t_leftright = fb_control_get_remaining_time(&self->axis[FB_AXIS_LEFTRIGHT]);
		float t_min = fminf(t_updown, t_leftright);
		float t_max = fmaxf(t_updown, t_leftright);
		if (fabsf(t_min) > 0.05f) {
			float scale = t_max / t_min;
			if (fabsf(scale) > 0.05) {
				if (t_updown >= t_leftright) {
					// if updown takes longer then leftright needs to go slower
					fb_control_set_timebase(&self->axis[FB_AXIS_UPDOWN], 1.f);
					fb_control_set_timebase(&self->axis[FB_AXIS_LEFTRIGHT],
								1.f / scale);
				} else if (t_leftright >= t_updown) {
					// if leftright takes longer then updown needs to go slower
					fb_control_set_timebase(&self->axis[FB_AXIS_LEFTRIGHT],
								1.f);
					fb_control_set_timebase(&self->axis[FB_AXIS_UPDOWN],
								1.f / scale);
				}
			} else {
				// otherwise set scales to default 1.f
				fb_control_set_timebase(&self->axis[FB_AXIS_LEFTRIGHT], 1.f);
				fb_control_set_timebase(&self->axis[FB_AXIS_UPDOWN], 1.f);
			}
		} else {
			// otherwise set scales to default 1.f
			fb_control_set_timebase(&self->axis[FB_AXIS_LEFTRIGHT], 1.f);
			fb_control_set_timebase(&self->axis[FB_AXIS_UPDOWN], 1.f);
		}
	}

	// console_printf(self->console, "Loading preset %d %d\n", (int32_t)(target_pitch *
	// 1000), (int32_t)(target_yaw * 1000));

	self->control_mode = FB_CONTROL_MODE_AUTO;
	return 0;
}
