#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include <libfdt/libfdt.h>
#include "dts/flyingbergman.h"
#include "flyingbergman.h"

#define print_status(name, ok) do { console_printf(con, PRINT_DEFAULT "%16s: %s\n" PRINT_DEFAULT, name, (ok)?PRINT_SUCCESS "OK":PRINT_ERROR "FAIL"); } while(0)
static int _fb_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 2 && strcmp(argv[1], "sw") == 0){
		for(int c = 0; c < 8; c++){
			int val = (int)gpio_read(self->sw_gpio, (uint32_t)(8+c));
			console_printf(con, "SW%d: %d\n", c, val);
		}
	} else if(argc == 2 && strcmp(argv[1], "status") == 0){
		console_printf(con, "Pitch: \n");
		print_status("h-bridge",
			!drv8302_is_in_error(self->drv_pitch) &&
			!drv8302_is_in_overcurrent(self->drv_pitch)
		);
		print_status("dc-cal", 
			abs(self->config.dc_cal.pitch_a - 2048) < 100 &&
			abs(self->config.dc_cal.pitch_b - 2048) < 100);
		console_printf(con, "Yaw: \n");
		print_status("h-bridge",
			!drv8302_is_in_error(self->drv_yaw) &&
			!drv8302_is_in_overcurrent(self->drv_yaw)
		);
		print_status("dc-cal",
			abs(self->config.dc_cal.yaw_a - 2048) < 100 &&
			abs(self->config.dc_cal.yaw_b - 2048) < 100);
	} else if(argc == 2 && strcmp(argv[1], "inputs") == 0){
		console_printf(con, "US: %u\n", micros());
		console_printf(con, "SW: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "(%d): %d ", c, self->inputs.sw[c].pressed);
		}
		console_printf(con, "\n");

		console_printf(con, "AIN: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "[%2d] %5d ", c, self->mux_adc[c]);
		}
		console_printf(con, "\n");

		console_printf(con, "ADC: ");
		for(unsigned c = 0; c < 16; c++){
			uint16_t val = 0;
			adc_read(self->adc, c, &val);
			console_printf(con, "[%2d] %5d ", c, val);
		}
		console_printf(con, "\n");

		console_printf(con, "ENC1: COUNT %d, Di2 %d, Di3 %d\n",
				encoder_read(self->enc1),
				self->inputs.enc1_aux1,
				self->inputs.enc1_aux2
		);
		console_printf(con, "ENC2: COUNT %d, Di2 %d, Di3 %d\n",
				encoder_read(self->enc2),
				self->inputs.enc2_aux1,
				gpio_read(self->enc2_gpio, 2)
				//self->inputs.enc2_aux2
		);

		console_printf(con, "VS1:\t %5d %5d %5d\n", self->inputs.vsa_yaw, self->inputs.vsb_yaw, self->inputs.vsc_yaw);
		console_printf(con, "VS2:\t %5d %5d %5d\n", self->inputs.vsa_pitch, self->inputs.vsb_pitch, self->inputs.vsc_pitch);
		console_printf(con, "I1:\t %5d %5d\n", self->inputs.ia_yaw, self->inputs.ib_yaw);
		console_printf(con, "I2:\t %5d %5d\n", self->inputs.ia_pitch, self->inputs.ib_pitch);

		console_printf(con, "JOYSTICK:\tYAW %5d PITCH %5d\n",
				self->inputs.joy_yaw,
				self->inputs.joy_pitch
		);
		console_printf(con, "INTENSITY:\tYAW %5d PITCH %5d\n",
				self->inputs.yaw_acc,
				self->inputs.pitch_acc
		);
		console_printf(con, "SPEED:\t\tYAW %5d PITCH %5d\n",
				self->inputs.yaw_speed,
				self->inputs.pitch_speed
		);
		console_printf(con, "POSITION:\tYAW %5d PITCH %5d\n",
				self->inputs.yaw,
				self->inputs.pitch
		);
		console_printf(con, "VMOT:\t\t%d\n", self->inputs.vmot);
		console_printf(con, "TEMP_YAW:\t\t%d\n", self->inputs.temp_yaw);
		console_printf(con, "TEMP_PITCH:\t\t%d\n", self->inputs.temp_pitch);
		console_printf(con, "CAN_ADDR:\t\t%02x\n", self->inputs.can_addr);
		console_printf(con, "FROM MASTER:\t\tYAW %5d PITCH %5d\n",
				self->remote.yaw,
				self->remote.pitch
		);
		thread_mutex_lock(&self->slave.lock);
		console_printf(con, "SLAVE RAW POS:\tYAW %5d PITCH %5d\n",
				self->slave.yaw,
				self->slave.pitch
		);
		thread_mutex_unlock(&self->slave.lock);
	} else if(argc == 2 && strcmp(argv[1], "meas") == 0){
		#define TAB "\033[16G"
		console_printf(con, "VS1 (mV):" TAB "%6d %6d %6d\n",
				(int32_t)(self->measured.vsa_yaw * 1000),
				(int32_t)(self->measured.vsb_yaw * 1000),
				(int32_t)(self->measured.vsc_yaw * 1000));
		console_printf(con, "VS2 (mV):" TAB "%6d %6d %6d\n",
				(int32_t)(self->measured.vsa_pitch * 1000),
				(int32_t)(self->measured.vsb_pitch * 1000),
				(int32_t)(self->measured.vsc_pitch * 1000));
		console_printf(con, "I1 (mA):" TAB "%6d %6d\n",
				(int32_t)(self->measured.ia_yaw * 1000),
				(int32_t)(self->measured.ib_yaw * 1000));
		console_printf(con, "I2 (mA):" TAB "%6d %6d\n",
				(int32_t)(self->measured.ia_pitch * 1000),
				(int32_t)(self->measured.ib_pitch * 1000));
		console_printf(con, "JOYSTICK:" TAB "YAW %5d PITCH %5d\n",
				(int32_t)(self->measured.joy_yaw * 1000),
				(int32_t)(self->measured.joy_pitch * 1000)
		);
		console_printf(con, "INTENSITY:" TAB "YAW %5d PITCH %5d\n",
				(int32_t)(self->measured.yaw_acc * 1000),
				(int32_t)(self->measured.pitch_acc * 1000)
		);
		console_printf(con, "SPEED:" TAB "YAW %5d PITCH %5d\n",
				(int32_t)(self->measured.yaw_speed * 1000),
				(int32_t)(self->measured.pitch_speed * 1000)
		);
		console_printf(con, "LOCAL POS:" TAB "YAW %5d PITCH %5d\n",
				(int32_t)(self->measured.yaw * 1000),
				(int32_t)(self->measured.pitch * 1000)
		);
		console_printf(con, "VMOT:" TAB "%dmV\n", (int32_t)(self->measured.vmot * 1000));
		console_printf(con, "TEMP_YAW:" TAB "%d mC\n", (int32_t)(self->measured.temp_yaw * 1000));
		console_printf(con, "TEMP_PITCH:" TAB "%d mC\n", (int32_t)(self->measured.temp_pitch * 1000));
	} else if(argc == 2 && strcmp(argv[1], "data") == 0){
		while(1){
			console_printf(con, "%u;", micros());
			console_printf(con, "%d;%d;",
					(int32_t)(self->output.pitch * 10000),	// output pitch
					(int32_t)(self->output.yaw * 10000)		// output yaw
			);
		
			if(self->mode == FB_MODE_MASTER){
				console_printf(con, "%d;%d;",
						self->slave.i_pitch,				// current in mA from slave
						self->slave.i_yaw					// current in mA from slave
				);
				console_printf(con, "%d;%d;",
						(int32_t)(self->measured.joy_pitch * 10000), // user control pitch
						(int32_t)(self->measured.joy_yaw * 10000)	// user control yaw
				);
				console_printf(con, "%d;%d;",
						(int32_t)(self->measured.pitch * 10000), // motor position pitch
						(int32_t)(self->measured.yaw * 10000)	// motor position yaw
				);
				console_printf(con, "%d;%d;",
						(int32_t)(self->controller.pitch.output * 10000),
						(int32_t)(self->controller.yaw.output * 10000)
				);
				console_printf(con, "%d;%d;",
						(int32_t)(self->controller.pitch.target * 10000),
						(int32_t)(self->controller.yaw.target * 10000)
				);
				console_printf(con, "%d;", self->slave.vmot);
				console_printf(con, "%d;%d;",
						(int32_t)(self->output.pitch * 1000),
						(int32_t)(self->output.yaw * 1000)
				);
			} else {
				console_printf(con, "%5d;%5d;%5d;%5d",
						(int32_t)(self->measured.ia_pitch * 10000),
						(int32_t)(self->measured.ib_pitch * 10000),
						(int32_t)(self->measured.ia_yaw * 10000),
						(int32_t)(self->measured.ib_yaw * 10000)
				);
				console_printf(con, "%5d;%5d;",
						(int32_t)(self->remote.pitch),
						(int32_t)(self->remote.yaw)
				);
				console_printf(con, "%d;%d;",
						(int32_t)(self->output.pitch * 1000),
						(int32_t)(self->output.yaw * 1000)
				);
			}

			console_printf(con, "\n");
			char ch;
			if(console_read(con, &ch, 1, 0) == 1){
				break;
			}

			thread_sleep_ms(10);
		}
	} else if(argc == 2 && strcmp(argv[1], "enc") == 0){
		int32_t val = encoder_read(self->enc1);
		console_printf(con, "ENC1: %d\n", val);
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
	} else if(argc == 2 && strcmp(argv[1], "leds") == 0){
		for(unsigned c = 0; c < 8; c++){
			float volts = 0;
			if(analog_read(self->sw_leds, c, &volts) == 0){
				printk("[%d]: %dmV\n", c, (int32_t)(volts * 1000));
			} else {
				printk(PRINT_ERROR "[%d]: could not read voltage!\n", c);
			}
		}
	} else if(argc == 2 && strcmp(argv[1], "motors") == 0){
		console_printf(con, "Pitch: %s %s\n",
			(drv8302_is_in_error(self->drv_pitch))?"FAULT":"NOFAULT",
			(drv8302_is_in_overcurrent(self->drv_pitch))?"OCW":"NOOCW"
		);
		console_printf(con, "Yaw: %s %s\n",
			(drv8302_is_in_error(self->drv_yaw))?"FAULT":"NOFAULT",
			(drv8302_is_in_overcurrent(self->drv_yaw))?"OCW":"NOOCW"
		);
		float gp = (float)drv8302_get_gain(self->drv_pitch);
		float gy = (float)drv8302_get_gain(self->drv_yaw);
		console_printf(con, "Gains: PITCH %d, YAW %d\n",
			(int32_t)(gp * 1000),
			(int32_t)(gy * 1000)
		);
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
#define _read(reg, value) ret |= memory_read(self->canopen_mem, addr | reg, &value, sizeof(value))
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
			if(memory_read(self->canopen_mem, ofs, &reg, sizeof(reg)) >= 0){
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
		for(unsigned c = 0; c < FB_LED_COUNT; c++){
			analog_write(self->sw_leds, c, self->state.leds[c].intensity);
		}
		thread_sleep_ms_until(&t, 1000/25);
	}
}


