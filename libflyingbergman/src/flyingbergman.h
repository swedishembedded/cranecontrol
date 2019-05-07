#pragma once

#include <libfirmware/serial.h>
#include <libfirmware/math.h>
#include <libfirmware/chip.h>
#include <libfirmware/driver.h>
#include <libfirmware/thread.h>
#include <libfirmware/console.h>
#include <libfirmware/leds.h>
#include <libfirmware/usb.h>
#include <libfirmware/timestamp.h>
#include <libfirmware/gpio.h>
#include <libfirmware/adc.h>
#include <libfirmware/analog.h>
#include <libfirmware/encoder.h>
#include <libfirmware/memory.h>
#include <libfirmware/timestamp.h>
#include <libfirmware/can.h>
#include <libfirmware/regmap.h>
#include <libfirmware/canopen.h>
#include <libfirmware/events.h>

#include <libdriver/hbridge/drv8302.h>

#include "canopen.h"
#include "motion_profile.h"

#define FB_SWITCH_COUNT 8
#define FB_LED_COUNT 8
#define FB_PRESET_COUNT 5
#define OC_POT_OC_ADJ_MOTOR1 2
#define OC_POT_OC_ADJ_MOTOR2 3

#define FB_SW_PRESET1 7
#define FB_SW_PRESET2 6
#define FB_SW_PRESET3 5
#define FB_SW_PRESET4 4
#define FB_SW_HOME 3
#define FB_SW_DIR_YAW_NORMAL 0
#define FB_SW_DIR_YAW_REVERSED 0
#define FB_SW_DIR_PITCH_NORMAL 0
#define FB_SW_DIR_PITCH_REVERSED 0

#define FB_LED_PRESET1 FB_SW_PRESET1
#define FB_LED_PRESET2 FB_SW_PRESET2
#define FB_LED_PRESET3 FB_SW_PRESET3
#define FB_LED_PRESET4 FB_SW_PRESET4
#define FB_LED_HOME FB_SW_HOME
#define FB_LED_CONN_STATUS 2

#define FB_LED_STATE_OFF 0
#define FB_LED_STATE_SLOW_RAMP 1
#define FB_LED_STATE_ON 2
#define FB_LED_STATE_3BLINKS_OFF 3
#define FB_LED_STATE_FAST_RAMP 4
#define FB_LED_STATE_FAINT_ON 5

#define FB_ADC_IB1_CHAN 0
#define FB_ADC_IB2_CHAN 1
#define FB_ADC_MUX_CHAN 2

#define FB_ADC_IA1_CHAN 3
#define FB_ADC_IA2_CHAN 4
#define FB_ADC_VMOT_CHAN 5

#define FB_ADC_VSA1_CHAN 6
#define FB_ADC_VSA2_CHAN 7
#define FB_ADC_TEMP1_CHAN 8

#define FB_ADC_VSB1_CHAN 9
#define FB_ADC_VSB2_CHAN 10
#define FB_ADC_TEMP2_CHAN 11

#define FB_ADC_VSC1_CHAN 12
#define FB_ADC_VSC2_CHAN 13
#define FB_ADC_TEMP1_2_CHAN 14

#define FB_CANOPEN_MASTER_ADDRESS 0x01
#define FB_CANOPEN_SLAVE_ADDRESS 0x02

#define FB_GPIO_CAN_ADDR0 4
#define FB_GPIO_CAN_ADDR1 5
#define FB_GPIO_CAN_ADDR2 6
#define FB_GPIO_CAN_ADDR3 7

#define FB_MOTOR_YAW_GEAR_RATIO (90.f / 17.f)
#define FB_MOTOR_YAW_TICKS_PER_ROT (FB_MOTOR_YAW_GEAR_RATIO * ((2800 * 512 * 4) / 28))

#define FB_SLAVE_TIMEOUT_MS 1000
#define FB_TICKS_ON_TARGET 2000

#define FB_PITCH_MAX_ACC 0.4f
#define FB_PITCH_MAX_VEL 0.15f

#define FB_YAW_MAX_ACC (2.f / (90.f / 17.f))
#define FB_YAW_MAX_VEL (1.3f / (90.f / 17.f))

enum {
	FB_ADCMUX_YAW_CHAN = 0,
	FB_ADCMUX_PITCH_CHAN = 1,
	FB_ADCMUX_YAW_SPEED_CHAN = 2,
	FB_ADCMUX_YAW_ACC_CHAN = 3,
	FB_ADCMUX_PITCH_ACC_CHAN = 4,
	FB_ADCMUX_PITCH_SPEED_CHAN = 5,
	FB_ADCMUX_CHAN_RESERVED15 = 6,
	FB_ADCMUX_MOTOR_PITCH_CHAN = 7
};

typedef enum {
	FB_MODE_MASTER = 0,
	FB_MODE_SLAVE = 1
} fb_mode_t;

#define FB_HOME_LONG_PRESS_TIME_US 1000000UL

