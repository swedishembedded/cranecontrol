#pragma once

#include <libfirmware/types/timestamp.h>
#include "motion_profile.h"
#include "fb_filter.h"

#define FB_CONTROL_OUTPUT_LIMIT (1.f)

typedef enum {
	FB_CONTROL_MODE_NO_MOTION = 0,
	FB_CONTROL_MODE_REMOTE,
	FB_CONTROL_MODE_MANUAL,
	FB_CONTROL_MODE_AUTO
} fb_control_mode_t;

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
	struct {
		float acc, dec, vel;
	} limits;
	struct fb_filter err_filt;
	float integral;
	float error;
	float new_target;
	float start_pos;
	bool start;
	bool moving;
	bool settling;
	float output;
	unsigned settle_time;
	float timebase;
};

void fb_control_init(struct fb_control *self, const struct fb_config_control *conf);
void fb_control_reset(struct fb_control *self);
void fb_control_set_input(struct fb_control *self, float pos, float vel);
void fb_control_set_limits(struct fb_control *self, float acc, float speed, float dec);
void fb_control_set_target(struct fb_control *self, float pos);
void fb_control_set_timebase(struct fb_control *self, float time);
float fb_control_get_remaining_time(struct fb_control *self);
void fb_control_clock(struct fb_control *self);
float fb_control_get_output(struct fb_control *self);
