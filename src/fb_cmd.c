/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb_cmd.h"
#include "fb.h"
#include "vt.h"
#include <stdio.h>
#include <stdlib.h>

#define print_status(name, ok)                                                      \
	do {                                                                              \
		console_printf(con, PRINT_DEFAULT "%16s: %s\n" PRINT_DEFAULT, name,             \
		               (ok) ? PRINT_SUCCESS "OK" : PRINT_ERROR "FAIL");                 \
	} while(0)

static int _fb_cmd_trace(console_device_t con, void *userptr, int argc,
                         char **argv) {
	struct fb *self = (struct fb *)userptr;
	console_printf(con, "VOLT YAW (mV):"  "%6d %6d %6d\n",
	               (int32_t)(self->measured.vsa_yaw * 1000),
	               (int32_t)(self->measured.vsb_yaw * 1000),
	               (int32_t)(self->measured.vsc_yaw * 1000));
	console_printf(con, "VOLT PITCH (mV):"  "%6d %6d %6d\n",
	               (int32_t)(self->measured.vsa_pitch * 1000),
	               (int32_t)(self->measured.vsb_pitch * 1000),
	               (int32_t)(self->measured.vsc_pitch * 1000));
	console_printf(con, "I YAW (mA):"  "%6d %6d\n",
	               (int32_t)(self->measured.ia_yaw * 1000),
	               (int32_t)(self->measured.ib_yaw * 1000));
	console_printf(con, "I PITCH (mA):"  "%6d %6d\n",
	               (int32_t)(self->measured.ia_pitch * 1000),
	               (int32_t)(self->measured.ib_pitch * 1000));
	return 0;
}

/*
static int _modbus_cmd(console_device_t con, void *userptr, int argc,
                         char **argv) {
	return 0;
}
*/
static int _fb_cmd_an_out(console_device_t con, void *userptr, int argc,
                         char **argv) {
	struct fb *self = (struct fb *)userptr;

	if(argc == 3){
		unsigned channel = 0;
		unsigned value = 0;
		sscanf(argv[1], "%u", &channel);
		sscanf(argv[2], "%u", &value);
		analog_write(self->an_out, channel, (float)value / 1000.f);
		printk("Analog out: %u -> %u\n", channel, value);
	} else {
		printk("Invalid arguments\n");
	}
	return 0;
}

uint8_t __attribute__((__section__(".pg_ram"))) pg_ram[1024];

