/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include <string.h>

#include "fb_config.h"
#include "fb_filter.h"

static const struct fb_config_filter _default_conf = { .b0 = 1, .b1 = 0, .a0 = 0, .a1 = 0 };

void fb_filter_init(struct fb_filter *self, const struct fb_config_filter *conf)
{
	memset(self, 0, sizeof(*self));
	self->conf = conf;
}

void fb_filter_init_default(struct fb_filter *self)
{
	fb_filter_init(self, &_default_conf);
}

/**
   Execute a filter specified as conf
 */
float fb_filter_update(struct fb_filter *self, float in)
{
	float out = self->conf->b0 * in + self->conf->b1 * self->in[0] -
		    self->conf->a0 * self->out[0] - self->conf->a1 * self->out[1];
	self->in[0] = in;
	self->out[1] = self->out[0];
	self->out[0] = out;
	return out;
}
