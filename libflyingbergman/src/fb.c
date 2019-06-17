/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dts/flyingbergman.h"
#include "fb.h"
#include "fb_cmd.h"
#include "fb_state.h"
#include "fb_can.h"
#include <libfdt/libfdt.h>

static void _fb_indicator_loop(void *ptr) {
	struct fb *self = (struct fb *)ptr;
	uint32_t t = thread_ticks_count();
	usec_t blink_us = 100000;
	timestamp_t blink_delay = timestamp();
	bool blink_state = false;
	while(1) {
		if(timestamp_expired(blink_delay)) {
			blink_state = !blink_state;
			blink_delay = timestamp_from_now_us(blink_us);
			if(blink_state) {
				led_on(self->leds, 0);
			} else {
				led_off(self->leds, 0);
			}
		}
		for(unsigned c = 0; c < FB_LED_COUNT; c++) {
			analog_write(self->sw_leds, c, fb_leds_get_intensity(&self->button_leds, c));
		}
		thread_sleep_ms_until(&t, 1000 / 25);
	}
}

void _fb_enter_state(struct fb *self, void (*fp)(struct fb *self, float dt)) {
	// enter new state
	if(fp == _fb_state_wait_power) {
		fb_leds_reset(&self->button_leds);
		// blink the led quickly
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_SLOW_RAMP);
		self->control_mode = FB_CONTROL_MODE_NO_MOTION;
	}
	if(fp == _fb_state_wait_home) {
		fb_leds_reset(&self->button_leds);
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_FAST_RAMP);
		self->control_mode = FB_CONTROL_MODE_MANUAL;
	} else if(fp == _fb_state_operational) {
		_fb_state_operational_enter(self);
	}

	self->state.fn = fp;
}

void _fb_state_wait_power(struct fb *self, float dt) {
	// in this state joystick does not move the motors and device waits for user to
	// toggle home button once
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if(home->toggled) {
		printk("fb: move motors to desired home position\n");
		// once home button is toggled once we enter the state where motors can be moved
		// but no other controls are available
		_fb_enter_state(self, _fb_state_wait_home);
	}
}

void _fb_state_wait_home(struct fb *self, float dt) {
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if(home->pressed) {
		// make sure home button is lit
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_ON);
		// if it has been held for a number of seconds then we save the home position
		timestamp_t ts = timestamp_add_us(home->pressed_time, FB_BTN_LONG_PRESS_TIME_US);
		if(timestamp_expired(ts)) {
			printk("fb: saving home position %d %d\n",
			       (int32_t)(self->measured.pitch * 1000),
			       (int32_t)(self->measured.yaw * 1000));
			self->config.presets[0].pitch = 0; // self->measured.pitch;
			self->config.presets[0].yaw = self->measured.yaw;
			self->config.presets[0].valid = true;
			_fb_enter_state(self, _fb_state_operational);
		}
	} else {
		fb_leds_set_state(&self->button_leds, FB_LED_HOME, FB_LED_STATE_FAST_RAMP);
	}
}

void _fb_state_save_preset(struct fb *self, float dt) {
	struct fb_switch_state *sw[] = {
	    &self->inputs.sw[FB_SW_HOME], &self->inputs.sw[FB_SW_PRESET_1],
	    &self->inputs.sw[FB_SW_PRESET_2], &self->inputs.sw[FB_SW_PRESET_3],
	    &self->inputs.sw[FB_SW_PRESET_4]};

	for(int c = 0; c < FB_PRESET_COUNT; c++) {
		if(!sw[c]->pressed && sw[c]->toggled) {
			_fb_enter_state(self, _fb_state_operational);
		}
	}
}

void _fb_update_state(struct fb *self, float dt) {
	self->state.fn(self, dt);

	// FIXME: this is wrong place to put this
	if(self->mode == FB_MODE_SLAVE) {
		if(timestamp_expired(self->remote.pitch_update_timeout) ||
		   timestamp_expired(self->remote.yaw_update_timeout)) {
			if(self->control_mode == FB_CONTROL_MODE_REMOTE) {
				self->control_mode = FB_CONTROL_MODE_NO_MOTION;
			}
			self->remote.pitch_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
			self->remote.yaw_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
		} else if(self->local.pitch != 0 || self->local.yaw != 0) {
			self->control_mode = FB_CONTROL_MODE_LOCAL;
		} else {
			self->control_mode = FB_CONTROL_MODE_REMOTE;
		}
	}
}

