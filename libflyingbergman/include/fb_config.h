/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define FB_PRESET_COUNT 5

#define FB_AXIS_UPDOWN 0
#define FB_AXIS_LEFTRIGHT 1
#define FB_AXIS_COUNT 2

struct fb_analog_limit {
	float min, max;
	float omin, omax;
};

typedef enum {
	FB_CONTROL_POS_UNITS_DEFAULT,
	FB_CONTROL_POS_UNITS_RAD
} fb_control_pos_units_t;

struct fb_config_filter {
	float a0, a1;
	float b0, b1;
};

struct fb_config_control {
	fb_control_pos_units_t pos_units;
	float ilimit_min, ilimit_max;
	float Kff, Kp, Ki, Kd;
	struct {
		float acc, vel, dec;
	} limits;
	struct fb_config_filter error_filter;
};

struct fb_config {
	struct config_preset {
		uint8_t flags;
		float pitch, yaw;
		bool valid;
	} presets[FB_PRESET_COUNT];
	struct fb_config_control axis[FB_AXIS_COUNT];
	struct {
		struct fb_analog_limit pitch;
		struct fb_analog_limit joy_pitch;
		struct fb_analog_limit joy_yaw;
		struct fb_analog_limit pitch_acc;
		struct fb_analog_limit yaw_acc;
		struct fb_analog_limit pitch_speed;
		struct fb_analog_limit yaw_speed;
		struct fb_analog_limit vmot;
		struct fb_analog_limit temp_yaw;
		struct fb_analog_limit temp_pitch;
	} limit;
	struct {
		int32_t pitch_a, pitch_b;
		int32_t yaw_a, yaw_b;
	} dc_cal;
};
