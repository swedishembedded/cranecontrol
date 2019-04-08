#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

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

#include <libfdt/libfdt.h>

#define FB_SWITCH_COUNT 8
#define FB_LED_COUNT 8
#define FB_PRESET_COUNT 4
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

#define FB_LED_STATE_OFF 0
#define FB_LED_STATE_SLOW_RAMP 1
#define FB_LED_STATE_ON 2
#define FB_LED_STATE_3BLINKS_OFF 3
#define FB_LED_STATE_FAST_RAMP 4
#define FB_LED_STATE_FAINT_ON 5

#define FB_ADC_MUX_CHANNEL 14

#define FB_HOME_LONG_PRESS_TIME_US 1000000UL

#define FB_PRESET_BIT_VALID (1 << 0)

#define FB_REMOTE_TIMEOUT 50000

#define CANOPEN_FB_MOTOR_PITCH 0x200100
#define CANOPEN_FB_MOTOR_YAW 0x200200

struct fb_config {
	struct config_preset {
		uint8_t flags;
		float pitch, yaw;
		bool valid;
	} presets[FB_PRESET_COUNT];
	struct {
		float pitch;
		float yaw;
	} home;
};

struct application {
    led_controller_t leds;
	console_device_t console;
	gpio_device_t sw_gpio;
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
	memory_device_t motor_mem;
	memory_device_t can1_mem;
	memory_device_t can2_mem;
	gpio_device_t enc1_gpio;
	gpio_device_t enc2_gpio;

	struct fb_config config;
	struct {
		timestamp_t prev_micros;
		float pitch, yaw;
		struct fb_switch_state {
			bool pressed;
			bool toggled;
			timestamp_t pressed_time;
		} sw[FB_SWITCH_COUNT];
		struct fb_led_state {
			float intensity;
			int dim;
			int state;
			int blinks;
		} leds[FB_LED_COUNT];
		float pitch_target, yaw_target;
		float prev_err_pitch, prev_err_yaw;
		float pitch_i, yaw_i;

		bool enable_motors;
		bool remote;

		void (*fn)(struct application *self, float dt);
	} state;

	struct {
		float pitch_rad, yaw_rad;
	} actual;

	struct {
		float yaw, pitch;
		float yaw_acc, pitch_acc;
		float yaw_speed, pitch_speed;
	} controls;

	struct {
		int16_t pitch, yaw;
	} output;

	struct {
		uint16_t pitch, yaw;
		timestamp_t last_pitch_update, last_yaw_update;
	} remote;

	int mux_chan;
	int16_t mux_adc[8];

	struct regmap_range comm_range, mfr_range, mfr_range_slave;
};

enum {
	FB_ADC_YAW_CHAN = 0,
	FB_ADC_PITCH_CHAN = 1,
	FB_ADC_YAW_SPEED_CHAN = 2,
	FB_ADC_YAW_ACC_CHAN = 3,
	FB_ADC_PITCH_ACC_CHAN = 4,
	FB_ADC_PITCH_SPEED_CHAN = 5,
	FB_ADC_CHAN_RESERVED15 = 6,
	FB_ADC_MOTOR_PITCH_CHAN = 7
};