//! this function constrains the demanded pitch and yaw output to be compliant with
//! controls for speed and intensity
static void _fb_output_constrained(struct fb *self, float demand_pitch,
                                   float demand_yaw, float pitch_acc, float yaw_acc,
                                   float dt) {
	demand_pitch = constrain_float(demand_pitch, -1.f, 1.f);
	demand_yaw = constrain_float(demand_yaw, -1.f, 1.f);

	float pitch = self->output.pitch;
	if(pitch < demand_pitch)
		pitch = constrain_float(pitch + 4.f * pitch_acc * dt, pitch, demand_pitch);
	else if(pitch > demand_pitch)
		pitch = constrain_float(pitch - 4.f * pitch_acc * dt, demand_pitch, pitch);

	float yaw = self->output.yaw;
	if(yaw < demand_yaw)
		yaw = constrain_float(yaw + 4.f * yaw_acc * dt, yaw, demand_yaw);
	else if(yaw > demand_yaw)
		yaw = constrain_float(yaw - 4.f * yaw_acc * dt, demand_yaw, yaw);

	float pitch_speed = self->measured.pitch_speed / FB_PITCH_MAX_VEL;
	float yaw_speed = self->measured.yaw_speed / FB_YAW_MAX_VEL;

	pitch = constrain_float(pitch, -pitch_speed, pitch_speed);
	yaw = constrain_float(yaw, -yaw_speed, yaw_speed);

	// limit pitch if measured pitch is outside of allowed limit
	if(self->measured.pitch > self->config.axis[FB_AXIS_UPDOWN].limits.pos_max) {
		pitch = constrain_float(pitch, 0, 1.f);
	} else if(self->measured.pitch <
	          self->config.axis[FB_AXIS_UPDOWN].limits.pos_min) {
		pitch = constrain_float(pitch, -1.f, 0);
	}

	// write final output value
	self->output.pitch = pitch;
	self->output.yaw = yaw;
}

static void _fb_update_control(struct fb *self, float dt) {
	switch(self->control_mode) {
		case FB_CONTROL_MODE_NO_MOTION: {
			self->output.pitch = 0;
			self->output.yaw = 0;
		} break;
		case FB_CONTROL_MODE_LOCAL: {
			float pitch = (float)self->local.pitch * 0.0005f;
			float yaw = (float)self->local.yaw * 0.0005f;
			_fb_output_constrained(self, pitch, yaw, 1, 1, dt);
		} break;
		case FB_CONTROL_MODE_REMOTE: {
			/**
			 * In this mode motors are controlled remotely over manufacturer segment
			 * It is still however important to limit change in output so that we do not
			 * suddenly stop the motor if remote communication is lost
			 */
			float pitch = (float)self->remote.pitch * 0.0005f;
			float yaw = (float)self->remote.yaw * 0.0005f;
			_fb_output_constrained(self, pitch, yaw, self->measured.pitch_acc,
			                       self->measured.yaw_acc, dt);
		} break;
		case FB_CONTROL_MODE_MANUAL: {
			/**
			 * In manual mode we generate output directly based on joystick input
			 */
			_fb_output_constrained(self, self->measured.joy_pitch, self->measured.joy_yaw,
			                       self->measured.pitch_acc, self->measured.yaw_acc, dt);
		} break;
		case FB_CONTROL_MODE_AUTO: {
			/*
			 * In auto mode we generate a trajectory that gives us a target velocity and
			 * target position for each frame The control output signal is then generated
			 * based on error between trajectory setpoint and actual position/velocity User
			 * controls are added to the control action to still make it possible to
			 * control using joystick
			 */
			float co_pitch = fb_control_get_output(&self->axis[FB_AXIS_UPDOWN]);
			float co_yaw = fb_control_get_output(&self->axis[FB_AXIS_LEFTRIGHT]);

			_fb_output_constrained(self, -co_pitch, -co_yaw, 1, 1, dt);

			// if joystick is moved during automatic move then we switch back to manual
			// mode
			if(fabsf(self->measured.joy_pitch) > 0.1 ||
			   fabsf(self->measured.joy_yaw) > 0.1) {
				self->control_mode = FB_CONTROL_MODE_MANUAL;
			}
		} break;
	}
}

