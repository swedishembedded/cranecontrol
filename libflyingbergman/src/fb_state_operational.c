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

static const unsigned preset_to_swled[FB_PRESET_COUNT] = {
    [FB_PRESET_HOME] = FB_LED_HOME,
    [FB_PRESET_1] = FB_LED_PRESET_1,
    [FB_PRESET_2] = FB_LED_PRESET_2,
    [FB_PRESET_3] = FB_LED_PRESET_3,
    [FB_PRESET_4] = FB_LED_PRESET_4};

void _fb_state_operational_clock_leds(struct fb *self) {
	for(unsigned c = 0; c < FB_PRESET_COUNT; c++) {
		if(self->inputs.sw[preset_to_swled[c]].pressed) {
			// always turn off the led if button is pressed
			fb_leds_set_state(&self->button_leds, preset_to_swled[c], FB_LED_STATE_OFF);
		} else {
			if(self->config.presets[c].valid) {
				fb_leds_set_state(&self->button_leds, preset_to_swled[c], FB_LED_STATE_ON);
			} else {
				fb_leds_set_state(&self->button_leds, preset_to_swled[c],
				                  FB_LED_STATE_FAINT_ON);
			}
		}
	}
}

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
		if(!sw[preset]->pressed && sw[preset]->toggled) {
			fb_try_load_preset(self, preset);
		} else if(sw[preset]->long_pressed) {
			if(preset == FB_PRESET_HOME) {
				fb_config_set_preset(&self->config, preset, 0, self->measured.yaw);
			} else {
				fb_config_set_preset(&self->config, preset, self->measured.pitch,
				                     self->measured.yaw);
			}

			fb_leds_set_state(&self->button_leds, preset_to_swled[preset],
			                  FB_LED_STATE_ON);

			_fb_enter_state(self, _fb_state_save_preset);
		}
	}
	_fb_state_operational_clock_leds(self);
}