static int _fb_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 2 && strcmp(argv[1], "sw") == 0){
		for(int c = 0; c < 8; c++){
			int val = gpio_read(self->sw_gpio, (uint32_t)(8+c));
			console_printf(con, "SW%d: %d\n", c, val);
		}
	} else if(argc == 2 && strcmp(argv[1], "stats") == 0){
		console_printf(con, "SW: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "[%2d] %d ", c, self->state.sw[c].pressed);
		}
		console_printf(con, "\n");

		console_printf(con, "AIN: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "[%2d] %5d ", c, self->mux_adc[c]);
		}
		console_printf(con, "\n");

		console_printf(con, "ADC: ");
		for(unsigned c = 0; c < 16; c++){
			int16_t val = 0;
			adc_read(self->adc, c, &val);
			console_printf(con, "[%2d] %5d ", c, val);
		}
		console_printf(con, "\n");

		console_printf(con, "ENC1: COUNT %d, Di2 %d, Di3 %d\n",
				encoder_read(self->enc1),
				gpio_read(self->enc1_gpio, 1),
				gpio_read(self->enc1_gpio, 2)
		);
		console_printf(con, "ENC2: COUNT %d, Di2 %d, Di3 %d\n",
				encoder_read(self->enc2),
				gpio_read(self->enc2_gpio, 1),
				gpio_read(self->enc2_gpio, 2)
		);

		console_printf(con, "JOYSTICK:\tYAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw * 1000),
				(int32_t)(self->controls.pitch * 1000)
		);
		console_printf(con, "INTENSITY:\tYAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw_acc * 1000),
				(int32_t)(self->controls.pitch_acc * 1000)
		);
		console_printf(con, "SPEED:\t\tYAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw_speed * 1000),
				(int32_t)(self->controls.pitch_speed * 1000)
		);
		console_printf(con, "ACTUAL:\t\tYAW %5d PITCH %5d\n",
				(int32_t)(self->actual.yaw_rad * 1000),
				(int32_t)(self->actual.pitch_rad * 1000)
		);
		console_printf(con, "REMOTE:\t\tYAW %5d PITCH %5d\n",
				self->remote.yaw,
				self->remote.pitch
		);

	} else if(argc == 2 && strcmp(argv[1], "enc") == 0){
		int32_t val = encoder_read(self->enc1);
		console_printf(con, "ENC1: %d\n", val);
	} else if(argc == 2 && strcmp(argv[1], "ee") == 0){
		console_printf(con, "Writing eeprom\n");
		static uint8_t buf[] = { 1, 2, 3, 4 };
		memory_write(self->eeprom, 0, buf, 4);
		console_printf(con, "Reading eeprom\n");
		static uint8_t rbuf[4];
		memory_read(self->eeprom, 0, rbuf, 4);
		for(size_t c = 0; c < 4; c++){
			console_printf(con, "%d\n", rbuf[c]);
		}
	} else if(argc == 2 && strcmp(argv[1], "can") == 0){
		struct can_message msg;
		msg.id = 0xaa;
		msg.len = 1;
		msg.data[0] = 0x12;
		//can_send(self->can1, &msg, 100);
		for(int c = 0; c < 100; c++){
			int err = can_send(self->can2, &msg, 100);
			if(err < 0){
				printk(PRINT_ERROR "can error: %d %s\n", err, strerror(-err));
			}
		}
	} else {
		console_printf(con, "Invalid option\n");
	}

	return 0;
}

uint32_t irq_get_count(int irq);

static int _reg_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 3 && strcmp(argv[1], "get") == 0){
		uint32_t value;
		unsigned int id = 0;
		sscanf(argv[2], "%x", &id);
		if(regmap_read_u32(self->regmap, (uint32_t)id, &value) < 0){
			console_printf(con, PRINT_ERROR "specified register not found\n");
		} else {
			console_printf(con, "%08x=%08x\n", id, value);
		}
	} else if(argc == 4 && strcmp(argv[1], "set") == 0){
		uint32_t value = 0;
		uint32_t id = 0;
		sscanf(argv[2], "%x", (unsigned int*)&id);
		sscanf(argv[3], "%x", (unsigned int*)&value);
		regmap_write_u32(self->regmap, id, value);
	} else {
		return -1;
	}
	return 0;
}

static int _motor_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 2 && strcmp(argv[1], "info") == 0){
		uint32_t addr = 0x02000000;
		uint32_t type = 0, error = 0, mfr_status = 0;
		int ret = 0;
#define _read(reg, value) ret |= memory_read(self->motor_mem, addr | reg, &value, sizeof(value))
		_read(CANOPEN_REG_DEVICE_TYPE, type);
		_read(CANOPEN_REG_DEVICE_ERROR, error);
		_read(CANOPEN_REG_DEVICE_MFR_STATUS, mfr_status);
#undef _read
		printk("Type: %d\n", type);
		printk("Error: %08x\n", error);
		printk("MFR Status: %08x\n", mfr_status);
	} else if(argc == 2 && strcmp(argv[1], "regs") == 0){
		for(uint32_t c = 0; c < 0x1fff00; c+=0x0100){
			uint32_t reg = 0;
			uint32_t ofs = 0x01000000 | c;
			if(memory_read(self->motor_mem, ofs, &reg, sizeof(reg)) >= 0){
				console_printf(con, "%08x=%08x\n", ofs, reg);
			} else {
				console_printf(con, "%08x=--------\n", ofs);
			}
		}
	} else {
		return -EINVAL;
	}
	return 0;
}

