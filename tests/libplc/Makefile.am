AUTOTEST = $(AUTOM4TE) --language=autotest

TESTS = \
	motion_profile
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
	-L$(abs_top_builddir)/lib\
	-L/usr/local/lib
AM_LDFLAGS=\
	-Wl,--gc-sections -lm -lgtest -lgmock -lpthread 

motion_profile_SOURCES=\
	../src/motion_profile.c\
	motion_profile.cpp

AUTOMAKE_OPTIONS=subdir-objects
