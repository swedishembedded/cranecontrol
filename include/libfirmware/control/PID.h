#pragma once

struct PID {
	float iState;
	float iMax, iMin;
	float iGain;
	float pGain;
	float dState;
	float dpGain;
	float dGain;
};

void PID_init(struct PID *self, float pGain, float iGain, float dGain);
void PID_set_integral_limit(struct PID *self, float min, float max);
float PID_update(struct PID *self, float input);
