AUTOTEST = $(AUTOM4TE) --language=autotest

TESTS = \
	motion-profile
check_PROGRAMS=$(TESTS)

AM_CFLAGS=\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include\
	-I$(abs_srcdir)/../../theboss/src/libfirmware/include
AM_CXXFLAGS=\
	-include gtest/gtest.h -include gmock/gmock.h\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include\
	-I$(abs_srcdir)/../../theboss/src/libfirmware/include
AM_LDFLAGS=\
	-lm -lgtest -lgmock -lpthread

motion_profile_SOURCES=\
	../src/motion_profile.c\
	../../theboss/src/libfirmware/src/timestamp.c\
	motion-profile.cpp

AUTOMAKE_OPTIONS=subdir-objects
