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
	float target_yaw_norm = normalize_angle(target_yaw - self->measured.yaw);
	if(preset == FB_PRESET_HOME) {
		if(fabsf(target_yaw_norm) > (M_PI / 2)) {
			target_yaw_norm = normalize_angle(target_yaw + M_PI);
		}
	}

	fb_control_set_target(&self->axis[FB_AXIS_UPDOWN], target_pitch);
	fb_control_set_target(&self->axis[FB_AXIS_LEFTRIGHT], target_yaw_norm);

	self->ticks_on_target = FB_TICKS_ON_TARGET;
	self->control_mode = FB_CONTROL_MODE_AUTO;
	return 0;
}
