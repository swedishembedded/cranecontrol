#include "flyingbergman.h"

void _fb_read_inputs(struct application *self){
	// read next adc mux channel
	thread_mutex_lock(&self->inputs.lock);
	adc_read(self->adc, FB_ADC_MUX_CHAN, &self->mux_adc[self->mux_chan]);

	uint16_t value = self->mux_adc[self->mux_chan];
	switch(self->mux_chan) {
		case FB_ADCMUX_YAW_CHAN: { self->inputs.joy_yaw = value; } break;
		case FB_ADCMUX_PITCH_CHAN: { self->inputs.joy_pitch = value; } break;
		case FB_ADCMUX_YAW_ACC_CHAN: { self->inputs.yaw_acc = value; } break;
		case FB_ADCMUX_PITCH_ACC_CHAN: { self->inputs.pitch_acc = value; } break;
		case FB_ADCMUX_YAW_SPEED_CHAN: { self->inputs.yaw_speed = value; } break;
		case FB_ADCMUX_MOTOR_PITCH_CHAN: { self->inputs.pitch = value; } break;
		case FB_ADCMUX_PITCH_SPEED_CHAN: { self->inputs.pitch_speed = value; } break;
	}

	self->mux_chan = (self->mux_chan+1) & 0x7;

	gpio_write(self->mux, 0, (self->mux_chan >> 0) & 1);
	gpio_write(self->mux, 1, (self->mux_chan >> 1) & 1);
	gpio_write(self->mux, 2, (self->mux_chan >> 2) & 1);

	// read switches
	for(unsigned int c = 0; c < 8; c++){
		bool on = !gpio_read(self->sw_gpio, 8 + c);
		self->inputs.sw[c].toggled = on != self->inputs.sw[c].pressed;
		if(self->inputs.sw[c].toggled || !on){
			self->inputs.sw[c].pressed_time = micros();
		}
		self->inputs.sw[c].pressed = on;
	}

	adc_read(self->adc, FB_ADC_VSA1_CHAN, &self->inputs.vsa_yaw);
	adc_read(self->adc, FB_ADC_VSB1_CHAN, &self->inputs.vsb_yaw);
	adc_read(self->adc, FB_ADC_VSC1_CHAN, &self->inputs.vsc_yaw);
	adc_read(self->adc, FB_ADC_VSA2_CHAN, &self->inputs.vsa_pitch);
	adc_read(self->adc, FB_ADC_VSB2_CHAN, &self->inputs.vsb_pitch);
	adc_read(self->adc, FB_ADC_VSC2_CHAN, &self->inputs.vsc_pitch);
	adc_read(self->adc, FB_ADC_IA1_CHAN, &self->inputs.ia_yaw);
	adc_read(self->adc, FB_ADC_IB1_CHAN, &self->inputs.ib_yaw);
	adc_read(self->adc, FB_ADC_IA2_CHAN, &self->inputs.ia_pitch);
	adc_read(self->adc, FB_ADC_IB2_CHAN, &self->inputs.ib_pitch);
	adc_read(self->adc, FB_ADC_VMOT_CHAN, &self->inputs.vmot);
	adc_read(self->adc, FB_ADC_TEMP1_CHAN, &self->inputs.temp_yaw);
	adc_read(self->adc, FB_ADC_TEMP2_CHAN, &self->inputs.temp_pitch);

	self->inputs.yaw = (int16_t)(uint16_t)encoder_read(self->enc1);

	// read CAN address switch
	self->inputs.can_addr = ~(
		(gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR3) << 3) |
		(gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR2) << 2) |
		(gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR1) << 1) |
		(gpio_read(self->gpio_ex, FB_GPIO_CAN_ADDR0) << 0)) & 0x0f;

	self->inputs.enc1_aux1 = gpio_read(self->enc1_gpio, 1);
	self->inputs.enc1_aux2 = gpio_read(self->enc1_gpio, 2);
	self->inputs.enc2_aux1 = gpio_read(self->enc2_gpio, 1);
	self->inputs.enc2_aux2 = gpio_read(self->enc2_gpio, 2);
	thread_mutex_unlock(&self->inputs.lock);
}