static int _fb_configure_slave(struct application *self){
    struct canopen_pdo_config conf = {
        .cob_id = 0x200 | FB_CANOPEN_SLAVE_ADDRESS,
        .index = 0,
        .type = CANOPEN_PDO_TYPE_CYCLIC(1),
        .inhibit_time = 100,
        .event_time = 100,
        .map = {
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_DEMAND, CANOPEN_PDO_SIZE_16),
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_DEMAND, CANOPEN_PDO_SIZE_16),
            0
        }
    };

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0){
		return -EIO;
    }

    if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0){
		return -EIO;
	}

    conf = (struct canopen_pdo_config){
        .cob_id = 0x200 | FB_CANOPEN_MASTER_ADDRESS,
        .index = 1,
        .type = CANOPEN_PDO_TYPE_CYCLIC(1),
        .inhibit_time = 100,
        .event_time = 100,
        .map = {
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_POSITION, CANOPEN_PDO_SIZE_16),
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_POSITION, CANOPEN_PDO_SIZE_16),
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_CURRENT, CANOPEN_PDO_SIZE_16),
            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_CURRENT, CANOPEN_PDO_SIZE_16),
            0
        }
    };

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0){
		return -EIO;
    }

    if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0){
		return -EIO;
	}

    conf.cob_id = 0x210 | FB_CANOPEN_MASTER_ADDRESS,
	conf.index = 2;
	conf.map[0] = CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_VMOT, CANOPEN_PDO_SIZE_16);
	conf.map[1] = 0;

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0){
		return -EIO;
    }

    if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0){
		return -EIO;
	}

	return 0;
}

