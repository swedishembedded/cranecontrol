/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb.h"
#include "fb_state.h"

void _fb_state_operational_enter(struct fb *self) {
	fb_leds_reset(&self->button_leds);
	self->control_mode = FB_CONTROL_MODE_MANUAL;
}

void _fb_state_operational(struct fb *self, float dt) {
	struct fb_switch_state *sw[] = {[FB_PRESET_HOME] = &self->inputs.sw[FB_SW_HOME],
	                                [FB_PRESET_1] = &self->inputs.sw[FB_SW_PRESET_1],
	                                [FB_PRESET_2] = &self->inputs.sw[FB_SW_PRESET_2],
	                                [FB_PRESET_3] = &self->inputs.sw[FB_SW_PRESET_3],
	                                [FB_PRESET_4] = &self->inputs.sw[FB_SW_PRESET_4]};

	for(unsigned preset = 0; preset < FB_PRESET_COUNT; preset++) {
		if(sw[preset]->long_pressed) {
			if(preset == FB_PRESET_HOME) {
				fb_config_set_preset(&self->config, preset, 0, self->measured.yaw);
			} else {
				struct fb_config_preset *home = &self->config.presets[FB_PRESET_HOME];
				fb_config_set_preset(&self->config, preset, self->measured.pitch - home->pitch,
				                     self->measured.yaw - home->yaw);
			}
		} else if(!sw[preset]->pressed && sw[preset]->toggled) {
			// if button is released
			fb_try_load_preset(self, preset);
		}
	}
}
