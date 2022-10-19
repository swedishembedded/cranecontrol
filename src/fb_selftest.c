#include "fb_selftest.h"
#include "fb.h"
#include "vt.h"

#define EXPECT_LE(a, b, con, fmt, ...)                                                             \
	do {                                                                                       \
		if ((a) > (b))                                                                     \
			console_printf(con, "FAIL (%s:%d): " fmt, __FILE__, __LINE__,              \
				       ##__VA_ARGS__);                                             \
	} while (0)

#define EXPECT_GE(a, b, con, fmt, ...)                                                             \
	do {                                                                                       \
		if ((a) < (b))                                                                     \
			console_printf(con, "FAIL (%s:%d): " fmt, __FILE__, __LINE__,              \
				       ##__VA_ARGS__);                                             \
	} while (0)

static int _check_timing(console_t con, struct fb *self)
{
	usec_t us;
	usec_diff_t diff;
	// check that micros and timestamp return the same value
	us = micros();
	thread_sleep_ms(10);
	diff = micros() - us;
	EXPECT_LE(con, diff, 11000, "expected thread_sleep_ms(10) to sleep for ~10000us");
	EXPECT_GE(con, diff, 9000, "expected thread_sleep_ms(10) to sleep for ~10000us");
}

int fb_selftest_run(struct fb *self)
{
	int r = 0;
	r |= _check_timing(self->console, self);
	return r;
}
