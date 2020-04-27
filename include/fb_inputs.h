/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#pragma once

#define FB_SWITCH_COUNT 8

struct fb_inputs_local {
	uint16_t joy_yaw, joy_pitch;
	uint16_t yaw_acc, pitch_acc;
	uint16_t yaw_speed, pitch_speed;
	bool enc1_aux1, enc1_aux2;
	bool enc2_aux1, enc2_aux2;
};

struct fb_switch_state {
	bool pressed;
	bool long_pressed;
	bool toggled;
	timestamp_t pressed_time;
};

struct fb_inputs {
	bool use_local;
	struct fb_inputs_local local;

	uint16_t vsa_yaw;
	uint16_t vsb_yaw;
	uint16_t vsc_yaw;
	uint16_t vsa_pitch;
	uint16_t vsb_pitch;
	uint16_t vsc_pitch;
	uint16_t ia_yaw;
	uint16_t ib_yaw;
	uint16_t ia_pitch;
	uint16_t ib_pitch;
	uint16_t pitch;
	int16_t yaw;

	struct fb_switch_state sw[FB_SWITCH_COUNT];

	uint16_t joy_yaw, joy_pitch;
	uint16_t yaw_acc, pitch_acc;
	uint16_t yaw_speed, pitch_speed;
	uint16_t vmot;
	uint16_t temp_yaw, temp_pitch;
	uint8_t can_addr;

	bool enc1_aux1, enc1_aux2, enc2_aux1, enc2_aux2;
	struct mutex lock;

	struct {
		timestamp_diff_t inputs_td;
	} stats;

	int mux_chan;
	uint16_t mux_adc[8];

	adc_device_t adc;
	gpio_device_t mux;
	gpio_device_t enc1_gpio;
	gpio_device_t enc2_gpio;
	gpio_device_t sw_gpio;
	encoder_device_t enc1, enc2;
	gpio_device_t gpio_ex;
};

void fb_inputs_init(struct fb_inputs *self);
void fb_inputs_update(struct fb_inputs *self);
