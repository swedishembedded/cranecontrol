#pragma once

struct derivative {
	float pgain;
	float dgain;
	float state;
};

void derivative_init(struct derivative *self, float pole, float gain);
float derivative_update(struct derivative *self, float value);
