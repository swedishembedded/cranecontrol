#pragma once

struct lowpass {
	float state;
	float gain;
};

void lowpass_init(struct lowpass *self, float gain);
float lowpass_update(struct lowpass *self, float input);