static void _fb_state_operational(struct application *self, float dt);
static void _fb_state_wait_home(struct application *self, float dt);
static void _fb_state_wait_power(struct application *self, float dt);
static void _fb_state_save_preset(struct application *self, float dt);

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
		self->control_mode = FB_CONTROL_MODE_NO_MOTION;
	} if(fp == _fb_state_wait_home){
		_fb_leds_off(self);
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_FAST_RAMP;
		self->control_mode = FB_CONTROL_MODE_MANUAL;
	} else if(fp == _fb_state_operational){
		_fb_leds_off(self);
		self->control_mode = FB_CONTROL_MODE_MANUAL;
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_ON;
		for(unsigned c = 0; c < FB_PRESET_COUNT; c++){
			struct fb_led_state *led = &self->state.leds[FB_LED_PRESET1];
			switch(c){
				case 0: led = &self->state.leds[FB_LED_HOME]; break;
				case 1: led = &self->state.leds[FB_LED_PRESET1]; break;
				case 2: led = &self->state.leds[FB_LED_PRESET2]; break;
				case 3: led = &self->state.leds[FB_LED_PRESET3]; break;
				case 4: led = &self->state.leds[FB_LED_PRESET4]; break;
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
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if(home->toggled){
		printk("fb: move motors to desired home position\n");
		// once home button is toggled once we enter the state where motors can be moved but no other controls are available
		_fb_enter_state(self, _fb_state_wait_home);
	}
}

static void _fb_state_wait_home(struct application *self, float dt){
	struct fb_switch_state *home = &self->inputs.sw[FB_SW_HOME];
	if(home->pressed){
		// make sure home button is lit
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_ON;
		// if it has been held for a number of seconds then we save the home position
		if(time_after(micros(), home->pressed_time + FB_HOME_LONG_PRESS_TIME_US)){
			printk("fb: saving home position %d %d\n", (int32_t)(self->measured.pitch * 1000), (int32_t)(self->measured.yaw * 1000));
			self->config.presets[0].pitch = 0; //self->measured.pitch;
			self->config.presets[0].yaw = self->measured.yaw;
			self->config.presets[0].valid = true;
			_fb_enter_state(self, _fb_state_operational);
		}
	} else {
		self->state.leds[FB_LED_HOME].state = FB_LED_STATE_FAST_RAMP;
	}
}

static void _fb_state_save_preset(struct application *self, float dt){
	struct fb_switch_state *sw[] = { 
		&self->inputs.sw[FB_SW_HOME],
		&self->inputs.sw[FB_SW_PRESET1],
		&self->inputs.sw[FB_SW_PRESET2],
		&self->inputs.sw[FB_SW_PRESET3],
		&self->inputs.sw[FB_SW_PRESET4]
	};

	for(int c = 0; c < FB_PRESET_COUNT; c++){
		if(!sw[c]->pressed && sw[c]->toggled){
			_fb_enter_state(self, _fb_state_operational);
		}
	}
}

static void _fb_state_operational(struct application *self, float dt){
	struct fb_switch_state *sw[] = { 
		&self->inputs.sw[FB_SW_HOME],
		&self->inputs.sw[FB_SW_PRESET1],
		&self->inputs.sw[FB_SW_PRESET2],
		&self->inputs.sw[FB_SW_PRESET3],
		&self->inputs.sw[FB_SW_PRESET4]
	};

	struct fb_led_state *leds[] = { 
		&self->state.leds[FB_LED_HOME],
		&self->state.leds[FB_LED_PRESET1],
		&self->state.leds[FB_LED_PRESET2],
		&self->state.leds[FB_LED_PRESET3],
		&self->state.leds[FB_LED_PRESET4]
	};

	int num_pressed = 0;
	int preset = 0;
	for(int c = 0; c < FB_PRESET_COUNT; c++){
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
				// for home we want to rotate to closest halv circle
				float target_yaw = self->config.presets[preset].yaw;
				float target_pitch = self->config.presets[preset].pitch;
				if(preset == 0){
					if(fabsf(normalize_angle(target_yaw - self->measured.yaw)) > (M_PI / 2)){
						target_yaw = normalize_angle(target_yaw + M_PI);
					}
				}
				target_yaw = normalize_angle(target_yaw - self->measured.yaw);

				struct timeval ts;
				time_gettime(&ts);

				motion_profile_init(&self->controller.pitch.trajectory, self->measured.pitch_acc, self->measured.pitch_speed, self->measured.pitch_acc);
				motion_profile_init(&self->controller.yaw.trajectory, self->measured.yaw_acc, self->measured.yaw_speed, self->measured.yaw_acc);

				motion_profile_plan_move(&self->controller.pitch.trajectory, &ts, self->measured.pitch, 0, target_pitch, 0);
				motion_profile_plan_move(&self->controller.yaw.trajectory, &ts, 0, 0, target_yaw, 0);

				//self->controller.pitch.target = target_pitch;
				//self->controller.yaw.target = target_yaw;
				self->controller.pitch.integral = 0;
				self->controller.yaw.integral = 0;
				self->controller.yaw.start = self->measured.yaw;
				self->ticks_on_target = FB_TICKS_ON_TARGET;
				//printk("loading preset %d at %d %d\n", preset, (int32_t)(self->controller.pitch.target * 1000), (int32_t)(self->controller.yaw.target * 1000));
				self->control_mode = FB_CONTROL_MODE_AUTO;
			}
			// reset the led to default glow
			if(self->config.presets[preset].valid){
				leds[preset]->state = FB_LED_STATE_ON;
			} else {
				leds[preset]->state = FB_LED_STATE_FAINT_ON;
			}
		} else if(sw[preset]->pressed && time_after(micros(), sw[preset]->pressed_time + FB_HOME_LONG_PRESS_TIME_US)){
			//printk("saving preset %d at %d %d\n", preset, (int32_t)(self->measured.pitch * 1000), (int32_t)(self->measured.yaw * 1000));
			self->config.presets[preset].pitch = (preset == 0)?0:self->measured.pitch;
			self->config.presets[preset].yaw = self->measured.yaw;
			self->config.presets[preset].valid = true;
			leds[preset]->state = FB_LED_STATE_ON;
			_fb_enter_state(self, _fb_state_save_preset);
		}
	}
}

