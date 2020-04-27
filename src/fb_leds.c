/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb_leds.h"
#include <string.h>

#define FB_LED_FAST_BLINK_DELAY 100

void fb_leds_init(struct fb_leds *self) {
	memset(self, 0, sizeof(*self));
}

void _fb_led_clock(struct fb_led *self) {
	float dt = 0.001f;
	switch(self->state) {
		case FB_LED_STATE_ON: {
			self->intensity = 1.f;
		} break;
		case FB_LED_STATE_FAINT_ON: {
			self->intensity = 0.1f;
		} break;
		case FB_LED_STATE_OFF: {
			self->intensity = 0.0f;
		} break;
		case FB_LED_STATE_FAST_BLINK:
			self->blink_delay++;
			if(self->blink_delay > FB_LED_FAST_BLINK_DELAY){
				if(self->intensity > 0.9){
					self->intensity = 0;
				} else {
					self->intensity = 1.f;
				}
				self->blink_delay = 0;
			}
			break;
		case FB_LED_STATE_SLOW_RAMP:
		case FB_LED_STATE_FAST_RAMP:
		case FB_LED_STATE_3BLINKS_OFF: {
			float dim_time = 0.5;
			if(self->state == FB_LED_STATE_FAST_RAMP ||
			   self->state == FB_LED_STATE_3BLINKS_OFF)
				dim_time = 5.0;
			if(self->dim < 0) {
				self->intensity -= dim_time * dt;
				if(self->intensity < 0.0f) {
					self->intensity = 0.0f;
					self->dim = -self->dim;
				}
			} else if(self->dim > 0) {
				self->intensity += dim_time * dt;
				if(self->intensity > 1.0f) {
					self->intensity = 1.0f;
					self->dim = -self->dim;
					if(self->state == FB_LED_STATE_3BLINKS_OFF)
						self->blinks++;
				}
			}

			if(self->state == FB_LED_STATE_3BLINKS_OFF && self->blinks > 3) {
				self->blinks = 0;
				self->state = FB_LED_STATE_OFF;
			}
		} break;
	}
}

void fb_leds_reset(struct fb_leds *self) {
	// bring all leds to known state upon state change
	for(unsigned c = 0; c < FB_LED_COUNT; c++) {
		self->leds[c].dim = 1;
		self->leds[c].state = FB_LED_STATE_OFF;
	}
}

void fb_leds_clock(struct fb_leds *self) {
	for(unsigned c = 0; c < FB_LED_COUNT; c++) {
		_fb_led_clock(&self->leds[c]);
	}
}

float fb_leds_get_intensity(struct fb_leds *self, unsigned idx) {
	if(idx >= FB_LED_COUNT)
		return 0;
	return self->leds[idx].intensity;
}

void fb_leds_set_state(struct fb_leds *self, unsigned led, fb_led_state_t state) {
	if(led >= FB_LED_COUNT)
		return;
	self->leds[led].state = state;
}