static unsigned char main_bin[] = {
  0x80, 0xb5, 0x00, 0xaf, 0x00, 0xf0, 0x9e, 0xf8, 0x07, 0x4b, 0x08, 0x4a,
    0x1a, 0x60, 0x08, 0x4b, 0x01, 0x22, 0x1a, 0x72, 0x06, 0x4b, 0x03, 0x22,
	  0xda, 0x60, 0x00, 0x20, 0x00, 0xf0, 0xa6, 0xf8, 0x03, 0x4b, 0x1b, 0x68,
	    0x18, 0x46, 0x80, 0xbd, 0x9c, 0x01, 0x02, 0x20, 0x98, 0x01, 0x02, 0x20,
		  0xa0, 0x01, 0x02, 0x20, 0x80, 0xb4, 0x83, 0xb0, 0x00, 0xaf, 0x78, 0x60,
		    0x7b, 0x68, 0x18, 0x46, 0x0c, 0x37, 0xbd, 0x46, 0x5d, 0xf8, 0x04, 0x7b,
			  0x70, 0x47, 0x00, 0xbf, 0x80, 0xb5, 0x82, 0xb0, 0x00, 0xaf, 0x39, 0x60,
			    0x13, 0x46, 0x02, 0x46, 0xfa, 0x71, 0xbb, 0x71, 0xfb, 0x79, 0x00, 0x2b,
				  0x07, 0xd1, 0x3b, 0x68, 0x00, 0x2b, 0x02, 0xd0, 0x3b, 0x68, 0x00, 0x22,
				    0x1a, 0x70, 0x00, 0x23, 0x0a, 0xe0, 0x3b, 0x68, 0x00, 0x2b, 0x02, 0xd0,
					  0x3b, 0x68, 0x01, 0x22, 0x1a, 0x70, 0xbb, 0x79, 0x18, 0x46, 0xff, 0xf7,
					    0xd7, 0xff, 0x03, 0x46, 0x18, 0x46, 0x08, 0x37, 0xbd, 0x46, 0x80, 0xbd,
						  0x80, 0xb4, 0x83, 0xb0, 0x00, 0xaf, 0x78, 0x60, 0x0b, 0x46, 0xfb, 0x70,
						    0x7b, 0x68, 0x00, 0x22, 0x1a, 0x72, 0x7b, 0x68, 0x5b, 0x7a, 0xda, 0xb2,
							  0xfb, 0x78, 0x00, 0x2b, 0x01, 0xd0, 0x04, 0x23, 0x00, 0xe0, 0x00, 0x23,
							    0x13, 0x43, 0xdb, 0xb2, 0xda, 0xb2, 0x7b, 0x68, 0x5a, 0x72, 0x7b, 0x68,
								  0x00, 0x22, 0xda, 0x60, 0x7b, 0x68, 0x1b, 0x7c, 0xda, 0xb2, 0xfb, 0x78,
								    0x00, 0x2b, 0x01, 0xd0, 0x04, 0x23, 0x00, 0xe0, 0x00, 0x23, 0x13, 0x43,
									  0xdb, 0xb2, 0xda, 0xb2, 0x7b, 0x68, 0x1a, 0x74, 0x7b, 0x68, 0x00, 0x22,
									    0x1a, 0x60, 0x7b, 0x68, 0x1b, 0x79, 0xda, 0xb2, 0xfb, 0x78, 0x00, 0x2b,
										  0x01, 0xd0, 0x04, 0x23, 0x00, 0xe0, 0x00, 0x23, 0x13, 0x43, 0xdb, 0xb2,
										    0xda, 0xb2, 0x7b, 0x68, 0x1a, 0x71, 0x0c, 0x37, 0xbd, 0x46, 0x5d, 0xf8,
											  0x04, 0x7b, 0x70, 0x47, 0x80, 0xb5, 0x82, 0xb0, 0x00, 0xaf, 0x78, 0x60,
											    0x7b, 0x68, 0x1b, 0x79, 0x03, 0xf0, 0x02, 0x03, 0x00, 0x2b, 0x0c, 0xd1,
												  0x7b, 0x68, 0x1b, 0x7a, 0x01, 0x20, 0x00, 0x21, 0x1a, 0x46, 0xff, 0xf7,
												    0x8f, 0xff, 0x02, 0x46, 0x7b, 0x68, 0xdb, 0x68, 0x1a, 0x44, 0x7b, 0x68,
													  0x1a, 0x60, 0x00, 0xbf, 0x00, 0xbf, 0x08, 0x37, 0xbd, 0x46, 0x80, 0xbd,
													    0x80, 0xb5, 0x82, 0xb0, 0x00, 0xaf, 0x00, 0x23, 0xfb, 0x71, 0x05, 0x4b,
														  0x00, 0x22, 0x1a, 0x70, 0xfb, 0x79, 0x04, 0x48, 0x19, 0x46, 0xff, 0xf7,
														    0x99, 0xff, 0x08, 0x37, 0xbd, 0x46, 0x80, 0xbd, 0xc4, 0x01, 0x02, 0x20,
															  0xa0, 0x01, 0x02, 0x20, 0x80, 0xb5, 0x82, 0xb0, 0x00, 0xaf, 0x78, 0x60,
															    0x06, 0x4b, 0x01, 0x22, 0x1a, 0x70, 0x05, 0x4b, 0x1b, 0x78, 0x00, 0x2b,
																  0x02, 0xd0, 0x04, 0x48, 0xff, 0xf7, 0xc2, 0xff, 0x08, 0x37, 0xbd, 0x46,
																    0x80, 0xbd, 0x00, 0xbf, 0xc4, 0x01, 0x02, 0x20, 0xa0, 0x01, 0x02, 0x20,
																	  0x01, 0x00, 0x00, 0x00
};

static int run_pg_ram(){
	int (*pg_ram_code)() = (int(*)())0x20020001;
	int ret = pg_ram_code();
	return ret;
}