static void _fb_update_state(struct application *self, float dt){
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

	timestamp_t t = micros();
	if(
		time_after(t, self->remote.last_pitch_update + FB_REMOTE_TIMEOUT) || 
		time_after(t, self->remote.last_yaw_update + FB_REMOTE_TIMEOUT)
	){
		if(self->control_mode == FB_CONTROL_MODE_REMOTE){
			self->control_mode = FB_CONTROL_MODE_NO_MOTION;
		}
	} else {
		self->control_mode = FB_CONTROL_MODE_REMOTE;
	}
}

//! this function constrains the demanded pitch and yaw to maximum allowed slopes
static void _fb_output_constrained_fastest(struct application *self, float demand_pitch, float demand_yaw, float dt){
	demand_pitch = constrain_float(demand_pitch, -1.f, 1.f);
	demand_yaw = constrain_float(demand_yaw, -1.f, 1.f);

	float pitch = self->output.pitch;
	if(pitch < demand_pitch) pitch = constrain_float(pitch + 4.f * dt, pitch, demand_pitch);
	else if(pitch > demand_pitch) pitch = constrain_float(pitch - 4.f * dt, demand_pitch, pitch);

	float yaw = self->output.yaw;
	if(yaw < demand_yaw) yaw = constrain_float(yaw + 4.f * dt, yaw, demand_yaw);
	else if(yaw > demand_yaw) yaw = constrain_float(yaw - 4.f * dt, demand_yaw, yaw);

	self->output.pitch = constrain_float(pitch, -1.f, 1.f);
	self->output.yaw = constrain_float(yaw, -1.f, 1.f);

	if(self->measured.pitch > 0.9f){
		self->output.pitch = constrain_float(self->output.pitch, 0, 1);
	} else if(self->measured.pitch < -0.9f){
		self->output.pitch = constrain_float(self->output.pitch, -1, 0);
	}
}

