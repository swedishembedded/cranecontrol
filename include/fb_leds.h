#pragma once

enum {
  FB_LED_PRESET_1 = 7,
  FB_LED_PRESET_2 = 6,
  FB_LED_PRESET_3 = 5,
  FB_LED_PRESET_4 = 4,
  FB_LED_HOME = 3,
  FB_LED_CONN_STATUS = 2,
  FB_LED_COUNT = 8
};

typedef enum {
	FB_LED_STATE_OFF = 0,
	FB_LED_STATE_SLOW_RAMP,
	FB_LED_STATE_ON,
	FB_LED_STATE_3BLINKS_OFF,
	FB_LED_STATE_FAST_RAMP,
	FB_LED_STATE_FAST_BLINK,
	FB_LED_STATE_FAINT_ON
} fb_led_state_t;

struct fb_led {
	float intensity;
	int dim;
	int state;
	unsigned blink_delay;
	int blinks;
};

struct fb_leds {
	struct fb_led leds[FB_LED_COUNT];
};

void fb_leds_init(struct fb_leds *self);
void fb_leds_clock(struct fb_leds *self);
void fb_leds_reset(struct fb_leds *self);
void fb_leds_set_state(struct fb_leds *self, unsigned led, fb_led_state_t state);
float fb_leds_get_intensity(struct fb_leds *self, unsigned led);