static int _can_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;

	memory_device_t mem = 0;
	if(strcmp(argv[0], "can1") == 0) mem = self->can1_mem;
	if(strcmp(argv[0], "can2") == 0) mem = self->can2_mem;
	if(!mem) return -EINVAL;

	if(argc == 2 && strcmp(argv[1], "info") == 0){
		struct can_counters cnt;
		memory_read(mem, 0, &cnt, sizeof(cnt));
		console_printf(con, "\tTX count: %d\n", cnt.tme); 
		console_printf(con, "\tTX dropped: %d\n", cnt.txdrop);
		console_printf(con, "\tRX count: %d\n", cnt.rxp); 
		console_printf(con, "\tRX dropped: %d\n", cnt.rxdrop);
		console_printf(con, "\tTX timeout: %d\n", cnt.txto);
		console_printf(con, "\tRX on FIFO0: %d\n", cnt.fmp0);
		console_printf(con, "\tRX on FIFO1: %d\n", cnt.fmp1);
		console_printf(con, "\tTotal errors: %d\n", cnt.lec);
		console_printf(con, "\tBus off errors: %d\n", cnt.bof);
		console_printf(con, "\tBus passive errors: %d\n", cnt.epv);
		console_printf(con, "\tBus errors warnings: %d\n", cnt.ewg);
		console_printf(con, "\tFIFO Overflow errors: %d\n", cnt.fov);
	}
	return 0;
}

static void _fb_indicator_loop(void *ptr){
	struct application *self = (struct application *)ptr;
	uint32_t t = thread_ticks_count();
	uint32_t blink_us = 100000;
	timestamp_t blink_delay = micros() + blink_us;
	bool blink_state = false;
	while(1){
		if(time_after(micros(), blink_delay)){
			blink_state = !blink_state;
			blink_delay = micros() + blink_us;
			if(blink_state){
				led_on(self->leds, 0);
			} else {
				led_off(self->leds, 0);
			}
		}
		// this is currently slow
		for(unsigned c = 0; c < FB_LED_COUNT; c++){
			analog_write(self->sw_leds, c, self->state.leds[c].intensity);
		}
		thread_sleep_ms_until(&t, 1000/60);
	}
}