//! this function constrains the demanded pitch and yaw output to be compliant with controls for speed and intensity
static void _fb_output_constrained(struct application *self, float demand_pitch, float demand_yaw, float dt){
	demand_pitch = constrain_float(demand_pitch, -1.f, 1.f);
	demand_yaw = constrain_float(demand_yaw, -1.f, 1.f);

	float pitch = self->output.pitch;
	if(pitch < demand_pitch) pitch = constrain_float(pitch + 4.f * self->measured.pitch_acc * dt, pitch, demand_pitch);
	else if(pitch > demand_pitch) pitch = constrain_float(pitch - 4.f * self->measured.pitch_acc * dt, demand_pitch, pitch);

	float yaw = self->output.yaw;
	if(yaw < demand_yaw) yaw = constrain_float(yaw + 4.f * self->measured.yaw_acc * dt, yaw, demand_yaw);
	else if(yaw > demand_yaw) yaw = constrain_float(yaw - 4.f * self->measured.yaw_acc * dt, demand_yaw, yaw);

	float pitch_speed = self->measured.pitch_speed / FB_PITCH_MAX_VEL;
	float yaw_speed = self->measured.yaw_speed / FB_YAW_MAX_VEL;
	self->output.pitch = constrain_float(pitch, -pitch_speed, pitch_speed);
	self->output.yaw = constrain_float(yaw, -yaw_speed, yaw_speed);

	if(self->measured.pitch > 0.9f){
		self->output.pitch = constrain_float(self->output.pitch, 0, 1);
	} else if(self->measured.pitch < -0.9f){
		self->output.pitch = constrain_float(self->output.pitch, -1, 0);
	}
}

