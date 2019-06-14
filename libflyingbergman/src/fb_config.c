#include "fb_config.h"
#include "flyingbergman.h"

void fb_config_init(struct fb_config *self) {
	memset(self, 0, sizeof(*self));
	self->axis[FB_AXIS_UPDOWN].error_filter =
	    (struct fb_config_filter){.a0 = -1.9112f,
	                              .a1 = 0.914976f,
	                              .b0 = 0.0019161f,
	                              .b1 = 0.00186018f};
	float pff = 5.95f;
	float pkp = 18.f;
	float pki = 0.0f;
	float pkd = 0.2f;
	float yff = 0.74f * (90.f / 17.f);
	float ykp = 2.f;
	float yki = 0.0f;
	float ykd = 0.2f;

}
