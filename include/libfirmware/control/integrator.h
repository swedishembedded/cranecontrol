#pragma once

struct integrator {
	float state;
	float min, max;
	float gain;
};

void integrator_init(struct integrator *self, float gain, float min, float max);
float integrator_update(struct integrator *self, float input);
