#include "flyingbergman.h"

float _fb_filter(struct fb_filter *self, struct fb_filter_config conf, float in){
	float out = conf.b0 * in + conf.b1 * self->in[0] - conf.a0 * self->out[0] - conf.a1 * self->out[1];
	self->in[0] = in;
	self->out[1] = self->out[0];
	self->out[0] = out;
	return out;
}