static int _fb_cmd(console_device_t con, void *userptr, int argc, char **argv) {
	struct fb *self = (struct fb *)userptr;
	if(argc == 2 && strcmp(argv[1], "run") == 0) {
		memcpy(pg_ram, main_bin, sizeof(main_bin));
		printk("result = %d\n", run_pg_ram());
	} else if(argc == 2 && strcmp(argv[1], "sw") == 0) {
		for(int c = 0; c < 8; c++) {
			int val = (int)gpio_read(self->inputs.sw_gpio, (uint32_t)(8 + c));
			console_printf(con, "SW%d: %d\n", c, val);
		}
	} else if(argc == 2 && strcmp(argv[1], "status") == 0) {
		console_printf(con, "CONTROL MODE: %d\n", self->control_mode);
		console_printf(con, "Pitch: \n");
		print_status("h-bridge", !drv8302_is_in_error(self->drv_pitch) &&
		                             !drv8302_is_in_overcurrent(self->drv_pitch));
		print_status("dc-cal", abs(self->config.dc_cal.pitch_a - 2048) < 100 &&
		                           abs(self->config.dc_cal.pitch_b - 2048) < 100);
		console_printf(con, "Yaw: \n");
		print_status("h-bridge", !drv8302_is_in_error(self->drv_yaw) &&
		                             !drv8302_is_in_overcurrent(self->drv_yaw));
		print_status("dc-cal", abs(self->config.dc_cal.yaw_a - 2048) < 100 &&
		                           abs(self->config.dc_cal.yaw_b - 2048) < 100);
	} else if(argc == 2 && strcmp(argv[1], "outputs") == 0) {
		console_printf(con, "PITCH %d\nYAW %d\n",
		               (int32_t)(self->output.pitch * 10000), // output pitch
		               (int32_t)(self->output.yaw * 10000)    // output yaw
		);
		console_printf(
		    con, "CONTROL PITCH: Err %d, I %d\n",
		    (int32_t)(self->axis[FB_AXIS_UPDOWN].error * 1000),   // output pitch
		    (int32_t)(self->axis[FB_AXIS_UPDOWN].integral * 1000) // output yaw
		);
		console_printf(con, "CONTROL YAW: Err %d, I %d, Target %d\n",
		               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].error * 1000),
		               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].integral * 1000),
		               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].target.pos * 1000));
	} else if(argc == 2 && strcmp(argv[1], "inputs") == 0) {
		char ch;
		vt_clear_screen(con);
		do {
			vt_cursor_home(con);
#define TITLE_FMT "%-20s "
			console_printf(con, TITLE_FMT "%u\n", "US:", micros());
			console_printf(con, TITLE_FMT, "SW:");
			for(int c = 0; c < FB_SWITCH_COUNT; c++) {
				console_printf(con, "[%2d] [%d%c] ", c, self->inputs.sw[c].pressed,
				               (self->inputs.sw[c].long_pressed) ? '-' : ' ');
			}
			console_printf(con, "\n");

			console_printf(con, TITLE_FMT, "AIN:");
			for(int c = 0; c < FB_SWITCH_COUNT; c++) {
				console_printf(con, "[%2d] [%5d] ", c, self->inputs.mux_adc[c]);
			}
			console_printf(con, "\n");

			console_printf(con, TITLE_FMT, "ADC:");
			for(unsigned c = 0; c < 16; c++) {
				uint16_t val = 0;
				adc_read(self->inputs.adc, c, &val);
				console_printf(con, "[%2d] [%5d] ", c, val);
			}
			console_printf(con, "\n");

			console_printf(con, TITLE_FMT "COUNT [%d], Di2 [%d], Di3 [%d]\n",
			               "ENC1:", encoder_read(self->inputs.enc1),
			               self->inputs.enc1_aux1, self->inputs.enc1_aux2);
			console_printf(con, TITLE_FMT "COUNT [%d], Di2 [%d], Di3 [%d]\n",
			               "ENC2:", encoder_read(self->inputs.enc2),
			               self->inputs.enc2_aux1, gpio_read(self->inputs.enc2_gpio, 2)
			               // self->inputs.enc2_aux2
			);

			console_printf(con, TITLE_FMT "[%5d] [%5d] [%5d]\n",
			               "VS1:", self->inputs.vsa_yaw, self->inputs.vsb_yaw,
			               self->inputs.vsc_yaw);
			console_printf(con, TITLE_FMT "[%5d] [%5d] [%5d]\n",
			               "VS2:", self->inputs.vsa_pitch, self->inputs.vsb_pitch,
			               self->inputs.vsc_pitch);
			console_printf(con, TITLE_FMT "[%5d] [%5d]\n", "I1:", self->inputs.ia_yaw,
			               self->inputs.ib_yaw);
			console_printf(con, TITLE_FMT "[%5d] [%5d]\n", "I2:", self->inputs.ia_pitch,
			               self->inputs.ib_pitch);

			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "JOYSTICK:", self->inputs.joy_yaw, self->inputs.joy_pitch);
			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "INTENSITY:", self->inputs.yaw_acc, self->inputs.pitch_acc);
			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "SPEED:", self->inputs.yaw_speed, self->inputs.pitch_speed);
			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "POSITION:", self->state.yaw_ticks, self->inputs.pitch);
			console_printf(con, TITLE_FMT "[%5d]\n", "VMOT:", self->inputs.vmot);
			console_printf(con, TITLE_FMT "[%5d]\n", "TEMP_YAW:", self->inputs.temp_yaw);
			console_printf(con, TITLE_FMT "[%5d]\n",
			               "TEMP_PITCH:", self->inputs.temp_pitch);
			console_printf(con, TITLE_FMT "[%02x]\n", "CAN_ADDR:", self->inputs.can_addr);
			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "FROM_MASTER:", self->remote.yaw, self->remote.pitch);
			console_printf(con, TITLE_FMT "YAW [%5d] PITCH [%5d]\n",
			               "SLAVE RAW POS:", self->slave.yaw, self->slave.pitch);
			console_printf(con, "\033[H");
			thread_sleep_ms(10);
		} while(console_read(con, &ch, 1, 0) != 1);
	} else if(argc == 2 && strcmp(argv[1], "meas") == 0) {
#define TAB "\033[16G"
		char ch;
		vt_clear_screen(con);
		do {
			vt_cursor_home(con);
			console_printf(con, "TIME:" TAB "%6d\n", micros());
			console_printf(con, "------- SLAVE --------\n");
			console_printf(con, "VOLT YAW (mV):" TAB "%6d %6d %6d\n",
			               (int32_t)(self->measured.vsa_yaw * 1000),
			               (int32_t)(self->measured.vsb_yaw * 1000),
			               (int32_t)(self->measured.vsc_yaw * 1000));
			console_printf(con, "VOLT PITCH (mV):" TAB "%6d %6d %6d\n",
			               (int32_t)(self->measured.vsa_pitch * 1000),
			               (int32_t)(self->measured.vsb_pitch * 1000),
			               (int32_t)(self->measured.vsc_pitch * 1000));
			console_printf(con, "I YAW (mA):" TAB "%6d %6d\n",
			               (int32_t)(self->measured.ia_yaw * 1000),
			               (int32_t)(self->measured.ib_yaw * 1000));
			console_printf(con, "I PITCH (mA):" TAB "%6d %6d\n",
			               (int32_t)(self->measured.ia_pitch * 1000),
			               (int32_t)(self->measured.ib_pitch * 1000));
			console_printf(con, "VMOT:" TAB "%dmV\n",
			               (int32_t)(self->measured.vmot * 1000));
			console_printf(con, "TEMP_YAW:" TAB "%d mC\n",
			               (int32_t)(self->measured.temp_yaw * 1000));
			console_printf(con, "TEMP_PITCH:" TAB "%d mC\n",
			               (int32_t)(self->measured.temp_pitch * 1000));
			console_printf(con, "------- MASTER --------\n");
			console_printf(con, "JOYSTICK:" TAB "YAW %5d, PITCH %5d\n",
			               (int32_t)(self->measured.joy_yaw * 1000),
			               (int32_t)(self->measured.joy_pitch * 1000));
			console_printf(con, "INTENSITY:" TAB "YAW %5d, PITCH %5d\n",
			               (int32_t)(self->measured.yaw_acc * 1000),
			               (int32_t)(self->measured.pitch_acc * 1000));
			console_printf(con, "SPEED:" TAB "YAW %5d, PITCH %5d\n",
			               (int32_t)(self->measured.yaw_speed * 1000),
			               (int32_t)(self->measured.pitch_speed * 1000));
			console_printf(con, TITLE_FMT "YAW [%5d], PITCH [%5d]\n",
			               "SLAVE MOTOR POS:", self->slave.yaw, self->slave.pitch);
			console_printf(con, "------- COMMON --------\n");
			console_printf(con, "CONTROL OUTPUT:" TAB "YAW %5d, PITCH %5d\n",
			               (int32_t)(self->output.yaw * 1000),
			               (int32_t)(self->output.pitch * 1000));
			console_printf(con, "MOTOR POSITION:" TAB "YAW %5d, PITCH %5d\n",
			               (int32_t)(self->measured.yaw * 1000),
			               (int32_t)(self->measured.pitch * 1000));
			thread_mutex_lock(&self->stats.lock);
			timestamp_diff_t loop_td = self->stats.loop_td;
			timestamp_diff_t inputs_td = self->inputs.stats.inputs_td;
			thread_mutex_unlock(&self->stats.lock);
			console_printf(con, "LOOP TIME:" TAB "%6d.%6d\n", loop_td.sec, loop_td.usec);
			console_printf(con, "INPUTS TIME:" TAB "%6d.%6d\n", inputs_td.sec,
			               inputs_td.usec);

			thread_sleep_ms(10);
		} while(console_read(con, &ch, 1, 0) != 1);
		vt_clear_screen(con);
		console_printf(con, "\n");
	} else if(argc == 2 && strcmp(argv[1], "data") == 0) {
		while(1) {
			console_printf(con, "%u;", micros());
			console_printf(con, "%d;%d;",
			               (int32_t)(self->output.pitch * 10000), // output pitch
			               (int32_t)(self->output.yaw * 10000)    // output yaw
			);

			if(self->mode == FB_MODE_MASTER) {
				console_printf(con, "%d;%d;",
				               self->slave.i_pitch, // current in mA from slave
				               self->slave.i_yaw    // current in mA from slave
				);
				console_printf(
				    con, "%d;%d;",
				    (int32_t)(self->measured.joy_pitch * 10000), // user control pitch
				    (int32_t)(self->measured.joy_yaw * 10000)    // user control yaw
				);
				console_printf(
				    con, "%d;%d;",
				    (int32_t)(self->measured.pitch * 10000), // motor position pitch
				    (int32_t)(self->measured.yaw * 10000)    // motor position yaw
				);
				console_printf(con, "%d;%d;",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].output * 10000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].output * 10000));
				console_printf(con, "%d;%d;",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].target.pos * 10000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].target.pos * 10000));
				console_printf(con, "%d;", self->slave.vmot);
				console_printf(con, "%d;%d;", (int32_t)(self->output.pitch * 1000),
				               (int32_t)(self->output.yaw * 1000));
			} else {
				console_printf(con, "%5d;%5d;%5d;%5d",
				               (int32_t)(self->measured.ia_pitch * 10000),
				               (int32_t)(self->measured.ib_pitch * 10000),
				               (int32_t)(self->measured.ia_yaw * 10000),
				               (int32_t)(self->measured.ib_yaw * 10000));
				console_printf(con, "%5d;%5d;", (int32_t)(self->remote.pitch),
				               (int32_t)(self->remote.yaw));
				console_printf(
				    con, "%d;%d;",
				    (int32_t)(self->measured.pitch * 10000), // motor position pitch
				    (int32_t)(self->measured.yaw * 10000)    // motor position yaw
				);
			}

			console_printf(con, "\n");
			char ch;
			if(console_read(con, &ch, 1, 0) == 1) {
				break;
			}

			thread_sleep_ms(10);
		}
	} else if(argc == 2 && strcmp(argv[1], "enc") == 0) {
		int32_t val = encoder_read(self->inputs.enc1);
		console_printf(con, "ENC1: %d\n", val);
	} else if(argc == 2 && strcmp(argv[1], "can") == 0) {
		struct can_message msg;
		msg.id = 0xaa;
		msg.len = 1;
		msg.data[0] = 0x12;
		// can_send(self->can1, &msg, 100);
		for(int c = 0; c < 100; c++) {
			int err = can_send(self->can2, &msg, 100);
			if(err < 0) {
				printk(PRINT_ERROR "can error: %d %s\n", err, strerror(-err));
			}
		}
	} else if(argc == 2 && strcmp(argv[1], "leds") == 0) {
		for(unsigned c = 0; c < 8; c++) {
			float volts = 0;
			if(analog_read(self->sw_leds, c, &volts) == 0) {
				printk("[%d]: %dmV\n", c, (int32_t)(volts * 1000));
			} else {
				printk(PRINT_ERROR "[%d]: could not read voltage!\n", c);
			}
		}
	} else if(argc == 2 && strcmp(argv[1], "motors") == 0) {
		console_printf(con, "Pitch: %s %s\n",
		               (drv8302_is_in_error(self->drv_pitch)) ? "FAULT" : "NOFAULT",
		               (drv8302_is_in_overcurrent(self->drv_pitch)) ? "OCW" : "NOOCW");
		console_printf(con, "Yaw: %s %s\n",
		               (drv8302_is_in_error(self->drv_yaw)) ? "FAULT" : "NOFAULT",
		               (drv8302_is_in_overcurrent(self->drv_yaw)) ? "OCW" : "NOOCW");
		float gp = (float)drv8302_get_gain(self->drv_pitch);
		float gy = (float)drv8302_get_gain(self->drv_yaw);
		console_printf(con, "Gains: PITCH %d, YAW %d\n", (int32_t)(gp * 1000),
		               (int32_t)(gy * 1000));
	} else if(argc == 3 && strcmp(argv[1], "p") == 0) {
		uint32_t preset = (uint32_t)strtoul(argv[2], NULL, 10);
		if(fb_try_load_preset(self, preset) < 0) {
			console_printf(con, "failed to load preset %d\n", preset);
		} else {
			console_printf(con, "loaded preset %d\n", preset);
			for(unsigned c = 0; c < 50; c++) {
				console_printf(con, "TIME %d %d, ",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].time * 1000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].time * 1000));
				console_printf(con, "TRG_PIT %d, TRG_YAW %d, ",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].new_target * 1000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].new_target * 1000));
				console_printf(
				    con, "PIT: %d, YAW: %d, ",
				    (int32_t)(self->measured.pitch * 1000), // motor position pitch
				    (int32_t)(self->measured.yaw * 1000)    // motor position yaw
				);

				console_printf(con, "PLAN_PIT %d, PLAN_YAW %d, ",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].target.pos * 1000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].target.pos * 1000));
				console_printf(con, "OUT_PIT %d, OUT_YAW %d, ",
				               (int32_t)(self->axis[FB_AXIS_UPDOWN].output * 1000),
				               (int32_t)(self->axis[FB_AXIS_LEFTRIGHT].output * 1000));

				console_printf(con, "\n");
				thread_sleep_ms(100);
			}
		}
	} else if(argc >= 4 && strcmp(argv[1], "set") == 0) {
		if(strcmp(argv[2], "local") == 0) {
			if(atoi(argv[3]) == 1) {
				self->inputs.use_local = true;
				self->config.presets[FB_PRESET_1].valid = true;
				self->config.presets[FB_PRESET_1].pitch = 0;
				self->config.presets[FB_PRESET_1].yaw = 0.5;
			} else {
				self->inputs.use_local = false;
			}
		} else if(strcmp(argv[2], "manual") == 0) {
			if(atoi(argv[3]) == 1) {
				self->control_mode = FB_CONTROL_MODE_MANUAL;
			}
		} else if(strcmp(argv[2], "remote") == 0) {
			if(atoi(argv[3]) == 1) {
				self->control_mode = FB_CONTROL_MODE_REMOTE;
			}
		} else if(strcmp(argv[2], "state") == 0) {
			if(strcmp(argv[3], "operational") == 0) {
				_fb_enter_state(self, _fb_state_operational);
			}
		} else if(strcmp(argv[2], "p0") == 0) {
			if(argc != 5) {
				console_printf(con, "not enough arguments to preset\n");
				return -1;
			}
			long pitch = strtol(argv[3], NULL, 10);
			long yaw = strtol(argv[4], NULL, 10);
			unsigned preset = FB_PRESET_HOME;
			struct fb_config_preset *p = &self->config.presets[preset];
			p->pitch = (float)pitch / 1000.f;
			p->yaw = (float)yaw * M_PI / 180;
			p->valid = true;
			console_printf(con, "set preset %d to %d %d\n", preset, pitch, yaw);
		} else if(strcmp(argv[2], "joy_pitch") == 0) {
			self->inputs.local.joy_pitch = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "joy_yaw") == 0) {
			self->inputs.local.joy_yaw = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "yaw_acc") == 0) {
			self->inputs.local.yaw_acc = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "pitch_acc") == 0) {
			self->inputs.local.pitch_acc = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "pitch_speed") == 0) {
			self->inputs.local.pitch_speed = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "yaw_speed") == 0) {
			self->inputs.local.yaw_speed = (uint16_t)strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "enc1_aux1") == 0) {
			self->inputs.local.enc1_aux1 = !!strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "enc1_aux2") == 0) {
			self->inputs.local.enc1_aux2 = !!strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "enc2_aux1") == 0) {
			self->inputs.local.enc2_aux1 = !!strtoul(argv[3], NULL, 10);
		} else if(strcmp(argv[2], "enc2_aux2") == 0) {
			self->inputs.local.enc2_aux2 = !!strtoul(argv[3], NULL, 10);
		}
	} else {
		console_printf(con, "Invalid option\n");
	}

	return 0;
}

