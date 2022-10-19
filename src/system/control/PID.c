#include "control/PID.h"

void PID_init(struct PID *self, float pGain, float iGain, float dGain)
{
	self->iState = 0;
	self->iMin = -1;
	self->iMax = 1;
	self->iGain = iGain;
	self->pGain = pGain;
	self->dState = 0;
	self->dpGain = 1;
	self->dGain = dGain;
}

void PID_set_integral_limit(struct PID *self, float min, float max)
{
	if (max < min)
		return;
	self->iMin = min;
	self->iMax = max;
}

void PID_set_derivative_pole_gain(struct PID *self, float pole, float gain)
{
	self->dpGain = 1.f - pole;
	self->dGain = gain * (1.f - pole);
}

float PID_update(struct PID *self, float input)
{
	float retval, dTemp;
	float pTerm = input * self->pGain;

	self->iState = self->iState + self->iGain * input;
	retval = self->iState + pTerm;
	if (retval > self->iMax) {
		self->iState = self->iMax - pTerm;
		retval = self->iMax;
	} else if (retval < self->iMin) {
		self->iState = self->iMin - pTerm;
		retval = self->iMin;
	}

	dTemp = input - self->dState;
	self->dState += self->dpGain * dTemp;

	return retval + dTemp * self->dGain;
}
