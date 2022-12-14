/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dts/flyingbergman.h"
#include "fb.h"
#include "fb_can.h"
#include "fb_cmd.h"
#include "fb_inputs.h"
#include "fb_leds.h"
#include "fb_state.h"

#include <math.h>
#include <libfirmware/debug.h>
#include <libfdt/libfdt.h>

static void _fb_indicator_loop(void *ptr)
{
	struct fb *self = (struct fb *)ptr;
	uint32_t t = thread_ticks_count();
	usec_t blink_us = 1000000;
	timestamp_t blink_delay = timestamp();
	bool blink_state = false;
	while (1) {
		if (timestamp_expired(blink_delay)) {
			blink_state = !blink_state;
			blink_delay = timestamp_from_now_us(blink_us);
			if (blink_state) {
				led_on(self->leds, 0);
			} else {
				led_off(self->leds, 0);
			}
		}
		for (unsigned c = 0; c < FB_LED_COUNT; c++) {
			analog_write(self->sw_leds, c,
				     fb_leds_get_intensity(&self->button_leds, c));
		}
		thread_sleep_ms_until(&t, 1000 / 25);
	}
}

void _fb_update_leds(struct fb *self)
{
	if (self->can.timed_out) {
		fb_leds_set_state(&self->button_leds, FB_LED_CONN_STATUS, FB_LED_STATE_ON);
	} else {
		fb_leds_set_state(&self->button_leds, FB_LED_CONN_STATUS, FB_LED_STATE_OFF);
	}

	if (self->can.timed_out) {
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_FAST_BLINK);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_1, FB_LED_STATE_FAST_BLINK);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_2, FB_LED_STATE_FAST_BLINK);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_3, FB_LED_STATE_FAST_BLINK);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_4, FB_LED_STATE_FAST_BLINK);
	} else if (self->state.fn == _fb_state_wait_power) {
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_SLOW_RAMP);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_1, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_2, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_3, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_4, FB_LED_STATE_OFF);
	} else if (self->state.fn == _fb_state_wait_home) {
		struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
		if (home->long_pressed) {
			fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_FAST_BLINK);
		} else if (home->pressed) {
			fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_OFF);
		} else {
			fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_FAST_RAMP);
		}
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_1, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_2, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_3, FB_LED_STATE_OFF);
		fb_leds_set_state(&self->button_leds, FB_LED_PRESET_4, FB_LED_STATE_OFF);
	} else if (self->state.fn == _fb_state_operational) {
		unsigned idx[] = { FB_SW_HOME, FB_SW_PRESET_1, FB_SW_PRESET_2, FB_SW_PRESET_3,
				   FB_SW_PRESET_4 };
		unsigned pidx[] = { FB_PRESET_HOME, FB_PRESET_1, FB_PRESET_2, FB_PRESET_3 };
		for (unsigned c = 0; c < (sizeof(idx) / sizeof(idx[0])); c++) {
			struct fb_switch_state *sw = &self->inputs.sw[idx[c]];
			if (sw->long_pressed) {
				fb_leds_set_state(&self->button_leds, idx[c],
						  FB_LED_STATE_FAST_BLINK);
			} else if (sw->pressed) {
				fb_leds_set_state(&self->button_leds, idx[c], FB_LED_STATE_OFF);
			} else {
				if (self->config.presets[pidx[c]].valid) {
					fb_leds_set_state(&self->button_leds, idx[c],
							  FB_LED_STATE_ON);
				} else {
					fb_leds_set_state(&self->button_leds, idx[c],
							  FB_LED_STATE_FAINT_ON);
				}
			}
		}
	}
}

void _fb_enter_state(struct fb *self, void (*fp)(struct fb *self, float dt))
{
	// enter new state
	if (fp == _fb_state_wait_power) {
		// blink the led quickly
		self->control_mode = FB_CONTROL_MODE_NO_MOTION;
	}
	if (fp == _fb_state_wait_home) {
		self->control_mode = FB_CONTROL_MODE_MANUAL;
	} else if (fp == _fb_state_operational) {
		_fb_state_operational_enter(self);
	}

	self->state.fn = fp;
}

