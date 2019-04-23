#include "flyingbergman.h"

static float _scale_input(float in, const struct fb_analog_limit *lim){
	in = constrain_float(in, lim->min, lim->max);
	float iscale = lim->max - lim->min;
	float oscale = lim->omax - lim->omin;
	float res = 0;
	if(iscale > 0)
		res = (in - lim->min) / iscale;
	return lim->omin + res * oscale;
}

void _fb_update_measurements(struct application *self){
	thread_mutex_lock(&self->measured.lock);

	if(self->mode == FB_MODE_MASTER){
		//self->measured.pitch = self->slave.pitch / 1000.f;
		float pitch = self->slave.pitch / 1000.f;
		self->measured.pitch = _fb_filter(&self->measured.pfilt, (struct fb_filter_config){.a0 = -1.44739, .a1 = 0.567971, .b0 = 0.0659765, .b1 = 0.0546078}, pitch);
		self->measured.yaw = self->slave.yaw / 1000.f;
	} else {
		self->measured.pitch = _scale_input((float)self->inputs.pitch, &self->config.limit.pitch);
		int16_t yaw_ticks = self->inputs.yaw;

		// convert ticks to radians. How this is done is important.
		self->state.yaw_ticks += (int16_t)(yaw_ticks - self->state.prev_yaw_ticks);
		int32_t ticks_per_pi = (int32_t)(FB_MOTOR_YAW_TICKS_PER_ROT / 2.f);
		if(self->state.yaw_ticks > ticks_per_pi) self->state.yaw_ticks -= 2 * ticks_per_pi;
		if(self->state.yaw_ticks < -ticks_per_pi) self->state.yaw_ticks += 2 * ticks_per_pi;
		self->state.prev_yaw_ticks = yaw_ticks;
		self->measured.yaw = M_PI * ((float)self->state.yaw_ticks / (float)ticks_per_pi);
	}

	float _joy_pitch_dir = (self->inputs.enc2_aux1 == 0)?-1:((self->inputs.enc1_aux1 == 0)?1:0);
	float _joy_yaw_dir = (self->inputs.enc2_aux2 == 0)?-1:((self->inputs.enc1_aux2 == 0)?1:0);

	self->measured.joy_pitch = _joy_pitch_dir * _scale_input((float)self->inputs.joy_pitch, &self->config.limit.joy_pitch);
	self->measured.joy_yaw = _joy_yaw_dir * _scale_input((float)self->inputs.joy_yaw, &self->config.limit.joy_yaw);
	self->measured.pitch_acc = _scale_input((float)self->inputs.pitch_acc, &self->config.limit.pitch_acc);
	self->measured.yaw_acc = _scale_input((float)self->inputs.yaw_acc, &self->config.limit.yaw_acc);
	self->measured.pitch_speed = _scale_input((float)self->inputs.pitch_speed, &self->config.limit.pitch_speed);
	self->measured.yaw_speed = _scale_input((float)self->inputs.yaw_speed, &self->config.limit.yaw_speed);
	self->measured.vmot = _scale_input((float)self->inputs.vmot, &self->config.limit.vmot);

	float gp = (float)drv8302_get_gain(self->drv_pitch);
	float gy = (float)drv8302_get_gain(self->drv_yaw);
	float res = 0.005;
	float scale = 2048.f / (3.3f / 2);
	
	self->measured.ia_pitch = ((float)self->inputs.ia_pitch - (float)self->config.dc_cal.pitch_a) / scale / gp / res;
	self->measured.ib_pitch = ((float)self->inputs.ib_pitch - (float)self->config.dc_cal.pitch_b) / scale / gp / res;
	self->measured.ia_yaw = ((float)self->inputs.ia_yaw - (float)self->config.dc_cal.yaw_a) / scale / gy / res;
	self->measured.ib_yaw = ((float)self->inputs.ib_yaw - (float)self->config.dc_cal.yaw_b) / scale / gy / res;

	float ntc_b = 3400;
	float ambient_temp_k = 273.15f;

	self->measured.temp_yaw = 1.f / (1.f / ambient_temp_k + 1.f / ntc_b * logf((float)self->inputs.temp_yaw / 4096.f)) - ambient_temp_k;
	self->measured.temp_pitch = 1.f / (1.f / ambient_temp_k + 1.f / ntc_b * logf((float)self->inputs.temp_pitch / 4096.f)) - ambient_temp_k;

	if(self->can_addr != self->inputs.can_addr){
		fb_mode_t mode = 0;
		if(self->inputs.can_addr == FB_CANOPEN_MASTER_ADDRESS){
			mode = FB_MODE_MASTER;
		} else {
			mode = FB_MODE_SLAVE;
		}

		canopen_set_address(self->canopen_mem, self->inputs.can_addr);
		canopen_set_mode(self->canopen_mem, (mode == FB_MODE_MASTER)?CANOPEN_MASTER:CANOPEN_SLAVE);

		if(mode == FB_MODE_MASTER){
			regmap_write_u32(self->regmap, CANOPEN_REG_DEVICE_CYCLE_PERIOD, 1000);
		}

		self->mode = mode;
		self->can_addr = self->inputs.can_addr;
	}

	thread_mutex_unlock(&self->measured.lock);
}