#define FB_PRESET_BIT_VALID (1 << 0)

#define FB_REMOTE_TIMEOUT 50000

typedef enum {
	FB_CONTROL_MODE_NO_MOTION = 0,
	FB_CONTROL_MODE_REMOTE,
	FB_CONTROL_MODE_MANUAL,
	FB_CONTROL_MODE_AUTO
} fb_control_mode_t;

enum {
	FB_SLAVE_VALID_PITCH	= (1 << 0),
	FB_SLAVE_VALID_YAW		= (1 << 1),
	FB_SLAVE_VALID_I_PITCH	= (1 << 2),
	FB_SLAVE_VALID_I_YAW	= (1 << 3),
	FB_SLAVE_VALID_MICROS	= (1 << 4)
};

struct fb_analog_limit {
	float min, max;
	float omin, omax;
};

struct fb_filter_config {
	float a0, a1;
	float b0, b1;
};

struct fb_filter {
	float in[2], out[2];
};

struct fb_config {
	struct config_preset {
		uint8_t flags;
		float pitch, yaw;
		bool valid;
	} presets[FB_PRESET_COUNT];
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

struct application {
    led_controller_t leds;
	console_device_t console;
	gpio_device_t sw_gpio;
	gpio_device_t gpio_ex;
	analog_device_t sw_leds;
	adc_device_t adc;
	analog_device_t mot_x;
	analog_device_t mot_y;
	gpio_device_t mux;
	encoder_device_t enc1, enc2;
	memory_device_t eeprom;
	analog_device_t oc_pot;
	can_device_t can1, can2;
	regmap_device_t regmap;
	memory_device_t canopen_mem;
	memory_device_t can1_mem;
	memory_device_t can2_mem;
	gpio_device_t enc1_gpio;
	gpio_device_t enc2_gpio;
	drv8302_t drv_pitch, drv_yaw;
	events_device_t events;
	gpio_device_t debug_gpio;

	struct events_subscriber sub;

	struct fb_config config;

	// raw sensor values
	struct fb_inputs_data {
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

		struct fb_switch_state {
			bool pressed;
			bool toggled;
			timestamp_t pressed_time;
		} sw[FB_SWITCH_COUNT];

		uint16_t joy_yaw, joy_pitch;
		uint16_t yaw_acc, pitch_acc;
		uint16_t yaw_speed, pitch_speed;
		uint16_t vmot;
		uint16_t temp_yaw, temp_pitch;
		uint8_t can_addr;

		bool enc1_aux1, enc1_aux2, enc2_aux1, enc2_aux2;
		struct mutex lock;
	} inputs;

	// processed measurements based on sensor values
	struct fb_meas_data{
		float vsa_yaw;
		float vsb_yaw;
		float vsc_yaw;
		float vsa_pitch;
		float vsb_pitch;
		float vsc_pitch;
		float ia_yaw;
		float ib_yaw;
		float ia_pitch;
		float ib_pitch;
		float pitch;
		float yaw;

		float joy_yaw, joy_pitch;
		float yaw_acc, pitch_acc;
		float yaw_speed, pitch_speed;
		float vmot;
		float temp_yaw, temp_pitch;

		struct fb_filter pfilt;
		struct mutex lock;
	} measured;

	struct {
		timestamp_t prev_loop_time;

		struct fb_led_state {
			float intensity;
			int dim;
			int state;
			int blinks;
		} leds[FB_LED_COUNT];

		struct {
			struct fb_filter pfilt;
			struct fb_filter vfilt;
		} pitch, yaw;

		int32_t yaw_ticks;
		int16_t prev_yaw_ticks;

		void (*fn)(struct application *self, float dt);
	} state;

	struct {
		struct {
			float error;
			float integral;
			float output;
			float target;
			float start;
			struct motion_profile trajectory;
			struct fb_filter efilt;
		} pitch, yaw;
	} controller;

	struct fb_ui {
		bool valid;
		float joy_yaw, joy_pitch;
	} ui;

	// data received from canopen slave (motor)
	struct {
		int16_t yaw, pitch;
		int16_t i_yaw, i_pitch;
		usec_t micros;
		timestamp_t timeout;
		uint32_t valid_bits;
		uint16_t vmot;
		struct mutex lock;
	} slave;

	struct {
		float pitch, yaw;
	} output;

	struct {
		int16_t pitch, yaw;
		timestamp_t pitch_update_timeout, yaw_update_timeout;
	} remote;

	uint8_t can_addr;

	fb_control_mode_t control_mode;
	fb_mode_t mode;

	int ticks_on_target;
	int mux_chan;
	uint16_t mux_adc[8];

	struct regmap_range comm_range, mfr_range, mfr_range_slave;
};

int _ui_cmd(console_device_t con, void *userptr, int argc, char **argv);

void _fb_update_measurements(struct application *self);
void _fb_read_inputs(struct application *self);
float _fb_filter(struct fb_filter *self, struct fb_filter_config conf, float in);