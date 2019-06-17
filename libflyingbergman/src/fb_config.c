#include "fb_config.h"
#include "fb.h"

void fb_config_init(struct fb_config *self) {
	memset(self, 0, sizeof(*self));

	do {
		struct fb_config_control *ax = &self->axis[FB_AXIS_UPDOWN];
		ax->error_filter = (struct fb_config_filter){
		    .a0 = -1.9112f, .a1 = 0.914976f, .b0 = 0.0019161f, .b1 = 0.00186018f};

		ax->limits.pos_max = 0.385;
		ax->limits.pos_min = -0.560;
		ax->limits.integral_max = 1.f;

		ax->Kff = 5.95f;
		ax->Kp = 18.f;
		ax->Ki = 0.0f;
		ax->Kd = 0.2f;
	} while(0);

	do {
		struct fb_config_control *ax = &self->axis[FB_AXIS_LEFTRIGHT];
		ax->error_filter =
		    (struct fb_config_filter){.a0 = 0, .a1 = 0, .b0 = 1.0f, .b1 = 0};

		ax->limits.pos_max = 0;
		ax->limits.pos_min = 0;
		ax->limits.integral_max = 1.f;

		ax->Kff = 0.39;
		ax->Kp = 2.f;
		ax->Ki = 0.0f;
		ax->Kd = 0.2f;
	} while(0);

	self->deadband.pitch = 0.1f;
	self->deadband.yaw = 0.1f;

	// set default limits
	self->limit.pitch =
	    (struct fb_analog_limit){.min = 0, .max = 3790, .omin = -1.f, .omax = 1.f};
	self->limit.joy_pitch =
	    (struct fb_analog_limit){.min = 0, .max = 4096, .omin = -1.f, .omax = 1.f};
	self->limit.joy_yaw =
	    (struct fb_analog_limit){.min = 0, .max = 4096, .omin = -1.f, .omax = 1.f};
	self->limit.pitch_acc = (struct fb_analog_limit){.min = 0,
	                                                 .max = 4096,
	                                                 .omin = FB_PITCH_MAX_ACC * 0.2,
	                                                 .omax = FB_PITCH_MAX_ACC};
	self->limit.yaw_acc = (struct fb_analog_limit){
	    .min = 0, .max = 4096, .omin = FB_YAW_MAX_ACC * 0.2, .omax = FB_YAW_MAX_ACC};
	self->limit.pitch_speed = (struct fb_analog_limit){.min = 0,
	                                                   .max = 4096,
	                                                   .omin = FB_PITCH_MAX_VEL * 0.2,
	                                                   .omax = FB_PITCH_MAX_VEL};
	self->limit.yaw_speed = (struct fb_analog_limit){
	    .min = 0, .max = 4096, .omin = FB_YAW_MAX_VEL * 0.2, .omax = FB_YAW_MAX_VEL};
	self->limit.vmot =
	    (struct fb_analog_limit){.min = 0, .max = 3470, .omin = 0, .omax = 80};
	self->limit.temp_yaw =
	    (struct fb_analog_limit){.min = 860, .max = 2000, .omin = 100, .omax = 100.f};
	self->limit.temp_pitch = (struct fb_analog_limit){
	    .min = 1070, .max = 3150, .omin = -100, .omax = 100.f};
}

void fb_config_set_preset(struct fb_config *self, unsigned preset, float pitch, float yaw) {
	if(preset >= FB_PRESET_COUNT) return;
	self->presets[preset].pitch = pitch;
	self->presets[preset].yaw = yaw;
	self->presets[preset].valid = true;
}