void _fb_state_wait_power(struct fb *self, float dt)
{
	// in this state joystick does not move the motors and device waits for user to
	// toggle home button once
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if (home->toggled) {
		printk("fb: move motors to desired home position\n");
		// once home button is toggled once we enter the state where motors can be moved
		// but no other controls are available
		_fb_enter_state(self, _fb_state_wait_home);
	}
}

void _fb_state_wait_home(struct fb *self, float dt)
{
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if (home->long_pressed) {
		// make sure home button is lit
		// if it has been held for a number of seconds then we save the home position
		self->config.presets[0].pitch = self->measured.pitch;
		self->config.presets[0].yaw = self->measured.yaw;
		self->config.presets[0].valid = true;
	} else if (self->config.presets[0].valid && !home->pressed && home->toggled) {
		_fb_enter_state(self, _fb_state_operational);
	}
}

static void _fb_check_mode(struct fb *self)
{
	if (self->can_addr != self->inputs.can_addr) {
		canopen_set_address(self->canopen_mem, self->inputs.can_addr);
		if (self->inputs.can_addr == FB_CANOPEN_MASTER_ADDRESS) {
			self->mode = FB_MODE_MASTER;
			canopen_set_mode(self->canopen_mem, CANOPEN_MASTER);
			regmap_write_u32(self->regmap, CANOPEN_REG_DEVICE_CYCLE_PERIOD, 1000);
		} else {
			self->mode = FB_MODE_SLAVE;
			self->control_mode = FB_CONTROL_MODE_REMOTE;
			canopen_set_mode(self->canopen_mem, CANOPEN_SLAVE);
		}
		self->can_addr = self->inputs.can_addr;
	}
}

void _fb_update_state(struct fb *self, float dt)
{
	self->state.fn(self, dt);

	_fb_check_mode(self);
}

static void _fb_update_control(struct fb *self, float dt)
{
	switch (self->control_mode) {
	case FB_CONTROL_MODE_NO_MOTION: {
		// important to apply the limit even when we want no output at all
		fb_output_limited(self, 0, 0, 0, 0, FB_PITCH_MAX_ACC, FB_YAW_MAX_ACC);
	} break;
	case FB_CONTROL_MODE_REMOTE: {
		/**
			 * In this mode motors are controlled remotely over manufacturer segment
			 * It is still however important to limit change in output so that we do not
			 * suddenly stop the motor if remote communication is lost
			 */
		float pitch = (float)self->remote.pitch * 0.001f;
		float yaw = (float)self->remote.yaw * 0.001f;
		fb_output_limited(self, pitch, yaw, 1.f, 1.f, 4.f * FB_PITCH_MAX_ACC,
				  4.f * FB_YAW_MAX_ACC);
	} break;
	case FB_CONTROL_MODE_MANUAL: {
		/**
			 * In manual mode we generate output directly based on joystick input
			 */
		fb_output_limited(self, self->measured.joy_pitch, self->measured.joy_yaw,
				  self->measured.pitch_speed, self->measured.yaw_speed,
				  self->measured.pitch_acc, self->measured.yaw_acc);
	} break;
	case FB_CONTROL_MODE_AUTO: {
		/*
			 * In auto mode we generate a trajectory that gives us a target velocity and
			 * target position for each frame The control output signal is then generated
			 * based on error between trajectory setpoint and actual position/velocity User
			 * controls are added to the control action to still make it possible to
			 * control using joystick
			 */
		// FIXME: set the current velocity
		fb_control_set_input(&self->axis[FB_AXIS_UPDOWN], self->measured.pitch, 0.f);
		fb_control_set_input(&self->axis[FB_AXIS_LEFTRIGHT], self->measured.yaw, 0.f);

		fb_control_clock(&self->axis[FB_AXIS_UPDOWN]);
		fb_control_clock(&self->axis[FB_AXIS_LEFTRIGHT]);

		float co_pitch = fb_control_get_output(&self->axis[FB_AXIS_UPDOWN]);
		float co_yaw = fb_control_get_output(&self->axis[FB_AXIS_LEFTRIGHT]);

		fb_output_limited(self, -co_pitch, -co_yaw, 1.f, 1.f, 8.f * FB_PITCH_MAX_ACC,
				  4.f * FB_YAW_MAX_ACC);
		/*
fb_output_limited(self, -co_pitch, -co_yaw, self->measured.pitch_speed,
			            self->measured.yaw_speed, self->measured.pitch_acc,
			            self->measured.yaw_acc);
			            */
		// if joystick is moved during automatic move then we switch back to manual
		if (fabsf(self->measured.joy_pitch) > self->config.deadband.pitch ||
		    fabsf(self->measured.joy_yaw) > self->config.deadband.yaw) {
			fb_control_reset(&self->axis[FB_AXIS_UPDOWN]);
			fb_control_reset(&self->axis[FB_AXIS_LEFTRIGHT]);
			self->control_mode = FB_CONTROL_MODE_MANUAL;
		}
	} break;
	}
}