static void _fb_update_control(struct application *self, float dt){
	switch(self->control_mode){
		case FB_CONTROL_MODE_NO_MOTION: {
			self->output.pitch = 0;
			self->output.yaw = 0;
		} break;
		case FB_CONTROL_MODE_REMOTE: {
			/**
			 * In this mode motors are controlled remotely over manufacturer segment
			 * It is still however important to limit change in output so that we do not
			 * suddenly stop the motor if remote communication is lost
			 */
			float pitch = (float)self->remote.pitch * 0.0005f;
			float yaw = (float)self->remote.yaw * 0.0005f;
			_fb_output_constrained_fastest(self, pitch, yaw, dt);
		} break;
		case FB_CONTROL_MODE_MANUAL: {
			/**
			 * In manual mode we generate output directly based on joystick input
			 */
			_fb_output_constrained(self, self->measured.joy_pitch, self->measured.joy_yaw, dt);
		} break;
		case FB_CONTROL_MODE_AUTO: {
			/*
			 * In auto mode we generate a trajectory that gives us a target velocity and target position for each frame
			 * The control output signal is then generated based on error between trajectory setpoint and actual position/velocity
			 * User controls are added to the control action to still make it possible to control using joystick
			 */
			struct timeval ts;
			float target_pitch = 0, target_pitch_vel = 0, target_pitch_acc = 0; //self->target.pitch.value;
			float target_yaw_calc = 0, target_yaw_vel = 0, target_yaw_acc = 0; //self->target.yaw.value;

			time_gettime(&ts);

			motion_profile_get_pva(&self->controller.pitch.trajectory, &ts, &target_pitch, &target_pitch_vel, &target_pitch_acc);
			motion_profile_get_pva(&self->controller.yaw.trajectory, &ts, &target_yaw_calc, &target_yaw_vel, &target_yaw_acc);

			float target_yaw = normalize_angle(self->controller.yaw.start + target_yaw_calc);

			self->controller.pitch.target = target_pitch;
			self->controller.yaw.target = target_yaw;

			float err_pitch = target_pitch - self->measured.pitch;
			float err_yaw = target_yaw - self->measured.yaw;

			err_pitch = _fb_filter(
					&self->controller.pitch.efilt,
					(struct fb_filter_config){.a0 = -1.9112, .a1 = 0.914976, .b0 = 0.0019161, .b1 = 0.00186018}, // cutoff 10Hz
					err_pitch);
			// since yaw is an angle we need to normalize the error
			err_yaw = normalize_angle(err_yaw);

			self->controller.pitch.integral = constrain_float(self->controller.pitch.integral + err_pitch * dt, -1.f, 1.0f);
			self->controller.yaw.integral = constrain_float(self->controller.yaw.integral + err_yaw * dt, -1.f, 1.f);

			float pff = 5.95f;
			float pkp = 18.f;
			float pki = 0.0f;
			float pkd = 0.2f;
			float yff = 0.74f * (90.f / 17.f);
			float ykp = 2.f;
			float yki = 0.0f;
			float ykd = 0.2f;

			float dpitch = (err_pitch - self->controller.pitch.error) / dt;
			float dyaw = (err_yaw - self->controller.yaw.error) / dt;

			float co_pitch =
				pff * target_pitch_vel +
				pkp * err_pitch +
				pki * self->controller.pitch.integral +
				pkd * dpitch;
			float co_yaw =
				yff * target_yaw_vel + 
				ykp * err_yaw +
				yki * self->controller.yaw.integral +
				ykd * dyaw;

			co_pitch = constrain_float(co_pitch, -1, 1);
			co_yaw = constrain_float(co_yaw, -1, 1);

			self->controller.pitch.output = co_pitch;
			self->controller.pitch.error = err_pitch;
			self->controller.yaw.output = co_yaw;
			self->controller.yaw.error = err_yaw;

			_fb_output_constrained_fastest(self, -co_pitch, -co_yaw, dt);

			// upon arrival at target switch back to manual mode
			if(fabsf(self->measured.joy_pitch) > 0.1 || fabsf(self->measured.joy_yaw) > 0.1){
				self->control_mode = FB_CONTROL_MODE_MANUAL;
			}
		} break;
	}
}

static void _fb_update_motors(struct application *self){
	float pitch = self->output.pitch;
	float yaw = self->output.yaw;

	// pitch hard limit protection
	/*
	if(self->measured.vmot > 10){
		pitch = pitch * (24.f /  self->measured.vmot);
		yaw = yaw * (24.f /  self->measured.vmot);
	}
	*/

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

static bool _fb_slave_timed_out(struct application *self){
	timestamp_t t = micros();

	uint32_t mask =
		FB_SLAVE_VALID_PITCH |
		FB_SLAVE_VALID_YAW |
		FB_SLAVE_VALID_I_PITCH |
		FB_SLAVE_VALID_I_YAW;

	thread_mutex_lock(&self->slave.lock);
	if((self->slave.valid_bits & mask) == mask){
		// if all items were received then reset the bits and update timestamp
		self->slave.valid_bits = 0;
		self->slave.last_valid = t;
		self->state.leds[FB_LED_CONN_STATUS].state = FB_LED_STATE_OFF;
	} else {
		// otherwise wait until timeout and reconfigure slave
		timestamp_t tout = self->slave.last_valid + FB_SLAVE_TIMEOUT_MS * 1000;
		if(time_after(t, tout)){
			self->slave.valid_bits = 0;
			self->slave.last_valid = t;
			self->state.leds[FB_LED_CONN_STATUS].state = FB_LED_STATE_ON;
			thread_mutex_unlock(&self->slave.lock);
			return true;
		}
	}
	thread_mutex_unlock(&self->slave.lock);
	return false;
}

static void _fb_monitor_slaves(struct application *self){
	if(self->mode != FB_MODE_MASTER) return;

	if(_fb_slave_timed_out(self)){
		_fb_enter_state(self, _fb_state_wait_power);
		_fb_configure_slave(self);
	}
}

static void _fb_control_loop(void *ptr){
	struct application *self = (struct application *)ptr;

	uint32_t ticks = thread_ticks_count();
	while(1){
		if(!self){
			printk("fb_control_loop: possible overflow!\n");
			return;
		}

		timestamp_t t = micros();
		float dt = (float)(t - self->state.prev_micros) * 1e-6;
		self->state.prev_micros = t;

		_fb_read_inputs(self);
		_fb_monitor_slaves(self);
		_fb_update_measurements(self);
		_fb_update_state(self, dt);
		_fb_update_control(self, dt);
		_fb_update_motors(self);

		thread_sleep_ms_until(&ticks, 1);
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
		case CANOPEN_FB_PITCH_DEMAND: {
			if(self->mode == FB_MODE_MASTER){
				ret = regmap_convert_u32((uint32_t)(int16_t)(self->output.pitch * 1000), type, data, size);
			} else {
				ret = regmap_convert_u16((uint16_t)self->remote.pitch, type, data, size);
			}
		} break;
		case CANOPEN_FB_YAW_DEMAND: {
			if(self->mode == FB_MODE_MASTER){
				ret = regmap_convert_u32((uint32_t)(int16_t)(self->output.yaw * 1000), type, data, size);
			} else {
				ret = regmap_convert_u16((uint16_t)self->remote.yaw, type, data, size);
			}
		} break;
		case CANOPEN_FB_PITCH_CURRENT: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.ia_pitch * 1000), type, data, size);
		} break;
		case CANOPEN_FB_YAW_CURRENT: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.ia_yaw * 1000), type, data, size);
		} break;
		case CANOPEN_FB_PITCH_POSITION: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.pitch * 1000), type, data, size);
		} break;
		case CANOPEN_FB_YAW_POSITION: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.yaw * 1000), type, data, size);
		} break;
		case CANOPEN_FB_MICROS:{
			ret = regmap_convert_u32(micros(), type, data, size);
		} break;
		case CANOPEN_FB_VMOT:{
			uint16_t vmot = (uint16_t)constrain_i32((int32_t)(self->measured.vmot * 1000), 0, UINT16_MAX);
			ret = regmap_convert_u16(vmot, type, data, size);
		} break;
	}
	return ret;
}

