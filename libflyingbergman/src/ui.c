#include "fb.h"
#include <ctype.h>

#define UI_UPDATE_INTERVAL_MS 100

#define TAB "\033[16G"
#define FIELD " \033[44;37m%7d\033[0m "
#define FIXED " \033[44;37m%4d.%-4d\033[0m "
#define FLOAT(num) console_printf(con, FIXED, (int)((num)), (int)(fabsf(num) * 1000) % 1000)
#define STR(str) console_printf(con, " \033[44;37m%9s\033[0m ", str)
#define BOOL(val, on, off) console_printf(con, " \033[44;37m%9s\033[0m ", (val)?on:off)
//#define FLOAT(num) console_printf(con, FIELD, (int)((num * 1000)))
#define NAME(name) console_printf(con, "%s:" TAB, name)
#define NL() console_printf(con, "\n")
#define GOTO(x, y) console_printf(con, "\033[%d;%dH", y, x)
#define HOME() console_printf(self->console, "\033[H")
#define CLEAR() console_printf(self->console, "\033[2J")
#define BLUE_BG() console_printf(con, "\033[44;37m")
#define DEFAULT_BG() console_printf(con, "\033[0m")
#define CURSOR_OFF() console_printf(con, "\033[?25l")
#define CURSOR_ON() console_printf(con, "\033[?25h")

#define CHOICE(val, choice) do { if((unsigned)(val) < (sizeof(choice) / sizeof(choice[0]))) STR(choice[(unsigned)(val)]); } while(0)

static void _hslider(console_device_t con, int px, int py, int val, int val_min, int val_max, int size){
	GOTO(px, py);
	val = constrain_i32(val, val_min, val_max);
	int pos = (val - val_min) * size / (val_max - val_min);
	BLUE_BG();
	for(int c = 0; c < pos; c++){
		console_printf(con, " ");
	}
	DEFAULT_BG();
	console_printf(con, " ");
	BLUE_BG();
	for(int c = pos + 1; c < size; c++){
		console_printf(con, " ");
	}
	DEFAULT_BG();
}

static void _ui_update(struct fb *self){
	console_device_t con = self->console;
	struct fb_meas_data meas;
	struct fb_inputs_data in;

	thread_mutex_lock(&meas.lock);
	memcpy(&meas, &self->measured, sizeof(meas));
	thread_mutex_unlock(&meas.lock);

	thread_mutex_lock(&in.lock);
	memcpy(&in, &self->inputs, sizeof(in));
	thread_mutex_unlock(&in.lock);

	static const char *control_modes[] = {
		"   OFF   ",
		" REMOTE  ",
		" MANUAL  ",
		"  AUTO   "
	};

	HOME();
	NAME("Board Mode"); BOOL(self->mode == FB_MODE_MASTER, "MASTER", "SLAVE"); NL();
	NAME("Control Mode"); CHOICE(self->control_mode, control_modes); NL();

	console_printf(con, "%-17s%-10s%-11s%-11s\n", "", "Phase A", "Phase B", "Phase C");
	NAME("VS PITCH (mV)"); FLOAT(meas.vsa_pitch); FLOAT(meas.vsb_pitch); FLOAT(meas.vsb_pitch); NL();
	NAME("VS YAW (mV)"); FLOAT(meas.vsa_yaw); FLOAT(meas.vsb_yaw); FLOAT(meas.vsb_yaw); NL();
	NAME("I PITCH (mA)"); FLOAT(meas.ia_pitch); FLOAT(meas.ib_pitch); NL();
	NAME("I YAW (mA)"); FLOAT(meas.ia_yaw); FLOAT(meas.ib_yaw); NL();
	console_printf(con, "%-17s%-10s%-11s\n", "", "Pitch", "Yaw");
	NAME("Joystick"); FLOAT(meas.joy_pitch); FLOAT(meas.joy_yaw); NL();
	NAME("Intensity"); FLOAT(meas.pitch_acc); FLOAT(meas.yaw_acc); NL();
	NAME("Speed"); FLOAT(meas.pitch_speed); FLOAT(meas.yaw_speed); NL();
	NAME("Position"); FLOAT(meas.pitch); FLOAT(meas.yaw); NL();
	console_printf(con, "%-17s%-10s\n", "", "Value");
	NAME("VMOT"); FLOAT(meas.vmot); NL();
	NAME("TEMP PITCH"); FLOAT(meas.temp_pitch); NL();
	NAME("TEMP YAW"); FLOAT(meas.temp_yaw); NL();
	_hslider(con, 1, 20, (int)self->ui.joy_yaw, 0, 4096, 32);
}

static int _ui_process_input(struct fb_ui *self, uint8_t ch){
	ch = (uint8_t)tolower(ch);
	switch(ch){
		case 'a': {
			self->joy_yaw -= 100;
		} break;
		case 'd': {
			self->joy_yaw += 100;
		} break;
		case 'w': {
			//self->ui.joy_pitch = 1.f;
		} break;
		case 'q': {
			return -1;
		} break;
	}
	return 0;
}

int _ui_cmd(console_device_t con, void *userptr, int argc, char **argv){
	struct fb *self = (struct fb*)userptr;
	CLEAR();
	CURSOR_OFF();
	while(1){
		_ui_update(self);
		char ch;
		if(console_read(con, &ch, 1, UI_UPDATE_INTERVAL_MS) == 1){
			if(_ui_process_input(&self->ui, (uint8_t)ch) < 0){
				break;
			}
		}
	}
	CURSOR_ON();
	return 0;
}