static void _fb_update_motors(struct fb *self)
{
	float pitch = self->output.pitch;
	float yaw = self->output.yaw;

#define MAX_SWING 0.46f
	pitch = constrain_float(pitch * 0.5, -MAX_SWING, MAX_SWING);
	yaw = constrain_float(yaw * 0.5, -MAX_SWING, MAX_SWING);

	// it is important that the following finishes in one block
	// otherwise there is a real danger that something ether breaks
	// or spins out of control
	thread_sched_suspend();

	analog_write(self->mot_y, 0, 0.5 + pitch);
	analog_write(self->mot_y, 1, 0.5 - pitch);
	analog_write(self->mot_y, 2, 0.5 + pitch);

	analog_write(self->mot_x, 0, 0.5 + yaw);
	analog_write(self->mot_x, 1, 0.5 - yaw);
	analog_write(self->mot_x, 2, 0.5 + yaw);

	thread_sched_resume();
}

static void fb_check_link_state(struct fb *self)
{
	if (self->mode == FB_MODE_SLAVE && !self->inputs.use_local) {
		if (self->can.timed_out) {
			if (self->control_mode == FB_CONTROL_MODE_REMOTE) {
				self->control_mode = FB_CONTROL_MODE_NO_MOTION;
			}
		} else {
			self->control_mode = FB_CONTROL_MODE_REMOTE;
		}
	} else if (self->mode == FB_MODE_MASTER) {
		//if(self->can.timed_out && self->state.fn != _fb_state_wait_power) {
		//	_fb_enter_state(self, _fb_state_wait_power);
		//}
	}
}

static void _fb_update(struct fb *self)
{
	fb_can_clock(self);
	fb_check_link_state(self);
	_fb_update_measurements(self);
	_fb_update_state(self, FB_DEFAULT_DT);
	_fb_update_control(self, FB_DEFAULT_DT);
	_fb_update_motors(self);
	_fb_update_leds(self);
	fb_leds_clock(&self->button_leds);
}

static void _fb_control_loop(void *ptr)
{
	struct fb *self = (struct fb *)ptr;
	uint32_t ticks = thread_ticks_count();
	timestamp_t ts, te;
	timestamp_diff_t loop_td;
	while (1) {
		fb_inputs_update(&self->inputs);

		ts = timestamp();
		_fb_update(self);
		te = timestamp();
		loop_td = timestamp_sub(te, ts);

		thread_mutex_lock(&self->stats.lock);
		self->stats.loop_td = loop_td;
		thread_mutex_unlock(&self->stats.lock);

		thread_sleep_ms_until(&ticks, 1);
	}
}