static ssize_t _mfr_range_write(regmap_range_t range, uint32_t addr, regmap_value_type_t type, const void *data, size_t size){
	struct application *self = container_of(range, struct application, mfr_range.ops);
	uint32_t id = addr & 0x00ffffff;
	int ret = -ENOENT;
	switch(id){
		case CANOPEN_FB_PITCH_CURRENT: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t*)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.i_pitch = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_I_PITCH;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_YAW_CURRENT: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t*)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.i_yaw = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_I_YAW;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_PITCH_POSITION: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t*)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.pitch = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_PITCH;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_YAW_POSITION: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t*)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.yaw = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_YAW;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_MICROS:{
			uint32_t val = 0;
			ret = regmap_mem_to_u32(type, data, size, &val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.micros = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_MICROS;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_PITCH_DEMAND: {
			int16_t pitch = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t*)&pitch);
			self->remote.pitch = constrain_i16(pitch, -1000, 1000);
			self->remote.last_pitch_update = micros();
			ret = (int)size;
		} break;
		case CANOPEN_FB_YAW_DEMAND: {
			int16_t yaw = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t*)&yaw);
			self->remote.yaw = constrain_i16(yaw, -1000, 1000);
			self->remote.last_yaw_update = micros();
			ret = (int)size;
		} break;
		case CANOPEN_FB_VMOT: {
			uint16_t vmot = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t*)&vmot);
			thread_mutex_lock(&self->slave.lock);
			self->slave.vmot = vmot;
			thread_mutex_unlock(&self->slave.lock);
		} break;
	}

	return ret;
}

static struct regmap_range_ops _mfr_range_ops = {
	.read = _mfr_range_read,
	.write = _mfr_range_write
};

static void _fb_load_config(struct application *self){
	// for now just fill in defaults
	self->config.limit.pitch = (struct fb_analog_limit){ .min = 0, .max = 3790, .omin = -1.f, .omax = 1.f };
	self->config.limit.joy_pitch = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = -1.f, .omax = 1.f };
	self->config.limit.joy_yaw = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = -1.f, .omax = 1.f };
	self->config.limit.pitch_acc = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = FB_PITCH_MAX_ACC * 0.2, .omax = FB_PITCH_MAX_ACC };
	self->config.limit.yaw_acc = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = FB_YAW_MAX_ACC * 0.2, .omax = FB_YAW_MAX_ACC };
	self->config.limit.pitch_speed = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = FB_PITCH_MAX_VEL * 0.2, .omax = FB_PITCH_MAX_VEL };
	self->config.limit.yaw_speed = (struct fb_analog_limit){ .min = 0, .max = 4096, .omin = FB_YAW_MAX_VEL * 0.2, .omax = FB_YAW_MAX_VEL };
	self->config.limit.vmot = (struct fb_analog_limit){ .min = 0, .max = 3470, .omin = 0, .omax = 80 };
	self->config.limit.temp_yaw = (struct fb_analog_limit){ .min = 860, .max = 2000, .omin = 100, .omax = 100.f };
	self->config.limit.temp_pitch = (struct fb_analog_limit){ .min = 1070, .max = 3150, .omin = -100, .omax = 100.f };
}

