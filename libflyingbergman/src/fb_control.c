/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb_control.h"
#include "fb.h"

#define FB_CONTROL_CLOCK_HZ 1000

void fb_control_init(struct fb_control *self, const struct fb_config_control *conf){
	memset(self, 0, sizeof(*self));
	self->conf = conf;
	fb_filter_init(&self->err_filt, &self->conf->error_filter);
}

void fb_control_set_input(struct fb_control *self, float pos, float vel) {
	self->input.pos = pos;
	self->input.vel = vel;
}

void fb_control_set_target(struct fb_control *self, float pos) {
	self->new_target = pos;
	self->start = true;
}

static void _fb_control_run(struct fb_control *self) {
	motion_profile_get_pva(&self->trajectory, self->time, &self->target.pos,
	                       &self->target.vel, &self->target.acc);

	if(self->conf->pos_units == FB_CONTROL_POS_UNITS_RAD) {
		self->target.pos = normalize_angle(self->initial.pos + self->target.pos);
	}

	float err = self->target.pos - self->input.pos;

	if(self->conf->pos_units == FB_CONTROL_POS_UNITS_RAD) {
		err = normalize_angle(err);
	}

	err = fb_filter_update(&self->err_filt, err);

	self->integral =
	    constrain_float(self->integral + err * (1.f / FB_CONTROL_CLOCK_HZ),
	                    -self->conf->limits.integral_max, self->conf->limits.integral_max);

	float derr = (err - self->error) / (1.f / FB_CONTROL_CLOCK_HZ);
	float co = self->conf->Kff * self->target.vel + self->conf->Kp * err +
	           self->conf->Ki * self->integral + self->conf->Kd * derr;

	co = constrain_float(co, -FB_CONTROL_OUTPUT_LIMIT, FB_CONTROL_OUTPUT_LIMIT);

	self->output = co;
	self->error = err;
}

void fb_control_clock(struct fb_control *self) {
	if(self->moving && motion_profile_completed(&self->trajectory, self->time)) {
		self->time = 0;
		self->moving = false;
	} else if(self->moving) {
		self->time += (1.f / FB_CONTROL_CLOCK_HZ);
		_fb_control_run(self);
	}

	// make sure we start next move in the same clock cycle as completion of previous move
	if(!self->moving && self->start) {
		motion_profile_init(&self->trajectory, self->conf->limits.acc, self->conf->limits.vel, self->conf->limits.dec);
		motion_profile_plan_move(&self->trajectory, self->input.pos,
		                         self->input.vel, self->new_target, 0.f);
		self->start = false;
		self->moving = true;
	}
}

float fb_control_get_output(struct fb_control *self){
	return self->output;
}
