#pragma once

#include <libfirmware/timestamp.h>
#include "motion_profile.h"
#include "fb_filter.h"

#define FB_CONTROL_OUTPUT_LIMIT (1.f)

struct fb_control {
	float time;
	struct motion_profile trajectory;
	const struct fb_config_control *conf;
	struct {
		float pos;
	} initial;
	struct {
		float pos, vel, acc;
	} target;
	struct {
		float pos, vel;
	} input;
	struct fb_filter err_filt;
	float integral;
	float error;
	float new_target;
	bool start;
	bool moving;
	float output;
};

void fb_control_init(struct fb_control *self, const struct fb_config_control *conf);
void fb_control_set_input(struct fb_control *self, float pos, float vel);
void fb_control_set_target(struct fb_control *self, float pos);
void fb_control_clock(struct fb_control *self);
float fb_control_get_output(struct fb_control *self);
