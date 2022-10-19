#pragma once

#include <libfirmware/adc.h>
#include <libfirmware/analog.h>
#include <libfirmware/can.h>
#include <libfirmware/canopen.h>
#include <libfirmware/chip.h>
#include <libfirmware/console.h>
#include <libfirmware/driver.h>
#include <libfirmware/encoder.h>
#include <libfirmware/events.h>
#include <libfirmware/gpio.h>
#include <libfirmware/leds.h>
#include <libfirmware/constrain.h>
#include <libfirmware/memory.h>
#include <libfirmware/regmap.h>
#include <libfirmware/serial.h>
#include <libfirmware/thread/thread.h>
#include <libfirmware/types/timestamp.h>
#include <libfirmware/usb.h>

#include <libdriver/hbridge/drv8302.h>

#include "canopen.h"
#include "fb_can.h"
#include "fb_cmd.h"
#include "fb_config.h"
#include "fb_control.h"
#include "fb_filter.h"
#include "fb_leds.h"
#include "fb_state.h"
#include "fb_inputs.h"
#include <libplc/motion_profile.h>

#define OC_POT_OC_ADJ_MOTOR1 2
#define OC_POT_OC_ADJ_MOTOR2 3

#define FB_DEFAULT_DT 0.001f

#define FB_SW_PRESET_1 7
#define FB_SW_PRESET_2 6
#define FB_SW_PRESET_3 5
#define FB_SW_PRESET_4 4
#define FB_SW_HOME 3
#define FB_SW_DIR_YAW_NORMAL 0
#define FB_SW_DIR_YAW_REVERSED 0
#define FB_SW_DIR_PITCH_NORMAL 0
#define FB_SW_DIR_PITCH_REVERSED 0

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

#define FB_MOTOR_YAW_GEARBOX_IN_RPM 2800
#define FB_MOTOR_YAW_GEARBOX_OUT_RPM 28
#define FB_MOTOR_YAW_GEAR_RATIO (90.f / 17.f)
#define FB_MOTOR_YAW_TICKS_PER_ROT                                                                 \
	(FB_MOTOR_YAW_GEAR_RATIO *                                                                 \
	 ((FB_MOTOR_YAW_GEARBOX_IN_RPM * 512 * 4) / FB_MOTOR_YAW_GEARBOX_OUT_RPM))

#define FB_SLAVE_TIMEOUT_MS 1000

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

typedef enum { FB_MODE_UNDEFINED = 0, FB_MODE_MASTER, FB_MODE_SLAVE } fb_mode_t;

#define FB_BTN_LONG_PRESS_TIME_US 1000000UL

#define FB_PRESET_BIT_VALID (1 << 0)

#define FB_REMOTE_TIMEOUT 50000

enum {
	FB_SLAVE_VALID_PITCH = (1 << 0),
	FB_SLAVE_VALID_YAW = (1 << 1),
	FB_SLAVE_VALID_I_PITCH = (1 << 2),
	FB_SLAVE_VALID_I_YAW = (1 << 3),
	FB_SLAVE_VALID_MICROS = (1 << 4)
};

struct fb {
	led_controller_t leds;
	console_device_t console;
	analog_device_t sw_leds;
	analog_device_t mot_x;
	analog_device_t mot_y;
	memory_device_t eeprom;
	analog_device_t oc_pot;
	analog_device_t an_out;
	can_device_t can1, can2;
	struct regmap *regmap;
	memory_device_t canopen_mem;
	memory_device_t can1_mem;
	memory_device_t can2_mem;

	drv8302_t drv_pitch, drv_yaw;
	events_device_t events;
	//gpio_device_t debug_gpio;

	struct events_subscriber sub;

	struct fb_config config;

	struct fb_can can;

	struct fb_inputs inputs;

	// processed measurements based on sensor values
	struct fb_meas_data {
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

	struct fb_leds button_leds;

	struct {
		timestamp_t prev_loop_time;

		struct {
			struct fb_filter pfilt;
			struct fb_filter vfilt;
		} pitch, yaw;

		int32_t yaw_ticks;
		uint16_t prev_yaw_ticks;

		void (*fn)(struct fb *self, float dt);
	} state;

	struct fb_control axis[FB_AXIS_COUNT];

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

	struct {
		struct mutex lock;
		timestamp_diff_t loop_td;
	} stats;

	uint8_t can_addr;

	fb_control_mode_t control_mode;
	fb_mode_t mode;

	int ticks_on_target;

	struct regmap_range comm_range, mfr_range, mfr_range_slave;
};

int _ui_cmd(console_device_t con, void *userptr, int argc, char **argv);

void _fb_update_measurements(struct fb *self);

void fb_init(struct fb *self);
int fb_try_load_preset(struct fb *self, unsigned preset);
void fb_output_limited(struct fb *self, float pitch, float yaw, float pitch_speed, float yaw_speed,
		       float pitch_acc, float yaw_acc);
