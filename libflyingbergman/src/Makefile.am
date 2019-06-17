lib_LIBRARIES = libflyingbergman.a
libflyingbergman_a_SOURCES = \
	fb.c\
	fb_can.c\
	fb_cmd.c\
	fb_config.c\
	fb_control.c\
	fb_filter.c\
	fb_leds.c\
	fb_preset.c\
	fb_state_operational.c\
	inputs.c\
	measured.c\
	motion_profile.c\
	ui.c

libflyingbergman_a_CFLAGS = \
	$(COVERAGE_FLAGS)\
	-I$(top_srcdir)/include\
	-I$(abs_top_builddir)/include/
AC_LOCAL_AMFLAGS=-I m4