static void _fb_read_controls(struct application *self){
	// read next adc mux channel
	adc_read(self->adc, FB_ADC_MUX_CHANNEL, &self->mux_adc[self->mux_chan]);

	float duty = constrain_float((float)(self->mux_adc[self->mux_chan]) / 4096, 0.0f, 1.0f);
	switch(self->mux_chan) {
		case FB_ADC_YAW_CHAN: { self->controls.yaw = -1.f + 2.f * duty; } break;
		case FB_ADC_PITCH_CHAN: { self->controls.pitch = -1.f + 2.f * duty; } break;
		case FB_ADC_YAW_ACC_CHAN: { self->controls.yaw_acc = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case FB_ADC_PITCH_ACC_CHAN: { self->controls.pitch_acc = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case FB_ADC_YAW_SPEED_CHAN: { self->controls.yaw_speed = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case FB_ADC_MOTOR_PITCH_CHAN: { self->actual.pitch_rad = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case FB_ADC_PITCH_SPEED_CHAN: { self->controls.pitch_speed = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
	}

	self->mux_chan = (self->mux_chan+1) & 0x7;

	gpio_write(self->mux, 0, self->mux_chan & 1);
	gpio_write(self->mux, 1, (self->mux_chan >> 1) & 1);
	gpio_write(self->mux, 2, (self->mux_chan >> 2) & 1);

	// read switches
	for(unsigned int c = 0; c < 8; c++){
		bool on = !gpio_read(self->sw_gpio, 8 + c);
		self->state.sw[c].toggled = on != self->state.sw[c].pressed;
		if(self->state.sw[c].toggled || !on){
			self->state.sw[c].pressed_time = micros();
		}
		self->state.sw[c].pressed = on;
	}

	// read motor positions
	float yrad = M_PI * ((float)(int16_t)(encoder_read(self->enc1) & 0xffff) / (float)(int16_t)(0xffff >> 1));
	self->actual.yaw_rad = yrad;
}

static void _fb_state_operational(struct application *self, float dt);
static void _fb_state_wait_home(struct application *self, float dt);
static void _fb_state_wait_power(struct application *self, float dt);
static void _fb_state_save_preset(struct application *self, float dt);
static void _fb_state_auto(struct application *self, float dt);

static void _fb_leds_off(struct application *self){
	// bring all leds to known state upon state change
	for(unsigned c = 0; c < FB_LED_COUNT; c++){
		self->state.leds[c].state = FB_LED_STATE_OFF;
	}
}

static void _fb_enter_state(struct application *self, void (*fp)(struct application *self, float dt)){	
	// enter new state
	if(fp == _fb_state_wait_power){
		_fb_leds_off(self);
		// blink the led quickly
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_SLOW_RAMP;
		self->state.enable_motors = false;
	} if(fp == _fb_state_wait_home){
		_fb_leds_off(self);
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_FAST_RAMP;
		self->state.enable_motors = true;
	} else if(fp == _fb_state_operational){
		_fb_leds_off(self);
		self->state.enable_motors = true;
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_ON;
		printk("fb: operational\n");
		for(unsigned c = 0; c < FB_PRESET_COUNT; c++){
			struct fb_led_state *led = &self->state.leds[FB_LED_PRESET1];
			switch(c){
				case 0: led = &self->state.leds[FB_LED_PRESET1]; break;
				case 1: led = &self->state.leds[FB_LED_PRESET2]; break;
				case 2: led = &self->state.leds[FB_LED_PRESET3]; break;
				case 3: led = &self->state.leds[FB_LED_PRESET4]; break;
				default: continue;
			}
			if(self->config.presets[c].valid){
				led->state = FB_LED_STATE_ON;
			} else {
				led->state = FB_LED_STATE_FAINT_ON;
			}
		}
	}

	self->state.fn = fp;
}

static void _fb_state_wait_power(struct application *self, float dt){
	// in this state joystick does not move the motors and device waits for user to toggle home button once
	struct fb_switch_state *home = &self->state.sw[FB_SW_HOME];
	if(home->toggled){
		// once home button is toggled once we enter the state where motors can be moved but no other controls are available
		_fb_enter_state(self, _fb_state_wait_home);
	}
}

static void _fb_state_wait_home(struct application *self, float dt){
	struct fb_switch_state *home = &self->state.sw[FB_SW_HOME];
	if(home->pressed){
		// make sure home button is lit
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_ON;
		// if it has been held for a number of seconds then we save the home position
		if(time_after(micros(), home->pressed_time + FB_HOME_LONG_PRESS_TIME_US)){
			printk("fb: saving home position %d %d\n", (int32_t)(self->actual.pitch_rad * 1000), (int32_t)(self->actual.yaw_rad * 1000));
			self->config.home.pitch = self->actual.pitch_rad;
			self->config.home.yaw = self->actual.yaw_rad;
			_fb_enter_state(self, _fb_state_operational);
		}
	} else {
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_FAST_RAMP;
	}
}

static void _fb_state_save_preset(struct application *self, float dt){
	struct fb_switch_state *sw[] = { 
		&self->state.sw[FB_SW_PRESET1],
		&self->state.sw[FB_SW_PRESET2],
		&self->state.sw[FB_SW_PRESET3],
		&self->state.sw[FB_SW_PRESET4]
	};

	for(int c = 0; c < 4; c++){
		if(!sw[c]->pressed && sw[c]->toggled){
			_fb_enter_state(self, _fb_state_operational);
		}
	}
}

static void _fb_state_operational(struct application *self, float dt){
	struct fb_switch_state *sw[] = { 
		&self->state.sw[FB_SW_PRESET1],
		&self->state.sw[FB_SW_PRESET2],
		&self->state.sw[FB_SW_PRESET3],
		&self->state.sw[FB_SW_PRESET4]
	};

	struct fb_led_state *leds[] = { 
		&self->state.leds[FB_LED_PRESET1],
		&self->state.leds[FB_LED_PRESET2],
		&self->state.leds[FB_LED_PRESET3],
		&self->state.leds[FB_LED_PRESET4]
	};

	int num_pressed = 0;
	int preset = 0;
	for(int c = 0; c < 4; c++){
		if(sw[c]->pressed || sw[c]->toggled){
			preset = c;
			num_pressed++;
		}
	}

	if(num_pressed == 1){
		if(sw[preset]->pressed){
			// always turn off the led if button is pressed
			leds[preset]->state = FB_LED_STATE_OFF;
		}
		
		if(!sw[preset]->pressed && sw[preset]->toggled){
			if(self->config.presets[preset].valid){
				printk("loading preset %d\n", preset);
				self->state.yaw_target = self->config.presets[preset].yaw;
				self->state.pitch_target = self->config.presets[preset].pitch;
				_fb_enter_state(self, _fb_state_auto);
			}
			// reset the led to default glow
			if(self->config.presets[preset].valid){
				leds[preset]->state = FB_LED_STATE_ON;
			} else {
				leds[preset]->state = FB_LED_STATE_FAINT_ON;
			}
		} else if(sw[preset]->pressed && time_after(micros(), sw[preset]->pressed_time + FB_HOME_LONG_PRESS_TIME_US)){
			printk("saving preset %d\n", preset);
			self->config.presets[preset].pitch = self->actual.pitch_rad;
			self->config.presets[preset].yaw = self->actual.yaw_rad;
			self->config.presets[preset].valid = true;
			leds[preset]->state = FB_LED_STATE_ON;
			_fb_enter_state(self, _fb_state_save_preset);
		}
	}
}

static void _fb_state_auto(struct application *self, float dt){
	float target_pitch = self->state.pitch_target;
	float target_yaw = self->state.yaw_target;
	float err_pitch = target_pitch - self->actual.pitch_rad;
	float err_yaw = target_yaw - self->actual.yaw_rad;

	self->controls.pitch = 10.f * err_pitch + 0.2f * self->state.pitch_i + 8.f * (err_pitch - self->state.prev_err_pitch) / dt;
	self->controls.yaw = 1.f * err_yaw + 0.2f * self->state.yaw_i + 0.1 * (err_yaw - self->state.prev_err_yaw) / dt;

	self->state.prev_err_pitch = err_pitch;
	self->state.prev_err_yaw = err_yaw;
	self->state.pitch_i = constrain_float(self->state.pitch_i + err_pitch * dt, -1.f, 1.f);
	self->state.yaw_i = constrain_float(self->state.yaw_i + err_yaw * dt, -1.f, 1.f);

	self->controls.pitch = constrain_float(self->controls.pitch, -1.f, 1.f);
	self->controls.yaw = -constrain_float(self->controls.yaw, -1.f, 1.f);

	if(fabsf(err_pitch) < 0.01 && fabsf(err_yaw) < 0.01){
		_fb_enter_state(self, _fb_state_operational);
	}

	printk("err: %d %d\n", (int32_t)(err_pitch * 1000), (int32_t)(err_yaw * 1000));
}

static void _fb_update_state(struct application *self){
	timestamp_t t = micros();
	float dt = (float)(t - self->state.prev_micros) * 1e-6;
	self->state.prev_micros = t;

	self->state.fn(self, dt);

	for(unsigned c = 0; c < FB_LED_COUNT; c++){
		struct fb_led_state *led = &self->state.leds[c];
		switch(led->state) {
			case FB_LED_STATE_ON: {
				led->intensity = 1.f;
			} break;
			case FB_LED_STATE_FAINT_ON: {
				led->intensity = 0.1f;
			} break;
			case FB_LED_STATE_OFF: {
				led->intensity = 0.0f;
			} break;
			case FB_LED_STATE_SLOW_RAMP:
			case FB_LED_STATE_FAST_RAMP:
			case FB_LED_STATE_3BLINKS_OFF: {
				float dim_time = 0.5;
				if(led->state == FB_LED_STATE_FAST_RAMP || led->state == FB_LED_STATE_3BLINKS_OFF) dim_time = 5.0;
				if(led->dim < 0){
					led->intensity -= dim_time * dt;
					if(led->intensity < 0.0f){
						led->intensity = 0.0f;
						led->dim = -led->dim;
					}
				} else if(led->dim > 0){
					led->intensity += dim_time * dt;
					if(led->intensity > 1.0f){
						led->intensity = 1.0f;
						led->dim = -led->dim;
						if(led->state == FB_LED_STATE_3BLINKS_OFF) led->blinks++;
					}
				}

				if(led->state == FB_LED_STATE_3BLINKS_OFF && led->blinks > 3){
					led->blinks = 0;
					led->state = FB_LED_STATE_OFF;
				}
			} break;
		}
	}
	
	// update pitch and yaw
	float pitch = self->state.pitch;
	if(pitch < self->controls.pitch) pitch += 4.f * self->controls.pitch_acc * dt;
	else if(pitch > self->controls.pitch) pitch -= 4.f * self->controls.pitch_acc * dt;

	float yaw = self->state.yaw;
	if(yaw < self->controls.yaw) yaw += 4.f * self->controls.yaw_acc * dt;
	else if(yaw > self->controls.yaw) yaw -= 4.f * self->controls.yaw_acc * dt;

	if(self->state.enable_motors){
		self->state.pitch = constrain_float(pitch, -self->controls.pitch_speed, self->controls.pitch_speed);
		self->state.yaw = constrain_float(yaw, -self->controls.yaw_speed, self->controls.yaw_speed);
	} else {
		self->state.pitch = 500;
		self->state.yaw = 500;
	}

	if(
		time_after(t, self->remote.last_pitch_update + FB_REMOTE_TIMEOUT) || 
		time_after(t, self->remote.last_yaw_update + FB_REMOTE_TIMEOUT)
	){
		self->state.remote = false;
	} else {
		self->state.remote = true;
	}
}

static void _fb_update_outputs(struct application *self){
	float pitch = self->state.pitch * 0.5;
	float yaw = self->state.yaw * 0.5;
	if(self->state.remote){
		pitch = ((float)constrain_u16(self->remote.pitch, 0, 1000) / 1000.f - 0.5f);
		yaw = ((float)constrain_u16(self->remote.yaw, 0, 1000) / 1000.f - 0.5f);
	}

	analog_write(self->mot_y, 0, 0.5 + pitch);
	analog_write(self->mot_y, 1, 0.5 - pitch);
	analog_write(self->mot_y, 2, 0.5 + pitch);

	analog_write(self->mot_x, 0, 0.5 + yaw);
	analog_write(self->mot_x, 1, 0.5 - yaw);
	analog_write(self->mot_x, 2, 0.5 + yaw);

	// save the values for sending over canopen
	self->output.pitch = (int16_t)(pitch * 2000 + 1000);
	self->output.yaw = (int16_t)(yaw * 2000 + 1000);
}

static void _fb_control_loop(void *ptr){
	struct application *self = (struct application *)ptr;

	uint32_t t = thread_ticks_count();
	while(1){
		if(!self){
			printk("fb_control_loop: possible overflow!\n");
			return;
		}
		_fb_read_controls(self);
		_fb_update_state(self);
		_fb_update_outputs(self);

		thread_sleep_ms_until(&t, 1);
	}
}

static ssize_t _comm_range_read(regmap_range_t range, uint32_t addr, regmap_value_type_t type, void *data, size_t size){
	//struct application *self = container_of(range, struct application, comm_range.ops);
	uint32_t id = addr & 0x00ffff00;
	//uint32_t sub = addr & 0xff;

	//thread_mutex_lock(&self->lock);

	if(id == CANOPEN_REG_DEVICE_TYPE){
		regmap_convert_u32(402, type, data, size);
		goto success;
	}

	return -ENOENT;
success:
	//thread_mutex_unlock(&self->lock);
	return (ssize_t)size;
}

static ssize_t _comm_range_write(regmap_range_t range, uint32_t addr, regmap_value_type_t type, const void *data, size_t size){
	return -ENOENT;
}

static struct regmap_range_ops _comm_range_ops = {
	.read = _comm_range_read,
	.write = _comm_range_write
};

static ssize_t _mfr_range_read(regmap_range_t range, uint32_t addr, regmap_value_type_t type, void *data, size_t size){
	struct application *self = container_of(range, struct application, mfr_range.ops);
	uint32_t id = addr & 0x00ffff00;
	int ret = -ENOENT;
	switch(id){
		case CANOPEN_FB_MOTOR_PITCH: {
			ret = regmap_convert_u32((uint16_t)self->output.pitch, type, data, size);
		} break;
		case CANOPEN_FB_MOTOR_YAW: {
			ret = regmap_convert_u32((uint16_t)self->output.yaw, type, data, size);
		} break;
	}
	return ret;
}

static ssize_t _mfr_range_write(regmap_range_t range, uint32_t addr, regmap_value_type_t type, const void *data, size_t size){
	return -1;
}

static ssize_t _mfr_range_slave_read(regmap_range_t range, uint32_t addr, regmap_value_type_t type, void *data, size_t size){
	return -1;
}

static ssize_t _mfr_range_slave_write(regmap_range_t range, uint32_t addr, regmap_value_type_t type, const void *data, size_t size){
	struct application *self = container_of(range, struct application, mfr_range_slave.ops);
	uint32_t id = addr & 0x00ffffff;
	int ret = -ENOENT;
	timestamp_t t = micros();
	switch(id){
		case CANOPEN_FB_MOTOR_PITCH: {
			uint16_t pitch = 0;
			regmap_mem_to_u16(type, data, size, &pitch);
			self->remote.pitch = constrain_u16(pitch, 0, 1000);
			self->remote.last_pitch_update = t;
			ret = (int)size;
		} break;
		case CANOPEN_FB_MOTOR_YAW: {
			uint16_t yaw = 0;
			regmap_mem_to_u16(type, data, size, &yaw);
			self->remote.yaw = constrain_u16(yaw, 0, 1000);
			self->remote.last_yaw_update = t;
			ret = (int)size;
		} break;
	}
	return ret;
}

static struct regmap_range_ops _mfr_range_ops = {
	.read = _mfr_range_read,
	.write = _mfr_range_write
};

static struct regmap_range_ops _mfr_range_slave_ops = {
	.read = _mfr_range_slave_read,
	.write = _mfr_range_slave_write
};

static int _fb_probe(void *fdt, int fdt_node){
	struct application *self = kzmalloc(sizeof(struct application));

	BUG_ON(!self);

	gpio_device_t sw_gpio = gpio_find_by_ref(fdt, fdt_node, "sw_gpio");
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
	regmap_device_t regmap_slave = regmap_find_by_ref(fdt, fdt_node, "regmap_slave");
	memory_device_t motor_mem = memory_find_by_ref(fdt, fdt_node, "canopen");
	gpio_device_t enc1_gpio = gpio_find_by_ref(fdt, fdt_node, "enc1_gpio");
	gpio_device_t enc2_gpio = gpio_find_by_ref(fdt, fdt_node, "enc2_gpio");
	//canopen_device_t canopen = canopen_find_by_ref(fdt, fdt_node, "canopen");

	if(!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y){
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if(!eeprom){ printk(PRINT_ERROR "fb: eeprom missing\n"); return -1; }
	if(!console){ printk(PRINT_ERROR "fb: console missing\n"); return -1; }
	if(!oc_pot){ printk(PRINT_ERROR "fb: oc_pot missing\n"); return -1; }
	if(!can1){ printk(PRINT_ERROR "fb: can1 missing\n"); return -1; }
	if(!can2){ printk(PRINT_ERROR "fb: can2 missing\n"); return -1; }
	if(!regmap){ printk(PRINT_ERROR "fb: regmap missing\n"); return -1; }
	if(!motor_mem){ printk(PRINT_ERROR "fb: canopen_slave missing\n"); return -1; }
	if(!regmap_slave){ printk(PRINT_ERROR "fb: regmap_slave missing\n"); return -1; }
	if(!enc1){ printk(PRINT_ERROR "fb: enc1 missing\n"); return -1; }
	if(!enc2){ printk(PRINT_ERROR "fb: enc2 missing\n"); return -1; }
	if(!enc1_gpio){ printk(PRINT_ERROR "fb: enc1_gpio missing\n"); return -1; }
	if(!enc2_gpio){ printk(PRINT_ERROR "fb: enc2_gpio missing\n"); return -1; }
	//if(!canopen){ printk(PRINT_ERROR "fb: canopen missing\n"); return -1; }

	console_add_command(console, self, "fb", "Flying Bergman Control", "", _fb_cmd);
	console_add_command(console, self, "reg", "Register ops", "", _reg_cmd);
	console_add_command(console, self, "motor", "Motor slave board control", "", _motor_cmd);
	console_add_command(console, self, "can1", "CAN1 control", "", _can_cmd);
	console_add_command(console, self, "can2", "CAN2 control", "", _can_cmd);

	self->leds = leds;
	self->console = console;
	self->sw_gpio = sw_gpio;
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
	self->motor_mem = motor_mem;
	self->enc1_gpio = enc1_gpio;
	self->enc2_gpio = enc2_gpio;
	//self->canopen = canopen;

	for(int c = 0; c < FB_LED_COUNT; c++){
		self->state.leds[c].dim = 1;
		self->state.leds[c].state = FB_LED_STATE_OFF;
	}

	analog_write(self->oc_pot, OC_POT_OC_ADJ_MOTOR1, 1.f);
	analog_write(self->oc_pot, OC_POT_OC_ADJ_MOTOR2, 1.f);

	led_on(self->leds, 0);
	thread_sleep_ms(500);
	led_off(self->leds, 0);
	thread_sleep_ms(500);

	led_on(self->leds, 0);
	led_off(self->leds, 1);
	led_on(self->leds, 2);

	printk("fb: configuring as canopen master\n");
	regmap_write_u32(self->regmap, CANOPEN_REG_DEVICE_CYCLE_PERIOD, 1000);

	// add the device profile map
	regmap_range_init(&self->comm_range, CANOPEN_COMM_RANGE_START, CANOPEN_COMM_RANGE_END, &_comm_range_ops);
	regmap_add(regmap_slave, &self->comm_range);

	regmap_range_init(&self->mfr_range, CANOPEN_MFR_RANGE_START, CANOPEN_MFR_RANGE_END, &_mfr_range_ops);
	regmap_add(regmap, &self->mfr_range);

	regmap_range_init(&self->mfr_range_slave, CANOPEN_MFR_RANGE_START, CANOPEN_MFR_RANGE_END, &_mfr_range_slave_ops);
	regmap_add(regmap_slave, &self->mfr_range_slave);

	//regmap_write_u32(regmap_slave, CANOPEN_REG_DEVICE_TYPE, 402);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	_fb_enter_state(self, _fb_state_wait_power);
/*
    struct canopen_pdo_config conf = {
        .cob_id = 0x201,
        .index = 0,
        .type = CANOPEN_PDO_TYPE_CYCLIC(1),
        .inhibit_time = 100,
        .event_time = 100,
        .map = {
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_MOTOR_PITCH, CANOPEN_PDO_SIZE_16),
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_MOTOR_YAW, CANOPEN_PDO_SIZE_16),
            0
        }
    };

    if(canopen_pdo_tx(self->motor_mem, 0x01, &conf) < 0){
		printk(PRINT_ERROR "fb: tx pdo setup failed\n");
	}

	conf.index = 0;
	if(canopen_pdo_rx(self->motor_mem, 0x02, &conf) < 0){
		printk(PRINT_ERROR "fb: rx pdo setup failed\n");
    }
*/
	thread_create(
		  _fb_control_loop,
		  "fb_ctrl",
		  500,
		  self,
		  2,
		  NULL);

	thread_create(
		  _fb_indicator_loop,
		  "fb_ind",
		  250,
		  self,
		  1,
		  NULL);

	printk("FB Rev A\n");

	return 0;
}

static int _fb_remove(void *fdt, int fdt_node){
	// TODO
    return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _fb_probe, _fb_remove)
