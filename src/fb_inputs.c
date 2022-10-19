/** :ms-top-comment
 *  ____            _        _   ____  _     ____
 * |  _ \ ___   ___| | _____| |_|  _ \| |   / ___|
 * | |_) / _ \ / __| |/ / _ \ __| |_) | |  | |
 * |  _ < (_) | (__|   <  __/ |_|  __/| |__| |___
 * |_| \_\___/ \___|_|\_\___|\__|_|   |_____\____|
 *
 * Copyright (c) 2020, Martin K. Schröder, All Rights Reserved
 *
 * RocketPLC is distributed under GPLv2
 *
 * Commercial licensing: http://swedishembedded.com/rocketplc
 * Contact: info@swedishembedded.com
 * Secondary email: mkschreder.uk@gmail.com
 **/
#include "fb.h"

static void _next_mux_chan(struct fb_inputs *self)
{
	self->mux_chan = (self->mux_chan + 1) & 0x7;

	gpio_write(self->mux, 0, (self->mux_chan >> 0) & 1);
	gpio_write(self->mux, 1, (self->mux_chan >> 1) & 1);
	gpio_write(self->mux, 2, (self->mux_chan >> 2) & 1);
}

static void _fb_read_pots(struct fb_inputs *self)
{
	adc_read(self->adc, FB_ADC_MUX_CHAN, &self->mux_adc[self->mux_chan]);

	uint16_t value = self->mux_adc[self->mux_chan];
	switch (self->mux_chan) {
	case FB_ADCMUX_YAW_CHAN: {
		self->joy_yaw = value;
	} break;
	case FB_ADCMUX_PITCH_CHAN: {
		self->joy_pitch = value;
	} break;
	case FB_ADCMUX_YAW_ACC_CHAN: {
		self->yaw_acc = value;
	} break;
	case FB_ADCMUX_PITCH_ACC_CHAN: {
		self->pitch_acc = value;
	} break;
	case FB_ADCMUX_YAW_SPEED_CHAN: {
		self->yaw_speed = value;
	} break;
	case FB_ADCMUX_MOTOR_PITCH_CHAN: {
		//self->pitch = (uint16_t)constrain_i32(4096 - value, 0, 4096);
		self->pitch = value;
	} break;
	case FB_ADCMUX_PITCH_SPEED_CHAN: {
		self->pitch_speed = value;
	} break;
	}

	_next_mux_chan(self);
}

static void _fb_read_pots_local(struct fb_inputs *self)
{
	adc_read(self->adc, FB_ADC_MUX_CHAN, &self->mux_adc[self->mux_chan]);

	uint16_t value = self->mux_adc[self->mux_chan];
	switch (self->mux_chan) {
	case FB_ADCMUX_MOTOR_PITCH_CHAN: {
		self->pitch = value;
	} break;
	}
	self->joy_yaw = self->local.joy_yaw;
	self->joy_pitch = self->local.joy_pitch;
	self->pitch_acc = self->local.pitch_acc;
	self->yaw_acc = self->local.yaw_acc;
	self->pitch_speed = self->local.pitch_speed;
	self->yaw_speed = self->local.yaw_speed;

	_next_mux_chan(self);
}

void _fb_read_switches_local(struct fb_inputs *self)
{
	self->enc1_aux1 = self->local.enc1_aux1;
	self->enc1_aux2 = self->local.enc1_aux2;
	self->enc2_aux1 = self->local.enc2_aux1;
	self->enc2_aux2 = self->local.enc2_aux2;
}

void _fb_read_switches(struct fb_inputs *self)
{
	self->enc1_aux1 = gpio_read(self->enc1_gpio, 1);
	self->enc1_aux2 = gpio_read(self->enc1_gpio, 2);
	self->enc2_aux1 = gpio_read(self->enc2_gpio, 1);
	self->enc2_aux2 = gpio_read(self->enc2_gpio, 2);
}

void _fb_read_inputs(struct fb_inputs *self)
{
	if (self->use_local) {
		_fb_read_pots_local(self);
		_fb_read_switches_local(self);
	} else {
		_fb_read_pots(self);
		_fb_read_switches(self);
	}

	// read switches
	for (unsigned int c = 0; c < 8; c++) {
		bool on = !gpio_read(self->sw_gpio, 8 + c);
		self->sw[c].toggled = on != self->sw[c].pressed;
		self->sw[c].pressed = on;

		if (self->sw[c].toggled && !on) {
			self->sw[c].long_pressed = false;
		} else if (self->sw[c].toggled && on) {
			self->sw[c].pressed_time = timestamp();
		} else if (on) {
			timestamp_t ts = timestamp_add_us(self->sw[c].pressed_time,
							  FB_BTN_LONG_PRESS_TIME_US);
			if (timestamp_expired(ts)) {
				self->sw[c].long_pressed = true;
			} else {
				self->sw[c].long_pressed = false;
			}
		}
	}

	adc_read(self->adc, FB_ADC_VSA1_CHAN, &self->vsa_yaw);
	adc_read(self->adc, FB_ADC_VSB1_CHAN, &self->vsb_yaw);
	adc_read(self->adc, FB_ADC_VSC1_CHAN, &self->vsc_yaw);
	adc_read(self->adc, FB_ADC_VSA2_CHAN, &self->vsa_pitch);
	adc_read(self->adc, FB_ADC_VSB2_CHAN, &self->vsb_pitch);
	adc_read(self->adc, FB_ADC_VSC2_CHAN, &self->vsc_pitch);
	adc_read(self->adc, FB_ADC_IA1_CHAN, &self->ia_yaw);
	adc_read(self->adc, FB_ADC_IB1_CHAN, &self->ib_yaw);
	adc_read(self->adc, FB_ADC_IA2_CHAN, &self->ia_pitch);
	adc_read(self->adc, FB_ADC_IB2_CHAN, &self->ib_pitch);
	adc_read(self->adc, FB_ADC_VMOT_CHAN, &self->vmot);
	adc_read(self->adc, FB_ADC_TEMP1_CHAN, &self->temp_yaw);
	adc_read(self->adc, FB_ADC_TEMP2_CHAN, &self->temp_pitch);

	self->yaw = (int16_t)encoder_read(self->enc1);

	// read CAN address switch
	self->can_addr = ~((gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR3) << 3) |
			   (gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR2) << 2) |
			   (gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR1) << 1) |
			   (gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR0) << 0)) &
			 0x0f;
}

void fb_inputs_update(struct fb_inputs *self)
{
	timestamp_t te, ts;
	timestamp_diff_t inputs_td;
	// measure timing
	ts = timestamp();
	_fb_read_inputs(self);
	te = timestamp();
	inputs_td = timestamp_sub(te, ts);

	thread_mutex_lock(&self->lock);
	self->stats.inputs_td = inputs_td;
	thread_mutex_unlock(&self->lock);
}

void fb_inputs_init(struct fb_inputs *self)
{
	self->local.joy_pitch = 2048;
	self->local.joy_yaw = 2048;
	self->local.pitch_acc = 2048;
	self->local.yaw_acc = 2048;
	self->local.pitch_speed = 2048;
	self->local.yaw_speed = 2048;
	self->local.enc1_aux1 = 0;
	self->local.enc2_aux1 = 1;
	self->local.enc1_aux2 = 0;
	self->local.enc2_aux2 = 1;

	thread_mutex_init(&self->lock);
}
