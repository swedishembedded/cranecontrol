/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FB_PITCH_MAX_ACC 1.f
#define FB_PITCH_MAX_VEL 1.f

#define FB_YAW_MAX_ACC 1.f
#define FB_YAW_MAX_VEL 1.f

#define FB_AXIS_UPDOWN 0
#define FB_AXIS_LEFTRIGHT 1
#define FB_AXIS_COUNT 2

enum {
	FB_PRESET_HOME = 0,
	FB_PRESET_1,
	FB_PRESET_2,
	FB_PRESET_3,
	FB_PRESET_4,
	FB_PRESET_COUNT
};

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
	unsigned settling_time;
	float Kff, Kp, Ki, Kd;
	struct {
		//float acc, vel, dec;
		float pos_max, pos_min;
		float integral_max;
	} limits;
	struct fb_config_filter error_filter;
};

struct fb_config {
	struct fb_config_preset {
		uint8_t flags;
		float pitch, yaw;
		bool valid;
	} presets[FB_PRESET_COUNT];
	struct fb_config_control axis[FB_AXIS_COUNT];
	struct {
		float pitch, yaw;
	} deadband;
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

void fb_config_init(struct fb_config *self);
void fb_config_set_preset(struct fb_config *self, unsigned preset, float pitch,
                          float yaw);
