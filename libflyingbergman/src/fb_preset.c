/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb.h"

int fb_try_load_preset(struct fb *self, unsigned preset) {
	struct fb_config_preset *p = &self->config.presets[preset];
	if(!p->valid)
		return -1;
	// for home we want to rotate to closest halv circle
	float target_yaw = p->yaw;
	float target_pitch = p->pitch;
	float yaw_diff = normalize_angle(target_yaw - self->measured.yaw);
	if(preset == FB_PRESET_HOME) {
		if(fabsf(yaw_diff) > (M_PI / 2)) {
			target_yaw = normalize_angle(target_yaw + M_PI);
		}
	}

	fb_control_set_limits(&self->axis[FB_AXIS_UPDOWN], self->measured.pitch_acc,
	                      self->measured.pitch_speed, self->measured.pitch_acc);
	fb_control_set_limits(&self->axis[FB_AXIS_LEFTRIGHT], self->measured.yaw_acc,
	                      self->measured.yaw_speed, self->measured.yaw_acc);

	fb_control_set_input(&self->axis[FB_AXIS_UPDOWN], self->measured.pitch, 0.f);
	fb_control_set_input(&self->axis[FB_AXIS_LEFTRIGHT], self->measured.yaw, 0.f);

	fb_control_set_target(&self->axis[FB_AXIS_UPDOWN], target_pitch);
	fb_control_set_target(&self->axis[FB_AXIS_LEFTRIGHT], target_yaw);

	//console_printf(self->console, "Loading preset %d %d\n", (int32_t)(target_pitch * 1000), (int32_t)(target_yaw * 1000));

	self->control_mode = FB_CONTROL_MODE_AUTO;
	return 0;
}