static void _fb_calibrate_current_sensors(struct fb *self)
{
	self->config.dc_cal.pitch_a = 0;
	self->config.dc_cal.pitch_b = 0;
	self->config.dc_cal.yaw_a = 0;
	self->config.dc_cal.yaw_b = 0;

	drv8302_enable_calibration(self->drv_pitch, true);
	drv8302_enable_calibration(self->drv_yaw, true);

	thread_sleep_ms(10);

	printk("Performing dc calibration...\n");

	int loops = 10;
	for (int c = 0; c < loops; c++) {
		uint16_t pa = 0, pb = 0, ya = 0, yb = 0;

		adc_read(self->inputs.adc, FB_ADC_IA1_CHAN, &ya);
		adc_read(self->inputs.adc, FB_ADC_IB1_CHAN, &yb);
		adc_read(self->inputs.adc, FB_ADC_IA2_CHAN, &pa);
		adc_read(self->inputs.adc, FB_ADC_IB2_CHAN, &pb);

		self->config.dc_cal.pitch_a += pa;
		self->config.dc_cal.pitch_b += pb;
		self->config.dc_cal.yaw_a += ya;
		self->config.dc_cal.yaw_b += yb;

		thread_sleep_ms(10);
	}

	self->config.dc_cal.pitch_a /= loops;
	self->config.dc_cal.pitch_b /= loops;
	self->config.dc_cal.yaw_a /= loops;
	self->config.dc_cal.yaw_b /= loops;

	printk("Current sensing calibration values: %d %d %d %d\n", self->config.dc_cal.pitch_a,
	       self->config.dc_cal.pitch_b, self->config.dc_cal.yaw_a, self->config.dc_cal.yaw_b);

	if (abs(self->config.dc_cal.pitch_a - 2048) > 100 ||
	    abs(self->config.dc_cal.pitch_b - 2048) > 100) {
		printk(PRINT_ERROR "Pitch motor calibration values too low! Check power stage!\n");
	}

	if (abs(self->config.dc_cal.yaw_a - 2048) > 100 ||
	    abs(self->config.dc_cal.yaw_b - 2048) > 100) {
		printk(PRINT_ERROR "Yaw motor calibration values too low! Check power stage!\n");
	}

	drv8302_enable_calibration(self->drv_pitch, false);
	drv8302_enable_calibration(self->drv_yaw, false);
}