uint32_t irq_get_count(int irq);

static int _reg_cmd(console_device_t con, void *userptr, int argc, char **argv) {
	struct fb *self = (struct fb *)userptr;
	if(argc == 3 && strcmp(argv[1], "get") == 0) {
		uint32_t value;
		unsigned int id = 0;
		sscanf(argv[2], "%x", &id);
		if(regmap_read_u32(self->regmap, (uint32_t)id, &value) < 0) {
			console_printf(con, PRINT_ERROR "specified register not found\n");
		} else {
			console_printf(con, "%08x=%08x\n", id, value);
		}
	} else if(argc == 4 && strcmp(argv[1], "set") == 0) {
		uint32_t value = 0;
		uint32_t id = 0;
		sscanf(argv[2], "%x", (unsigned int *)&id);
		sscanf(argv[3], "%x", (unsigned int *)&value);
		regmap_write_u32(self->regmap, id, value);
	} else {
		return -1;
	}
	return 0;
}

static int _motor_cmd(console_device_t con, void *userptr, int argc, char **argv) {
	struct fb *self = (struct fb *)userptr;
	if(argc == 2 && strcmp(argv[1], "info") == 0) {
		uint32_t addr = 0x02000000;
		uint32_t type = 0, error = 0, mfr_status = 0;
		int ret = 0;
#define _read(reg, value)                                                           \
	ret |= memory_read(self->canopen_mem, addr | reg, &value, sizeof(value))
		_read(CANOPEN_REG_DEVICE_TYPE, type);
		_read(CANOPEN_REG_DEVICE_ERROR, error);
		_read(CANOPEN_REG_DEVICE_MFR_STATUS, mfr_status);
#undef _read
		printk("Type: %d\n", type);
		printk("Error: %08x\n", error);
		printk("MFR Status: %08x\n", mfr_status);
	} else if(argc == 2 && strcmp(argv[1], "regs") == 0) {
		for(uint32_t c = 0; c < 0x1fff00; c += 0x0100) {
			uint32_t reg = 0;
			uint32_t ofs = 0x01000000 | c;
			if(memory_read(self->canopen_mem, ofs, &reg, sizeof(reg)) >= 0) {
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

void fb_cmd_init(struct fb *self) {
	console_add_command(self->console, self, "trace",
	                    "A trace utility that outputs VCD file", "", _fb_cmd_trace);
	console_add_command(self->console, self, "an_out",
	                    "Analog output functions", "", _fb_cmd_an_out);

	console_add_command(self->console, self, "fb", "Flying Bergman Control", "",
	                    _fb_cmd);
	console_add_command(self->console, self, "reg", "Register ops", "", _reg_cmd);
	console_add_command(self->console, self, "motor", "Motor slave board control", "",
	                    _motor_cmd);
	console_add_command(self->console, self, "ui", "GUI interface", "", _ui_cmd);
}
