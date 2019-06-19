AUTOTEST = $(AUTOM4TE) --language=autotest

TESTS = \
	motion_profile\
	fb\
	fb_control\
	fb_state_operational
check_PROGRAMS=$(TESTS)

AM_CFLAGS=\
	$(COVERAGE_FLAGS)\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include
AM_CXXFLAGS=\
	$(COVERAGE_FLAGS)\
	-include gtest/gtest.h -include gmock/gmock.h\
	-ffunction-sections\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include\
	-L$(abs_top_builddir)/lib
AM_LDFLAGS=\
	-Wl,--gc-sections -lm -lgtest -lgmock -lpthread

fb_SOURCES=\
	../src/fb_filter.c\
	../src/fb_control.c\
	../src/fb_leds.c\
	../src/measured.c\
	../src/fb_output.c\
	../src/fb_can.c\
	../src/fb_config.c\
	../src/fb_preset.c\
	../src/fb_inputs.c\
	../src/motion_profile.c\
	../src/fb_state_operational.c\
	fb.cpp

fb_control_SOURCES=\
	../src/fb_control.c\
	../src/fb_filter.c\
	fb_control.cpp

fb_state_operational_SOURCES=\
	../src/fb_state_operational.c\
	../src/fb_preset.c\
	fb_state_operational.cpp

motion_profile_SOURCES=\
	../src/motion_profile.c\
	motion_profile.cpp

AUTOMAKE_OPTIONS=subdir-objects
