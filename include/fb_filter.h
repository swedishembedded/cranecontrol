#pragma once

struct fb_config_filter;
struct fb_filter {
	const struct fb_config_filter *conf;
	float in[2], out[2];
};

void fb_filter_init(struct fb_filter *self, const struct fb_config_filter *conf);
void fb_filter_init_default(struct fb_filter *self);
float fb_filter_update(struct fb_filter *self, float value);
