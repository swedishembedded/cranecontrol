#include "control/lowpass.h"

void lowpass_init(struct lowpass *self, float gain){
	self->state = 0;
	self->gain = gain;
}

float lowpass_update(struct lowpass *self, float input){
	self->state += self->gain * (input - self->state);
	return self->state;
}
