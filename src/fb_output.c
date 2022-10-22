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

float _limit_pitch_output(struct fb *self, float pitch, float pitch_speed)
{
	// limit pitch if measured pitch is outside of allowed limit
	float slow_zone_pcnt = 0.15;
	float limit_abs_max = self->config.axis[FB_AXIS_UPDOWN].limits.pos_max;
	float limit_abs_min = self->config.axis[FB_AXIS_UPDOWN].limits.pos_min;
	float travel = limit_abs_max - limit_abs_min;
	float limit_deadband = travel * slow_zone_pcnt;
	float limit_max = (limit_abs_max - limit_deadband * 2.f);
	float limit_min = (limit_abs_min + limit_deadband);

	if (self->measured.pitch > limit_max && self->measured.pitch < limit_abs_max) {
		// if in the slowdown range then limit to percentage before the max point
		float pcnt = 1.f - (self->measured.pitch - limit_max) / (limit_deadband * 2.f);
		pitch = constrain_float(pitch, -pitch_speed * pcnt, pitch_speed);
	} else if (self->measured.pitch > limit_abs_max) {
		pitch = constrain_float(pitch, 0, pitch_speed);
	} else if (self->measured.pitch < limit_min && self->measured.pitch > limit_abs_min) {
		float pcnt = 1.f - (limit_min - self->measured.pitch) / limit_deadband;
		pitch = constrain_float(pitch, -pitch_speed, pitch_speed * pcnt);
	} else if (self->measured.pitch < limit_abs_min) {
		pitch = constrain_float(pitch, -pitch_speed, 0);
	}
	return pitch;
}
//! this function constrains the demanded pitch and yaw output to be compliant with
//! controls for speed and intensity
void fb_output_limited(struct fb *self, float demand_pitch, float demand_yaw, float pitch_speed,
		       float yaw_speed, float pitch_acc, float yaw_acc)
{
	float dt = FB_DEFAULT_DT;
	demand_pitch = constrain_float(demand_pitch, -1.f, 1.f);
	demand_yaw = constrain_float(demand_yaw, -1.f, 1.f);

	float pitch = self->output.pitch;
	if (pitch < demand_pitch)
		pitch = constrain_float(pitch + pitch_acc * dt, pitch, demand_pitch);
	else if (pitch > demand_pitch)
		pitch = constrain_float(pitch - pitch_acc * dt, demand_pitch, pitch);

	float yaw = self->output.yaw;
	if (yaw < demand_yaw)
		yaw = constrain_float(yaw + yaw_acc * dt, yaw, demand_yaw);
	else if (yaw > demand_yaw)
		yaw = constrain_float(yaw - yaw_acc * dt, demand_yaw, yaw);

	pitch = _limit_pitch_output(self, pitch, pitch_speed);

	pitch = constrain_float(pitch, -pitch_speed, pitch_speed);
	yaw = constrain_float(yaw, -yaw_speed, yaw_speed);

	// write final output value
	self->output.pitch = pitch;
	self->output.yaw = yaw;
}