static void _fb_update_motors(struct fb *self) {
	float pitch = self->output.pitch;
	// FIXME: remove the 2.0f here
	float yaw = self->output.yaw * 2.f;


	pitch = constrain_float(pitch * 0.5, -0.5, 0.5);
	yaw = constrain_float(yaw * 0.5, -0.5, 0.5);

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

static bool _fb_slave_timed_out(struct fb *self) {
	timestamp_t timeout = timestamp_from_now_ms(FB_SLAVE_TIMEOUT_MS);

	uint32_t mask = FB_SLAVE_VALID_PITCH | FB_SLAVE_VALID_YAW |
	                FB_SLAVE_VALID_I_PITCH | FB_SLAVE_VALID_I_YAW;

	thread_mutex_lock(&self->slave.lock);
	if((self->slave.valid_bits & mask) == mask) {
		// if all items were received then reset the bits and update timestamp
		self->slave.valid_bits = 0;
		self->slave.timeout = timeout;
		fb_leds_set_state(&self->button_leds, FB_LED_CONN_STATUS, FB_LED_STATE_OFF);
	} else {
		// otherwise wait until timeout and reconfigure slave
		if(timestamp_expired(self->slave.timeout)) {
			self->slave.valid_bits = 0;
			self->slave.timeout = timeout;
			fb_leds_set_state(&self->button_leds, FB_LED_CONN_STATUS, FB_LED_STATE_ON);
			thread_mutex_unlock(&self->slave.lock);
			return true;
		}
	}
	thread_mutex_unlock(&self->slave.lock);
	return false;
}

static void _fb_monitor_slaves(struct fb *self) {
	if(self->mode != FB_MODE_MASTER)
		return;

	if(_fb_slave_timed_out(self)) {
		_fb_enter_state(self, _fb_state_wait_power);
		fb_reinit_can_slave(self);
	}
}

static void _fb_control_loop(void *ptr) {
	struct fb *self = (struct fb *)ptr;

	uint32_t ticks = thread_ticks_count();
	while(1) {
		timestamp_t now = timestamp();
		timestamp_diff_t diff = timestamp_sub(now, self->state.prev_loop_time);
		float dt = (float)(diff.usec) * 1e-6;
		self->state.prev_loop_time = now;

		_fb_read_inputs(self);
		_fb_monitor_slaves(self);
		_fb_update_measurements(self);
		_fb_update_state(self, dt);
		_fb_update_control(self, dt);
		_fb_update_motors(self);

		thread_sleep_ms_until(&ticks, 1);
	}
}

static void _fb_calibrate_current_sensors(struct fb *self) {
	self->config.dc_cal.pitch_a = 0;
	self->config.dc_cal.pitch_b = 0;
	self->config.dc_cal.yaw_a = 0;
	self->config.dc_cal.yaw_b = 0;

	drv8302_enable_calibration(self->drv_pitch, true);
	drv8302_enable_calibration(self->drv_yaw, true);

	thread_sleep_ms(10);

	printk("Performing dc calibration...\n");

	int loops = 10;
	for(int c = 0; c < loops; c++) {
		uint16_t pa = 0, pb = 0, ya = 0, yb = 0;

		adc_read(self->adc, FB_ADC_IA1_CHAN, &ya);
		adc_read(self->adc, FB_ADC_IB1_CHAN, &yb);
		adc_read(self->adc, FB_ADC_IA2_CHAN, &pa);
		adc_read(self->adc, FB_ADC_IB2_CHAN, &pb);

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

	printk("Current sensing calibration values: %d %d %d %d\n",
	       self->config.dc_cal.pitch_a, self->config.dc_cal.pitch_b,
	       self->config.dc_cal.yaw_a, self->config.dc_cal.yaw_b);

	if(abs(self->config.dc_cal.pitch_a - 2048) > 100 ||
	   abs(self->config.dc_cal.pitch_b - 2048) > 100) {
		printk(PRINT_ERROR
		       "Pitch motor calibration values too low! Check power stage!\n");
	}

	if(abs(self->config.dc_cal.yaw_a - 2048) > 100 ||
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
static void _fb_check_connected_devices(struct fb *self) {
	if(self->mode == FB_MODE_MASTER) {
		// check that all leds are connected
		const struct {
			const char *name;
			unsigned int led_idx;
			unsigned int sw_idx;
		} leds[5] = {{.name = "HOME", .led_idx = 3, .sw_idx = 11},
		             {.name = "PRESET1", .led_idx = 4, .sw_idx = 12},
		             {.name = "PRESET2", .led_idx = 5, .sw_idx = 13},
		             {.name = "PRESET3", .led_idx = 6, .sw_idx = 14},
		             {.name = "PRESET4", .led_idx = 7, .sw_idx = 15}};
		for(unsigned c = 0; c < sizeof(leds) / sizeof(leds[0]); c++) {
			float volts = 0;
			int r = analog_read(self->sw_leds, leds[c].led_idx, &volts);
			if(gpio_read(self->sw_gpio, 8 + c)) {
				printk("SW %s: OK, ", leds[c].name);
			} else {
				printk("SW %s: FAIL, ", leds[c].name);
			}
			if(r != 0 || volts < 0.5) {
				printk("LED: FAIL\n", leds[c].name);
			} else {
				printk("LED: OK\n", leds[c].name);
			}
		}
	}
}

static int _fb_events_handler(struct events_subscriber *sub, uint32_t ev) {
	struct fb *self = container_of(sub, struct fb, sub);
	gpio_set(self->debug_gpio, 4);
	gpio_reset(self->debug_gpio, 4);
	return 0;
}

static int _fb_probe(void *fdt, int fdt_node) {
	struct fb *self = kzmalloc(sizeof(struct fb));

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

	if(!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y) {
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if(!eeprom) {
		printk(PRINT_ERROR "fb: eeprom missing\n");
		return -1;
	}
	if(!console) {
		printk(PRINT_ERROR "fb: console missing\n");
		return -1;
	}
	if(!oc_pot) {
		printk(PRINT_ERROR "fb: oc_pot missing\n");
		return -1;
	}
	if(!can1) {
		printk(PRINT_ERROR "fb: can1 missing\n");
		return -1;
	}
	if(!can2) {
		printk(PRINT_ERROR "fb: can2 missing\n");
		return -1;
	}
	if(!regmap) {
		printk(PRINT_ERROR "fb: regmap missing\n");
		return -1;
	}
	if(!canopen_mem) {
		printk(PRINT_ERROR "fb: canopen missing\n");
		return -1;
	}
	if(!enc1) {
		printk(PRINT_ERROR "fb: enc1 missing\n");
		return -1;
	}
	if(!enc2) {
		printk(PRINT_ERROR "fb: enc2 missing\n");
		return -1;
	}
	if(!enc1_gpio) {
		printk(PRINT_ERROR "fb: enc1_gpio missing\n");
		return -1;
	}
	if(!enc2_gpio) {
		printk(PRINT_ERROR "fb: enc2_gpio missing\n");
		return -1;
	}
	if(!gpio_ex) {
		printk(PRINT_ERROR "fb: gpio_ex missing\n");
		return -1;
	}
	if(!drv_pitch) {
		printk(PRINT_ERROR "fb: drv_pitch missing\n");
		return -1;
	}
	if(!drv_yaw) {
		printk(PRINT_ERROR "fb: drv_yaw missing\n");
		return -1;
	}

	fb_cmd_init(self);

	thread_mutex_init(&self->slave.lock);
	thread_mutex_init(&self->inputs.lock);
	thread_mutex_init(&self->measured.lock);

	self->leds = leds;
	self->console = console;
	self->sw_gpio = sw_gpio;
	self->gpio_ex = gpio_ex;
	self->sw_leds = sw_leds;
	self->adc = adc;
	self->mot_x = mot_x;
	self->mot_y = mot_y;
	self->mux = mux;
	self->enc1 = enc1;
	self->enc2 = enc2;
	self->eeprom = eeprom;
	self->oc_pot = oc_pot;
	self->can1 = can1;
	self->can1_mem = can1_mem;
	self->can2 = can2;
	self->can2_mem = can2_mem;
	self->regmap = regmap;
	self->canopen_mem = canopen_mem;
	self->enc1_gpio = enc1_gpio;
	self->enc2_gpio = enc2_gpio;
	self->drv_pitch = drv_pitch;
	self->drv_yaw = drv_yaw;
	self->events = events;
	self->debug_gpio = gpio_find_by_ref(fdt, fdt_node, "debug_gpio");
	// self->canopen = canopen;

	self->state.prev_loop_time = timestamp();
	self->slave.timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
	self->remote.pitch_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
	self->remote.yaw_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);

	fb_config_init(&self->config);
	fb_control_init(&self->axis[FB_AXIS_UPDOWN], &self->config.axis[FB_AXIS_UPDOWN]);
	fb_control_init(&self->axis[FB_AXIS_LEFTRIGHT],
	                &self->config.axis[FB_AXIS_LEFTRIGHT]);

	fb_leds_reset(&self->button_leds);

	static const struct fb_config_filter _pfilt = {
		    .a0 = -1.44739, .a1 = 0.567971, .b0 = 0.0659765, .b1 = 0.0546078};
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

	_fb_read_inputs(self);
	_fb_update_measurements(self);
	fb_init_can(self);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	_fb_calibrate_current_sensors(self);

	_fb_check_connected_devices(self);

	_fb_enter_state(self, _fb_state_wait_power);

	thread_create(_fb_control_loop, "fb_ctrl", 650, self, 2, NULL);

	thread_create(_fb_indicator_loop, "fb_ind", 250, self, 1, NULL);

	return 0;
}

static int _fb_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _fb_probe, _fb_remove)