static void _fb_calibrate_current_sensors(struct application *self){
	self->config.dc_cal.pitch_a = 0;
	self->config.dc_cal.pitch_b = 0;
	self->config.dc_cal.yaw_a = 0;
	self->config.dc_cal.yaw_b = 0;

	drv8302_enable_calibration(self->drv_pitch, true);
	drv8302_enable_calibration(self->drv_yaw, true);

	thread_sleep_ms(10);

	printk("Performing dc calibration...\n");

	int loops = 10;
	for(int c = 0; c < loops; c++){
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
		self->config.dc_cal.pitch_a,
		self->config.dc_cal.pitch_b,
		self->config.dc_cal.yaw_a,
		self->config.dc_cal.yaw_b
	);

	if(abs(self->config.dc_cal.pitch_a - 2048) > 100 ||
		abs(self->config.dc_cal.pitch_b - 2048) > 100){
		printk(PRINT_ERROR "Pitch motor calibration values too low! Check power stage!\n");
	}

	if(abs(self->config.dc_cal.yaw_a - 2048) > 100 ||
		abs(self->config.dc_cal.yaw_b - 2048) > 100){
		printk(PRINT_ERROR "Yaw motor calibration values too low! Check power stage!\n");
	}

	drv8302_enable_calibration(self->drv_pitch, false);
	drv8302_enable_calibration(self->drv_yaw, false);
}

static void _fb_check_connected_devices(struct application *self){
	if(self->mode == FB_MODE_MASTER){
		// check that all leds are connected
		const struct {
			const char *name;
			unsigned int led_idx;
			unsigned int sw_idx;
		} leds[5] = {
			{ .name = "HOME", .led_idx = 3, .sw_idx = 11},
			{ .name = "PRESET1", .led_idx = 4, .sw_idx = 12 },
			{ .name = "PRESET2", .led_idx = 5, .sw_idx = 13 },
			{ .name = "PRESET3", .led_idx = 6, .sw_idx = 14 },
			{ .name = "PRESET4", .led_idx = 7, .sw_idx = 15 }
		};
		for(unsigned c = 0; c < sizeof(leds) / sizeof(leds[0]); c++){
			float volts = 0;
			int r = analog_read(self->sw_leds, leds[c].led_idx, &volts);
			if(gpio_read(self->sw_gpio, 8 + c)){
				printk("SW %s: OK, ", leds[c].name);
			} else {
				printk("SW %s: FAIL, ", leds[c].name);
			}
			if(r != 0 || volts < 0.5){
				printk("LED: FAIL\n", leds[c].name);
			} else {
				printk("LED: OK\n", leds[c].name);
			}
		}
	}
}

static int _fb_events_handler(struct events_subscriber *sub, uint32_t ev){
	struct application *self = container_of(sub, struct application, sub);
	gpio_set(self->debug_gpio, 4);
	gpio_reset(self->debug_gpio, 4);
	return 0;
}

static int _fb_probe(void *fdt, int fdt_node){
	struct application *self = kzmalloc(sizeof(struct application));

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
	if(!canopen_mem){ printk(PRINT_ERROR "fb: canopen missing\n"); return -1; }
	if(!enc1){ printk(PRINT_ERROR "fb: enc1 missing\n"); return -1; }
	if(!enc2){ printk(PRINT_ERROR "fb: enc2 missing\n"); return -1; }
	if(!enc1_gpio){ printk(PRINT_ERROR "fb: enc1_gpio missing\n"); return -1; }
	if(!enc2_gpio){ printk(PRINT_ERROR "fb: enc2_gpio missing\n"); return -1; }
	if(!gpio_ex){ printk(PRINT_ERROR "fb: gpio_ex missing\n"); return -1; }
	if(!drv_pitch){ printk(PRINT_ERROR "fb: drv_pitch missing\n"); return -1; }
	if(!drv_yaw){ printk(PRINT_ERROR "fb: drv_yaw missing\n"); return -1; }

	console_add_command(console, self, "fb", "Flying Bergman Control", "", _fb_cmd);
	console_add_command(console, self, "reg", "Register ops", "", _reg_cmd);
	console_add_command(console, self, "motor", "Motor slave board control", "", _motor_cmd);
	console_add_command(console, self, "ui", "GUI interface", "", _ui_cmd);

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
	//self->canopen = canopen;

	self->slave.last_valid = micros();
	self->remote.last_pitch_update = micros();
	self->remote.last_yaw_update = micros();

	_fb_load_config(self);

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

	// add the device profile map
	regmap_range_init(&self->comm_range, CANOPEN_COMM_RANGE_START, CANOPEN_COMM_RANGE_END, &_comm_range_ops);
	regmap_add(regmap, &self->comm_range);

	regmap_range_init(&self->mfr_range, CANOPEN_MFR_RANGE_START, CANOPEN_MFR_RANGE_END, &_mfr_range_ops);
	regmap_add(regmap, &self->mfr_range);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	_fb_calibrate_current_sensors(self);

	_fb_check_connected_devices(self);

	_fb_enter_state(self, _fb_state_wait_power);

	thread_create(
		  _fb_control_loop,
		  "fb_ctrl",
		  650,
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

	return 0;
}

static int _fb_remove(void *fdt, int fdt_node){
	// TODO
    return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _fb_probe, _fb_remove)