//! Check connected peripherals
/*!
  Checks the status of all connected devices and prints it out on the console

  @param self main application object
*/
static void _fb_check_connected_devices(struct fb *self)
{
	if (self->mode == FB_MODE_MASTER) {
		// check that all leds are connected
		const struct {
			const char *name;
			unsigned int led_idx;
			unsigned int sw_idx;
		} leds[5] = { { .name = "HOME", .led_idx = 3, .sw_idx = 11 },
			      { .name = "PRESET1", .led_idx = 4, .sw_idx = 12 },
			      { .name = "PRESET2", .led_idx = 5, .sw_idx = 13 },
			      { .name = "PRESET3", .led_idx = 6, .sw_idx = 14 },
			      { .name = "PRESET4", .led_idx = 7, .sw_idx = 15 } };
		for (unsigned c = 0; c < sizeof(leds) / sizeof(leds[0]); c++) {
			float volts = 0;
			int r = analog_read(self->sw_leds, leds[c].led_idx, &volts);
			if (gpio_read(self->inputs.sw_gpio, 8 + c)) {
				printk("SW %s: OK, ", leds[c].name);
			} else {
				printk("SW %s: FAIL, ", leds[c].name);
			}
			if (r != 0 || volts < 0.5) {
				printk("LED: FAIL\n", leds[c].name);
			} else {
				printk("LED: OK\n", leds[c].name);
			}
		}
	}
}
#if 0
static int _fb_events_handler(struct events_subscriber *sub, uint32_t ev) {
	struct fb *self = container_of(sub, struct fb, sub);
	//gpio_set(self->debug_gpio, 4);
	//gpio_reset(self->debug_gpio, 4);
	return 0;
}
#endif
void fb_init(struct fb *self)
{
	self->state.prev_loop_time = timestamp();
	self->slave.timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
	self->remote.pitch_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
	self->remote.yaw_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);

	thread_mutex_init(&self->slave.lock);
	thread_mutex_init(&self->measured.lock);
	thread_mutex_init(&self->stats.lock);

	fb_inputs_init(&self->inputs);

	fb_config_init(&self->config);

	fb_cmd_init(self);

	fb_control_init(&self->axis[FB_AXIS_UPDOWN], &self->config.axis[FB_AXIS_UPDOWN]);
	fb_control_init(&self->axis[FB_AXIS_LEFTRIGHT], &self->config.axis[FB_AXIS_LEFTRIGHT]);

	fb_leds_reset(&self->button_leds);

	static const struct fb_config_filter _pfilt = {
		.a0 = -1.44739, .a1 = 0.567971, .b0 = 0.0659765, .b1 = 0.0546078
	};

	// static const struct fb_config_filter _pfilt = {.a0 = 0, .a1 = 0, .b0 = 1, .b1 =
	// 0};
	fb_filter_init(&self->measured.pfilt, &_pfilt);

	analog_write(self->oc_pot, OC_POT_OC_ADJ_MOTOR1, 1.f);
	analog_write(self->oc_pot, OC_POT_OC_ADJ_MOTOR2, 1.f);

	led_on(self->leds, 0);
	thread_sleep_ms(500);
	led_off(self->leds, 0);
	thread_sleep_ms(500);

	led_on(self->leds, 0);
	led_off(self->leds, 1);
	led_on(self->leds, 2);

	thread_sleep_ms(100);

	/*
	events_subscriber_init(&self->sub, _fb_events_handler);
	events_subscribe(self->events, FB_EVENT_MOTOR1_UPDATE, &self->sub);
	events_subscribe(self->events, FB_EVENT_MOTOR2_UPDATE, &self->sub);
	*/

	// default presets
	/*
	self->config.presets[1].pitch = 0.5f;
	self->config.presets[1].yaw = 3.0f;
	self->config.presets[1].valid = true;

	self->config.presets[2].pitch = -0.5;
	self->config.presets[2].yaw = -1.0f;
	self->config.presets[2].valid = true;
	*/

	fb_inputs_update(&self->inputs);
	_fb_update_measurements(self);
	fb_can_init(self);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	_fb_calibrate_current_sensors(self);

	_fb_check_connected_devices(self);

	_fb_enter_state(self, _fb_state_operational);

	thread_create(_fb_control_loop, "fb_ctrl", 750, self, 2, NULL);

	thread_create(_fb_indicator_loop, "fb_ind", 250, self, 1, NULL);
}
static int _fb_probe(void *fdt, int fdt_node)
{
	struct fb *self = (struct fb *)kzmalloc(sizeof(struct fb));

	BUG_ON(!self);

	gpio_device_t sw_gpio = gpio_find_by_ref(fdt, fdt_node, "sw_gpio");
	gpio_device_t gpio_ex = gpio_find_by_ref(fdt, fdt_node, "gpio_ex");
	adc_device_t adc = adc_find_by_ref(fdt, fdt_node, "adc");
	analog_device_t mot_x = analog_find_by_ref(fdt, fdt_node, "mot_x");
	analog_device_t mot_y = analog_find_by_ref(fdt, fdt_node, "mot_y");
	analog_device_t sw_leds = analog_find_by_ref(fdt, fdt_node, "sw_leds");
	console_device_t console = console_find_by_ref(fdt, fdt_node, "console");
	gpio_device_t mux = gpio_find_by_ref(fdt, fdt_node, "mux");
	led_controller_t leds = leds_find_by_ref(fdt, fdt_node, "leds");
	encoder_device_t enc1 = encoder_find_by_ref(fdt, fdt_node, "enc1");
	encoder_device_t enc2 = encoder_find_by_ref(fdt, fdt_node, "enc2");
	memory_device_t eeprom = memory_find_by_ref(fdt, fdt_node, "eeprom");
	analog_device_t oc_pot = analog_find_by_ref(fdt, fdt_node, "oc_pot");
	analog_device_t an_out = analog_find_by_ref(fdt, fdt_node, "an_out");
	can_device_t can1 = can_find_by_ref(fdt, fdt_node, "can1");
	memory_device_t can1_mem = memory_find_by_ref(fdt, fdt_node, "can1");
	can_device_t can2 = can_find_by_ref(fdt, fdt_node, "can2");
	memory_device_t can2_mem = memory_find_by_ref(fdt, fdt_node, "can2");
	regmap_device_t regmap = regmap_find_by_ref(fdt, fdt_node, "regmap");
	memory_device_t canopen_mem = memory_find_by_ref(fdt, fdt_node, "canopen");
	gpio_device_t enc1_gpio = gpio_find_by_ref(fdt, fdt_node, "enc1_gpio");
	gpio_device_t enc2_gpio = gpio_find_by_ref(fdt, fdt_node, "enc2_gpio");
	drv8302_t drv_pitch = memory_find_by_ref(fdt, fdt_node, "drv_pitch");
	drv8302_t drv_yaw = memory_find_by_ref(fdt, fdt_node, "drv_yaw");
	events_device_t events = events_find_by_ref(fdt, fdt_node, "events");

	if (!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y) {
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if (!eeprom) {
		printk(PRINT_ERROR "fb: eeprom missing\n");
		return -1;
	}
	if (!console) {
		printk(PRINT_ERROR "fb: console missing\n");
		return -1;
	}
	if (!oc_pot) {
		printk(PRINT_ERROR "fb: oc_pot missing\n");
		return -1;
	}
	if (!an_out) {
		printk(PRINT_ERROR "fb: an_out missing\n");
		//return -1;
	}

	if (!can1) {
		printk(PRINT_ERROR "fb: can1 missing\n");
		return -1;
	}
	if (!can2) {
		printk(PRINT_ERROR "fb: can2 missing\n");
		return -1;
	}
	if (!regmap) {
		printk(PRINT_ERROR "fb: regmap missing\n");
		return -1;
	}
	if (!canopen_mem) {
		printk(PRINT_ERROR "fb: canopen missing\n");
		return -1;
	}
	if (!enc1) {
		printk(PRINT_ERROR "fb: enc1 missing\n");
		return -1;
	}
	if (!enc2) {
		printk(PRINT_ERROR "fb: enc2 missing\n");
		return -1;
	}
	if (!enc1_gpio) {
		printk(PRINT_ERROR "fb: enc1_gpio missing\n");
		return -1;
	}
	if (!enc2_gpio) {
		printk(PRINT_ERROR "fb: enc2_gpio missing\n");
		return -1;
	}
	if (!gpio_ex) {
		printk(PRINT_ERROR "fb: gpio_ex missing\n");
		return -1;
	}
	if (!drv_pitch) {
		printk(PRINT_ERROR "fb: drv_pitch missing\n");
		return -1;
	}
	if (!drv_yaw) {
		printk(PRINT_ERROR "fb: drv_yaw missing\n");
		return -1;
	}

	self->inputs.sw_gpio = sw_gpio;
	self->inputs.gpio_ex = gpio_ex;
	self->inputs.enc1 = enc1;
	self->inputs.enc2 = enc2;
	self->inputs.enc1_gpio = enc1_gpio;
	self->inputs.enc2_gpio = enc2_gpio;
	self->inputs.mux = mux;
	self->inputs.adc = adc;

	self->leds = leds;
	self->console = console;
	self->sw_leds = sw_leds;
	self->mot_x = mot_x;
	self->mot_y = mot_y;
	self->eeprom = eeprom;
	self->oc_pot = oc_pot;
	self->an_out = an_out;
	self->can1 = can1;
	self->can1_mem = can1_mem;
	self->can2 = can2;
	self->can2_mem = can2_mem;
	self->regmap = regmap_device_to_regmap(regmap);
	self->canopen_mem = canopen_mem;
	self->drv_pitch = drv_pitch;
	self->drv_yaw = drv_yaw;
	self->events = events;
	//self->debug_gpio = gpio_find_by_ref(fdt, fdt_node, "debug_gpio");
	// self->canopen = canopen;

	fb_init(self);

	return 0;
}

static int _fb_remove(void *fdt, int fdt_node)
{
	// TODO
	return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _fb_probe, _fb_remove)
