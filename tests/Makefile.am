# The `:;' works around a Bash 3.2 bug when the output is not writeable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
		echo '# Signature of the current package.' && \
		echo 'm4_define([AT_PACKAGE_NAME],' && \
		echo '	[$(PACKAGE_NAME)])' && \
		echo 'm4_define([AT_PACKAGE_TARNAME],' && \
		echo '	[$(PACKAGE_TARNAME)])' && \
		echo 'm4_define([AT_PACKAGE_VERSION],' && \
		echo '	[$(PACKAGE_VERSION)])' && \
		echo 'm4_define([AT_PACKAGE_STRING],' && \
		echo '	[$(PACKAGE_STRING)])' && \
		echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
		echo '	[$(PACKAGE_BUGREPORT)])'; \
		echo 'm4_define([AT_PACKAGE_URL],' && \
		echo '	[$(PACKAGE_URL)])'; \
	} >'$(srcdir)/package.m4'

EXTRA_DIST = testsuite.at atlocal.in $(srcdir)/package.m4 $(TESTSUITE) atlocal.in
TESTSUITE = $(srcdir)/testsuite

check-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
		$(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
	$(SHELL) '$(TESTSUITE)' --clean

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/testsuite.at $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

TESTS = \
	check-timestamp\
	check-cbuf\
	motion_profile\
	regmap\
	modbus\
	linux_uart\
	fdt-rw\
	check-fdt-sw

check_PROGRAMS = $(TESTS)

AM_CFLAGS = \
	-Wall -Werror \
	-I$(top_srcdir)/include\
	-I$(top_srcdir)/include/libfdt/\
	-I$(abs_top_builddir)/include\
	$(COVERAGE_CFLAGS)
AM_CXXFLAGS = \
	-Wall -Werror\
	-I$(top_srcdir)/include\
	-I$(top_srcdir)/include/libfdt/\
	-I$(abs_top_builddir)/include/\
	$(COVERAGE_CFLAGS)
AM_LDFLAGS =\
	-L$(top_builddir)/src/ \
	-L$(abs_top_builddir)/lib/ \
	-Wl,--start-group \
	$(COVERAGE_LDFLAGS) -lgtest -lgmock -lpthread -lrt

FDT_SRC = \
	../src/libfdt/fdt.c\
	../src/libfdt/fdt_ro.c\
	../src/libfdt/fdt_rw.c\
	../src/libfdt/fdt_strerror.c\
	../src/libfdt/fdt_wip.c\
	../src/libfdt/fdt_sw.c\
	../src/libfdt/libfdt_env.c

LINUX_THREAD_SRC = \
	../src/linux/thread.c\
	../src/linux/thread_mutex.c\
	../src/linux/thread_queue.c\
	../src/linux/thread_sem.c\
	../src/linux/thread_work.c\
	../src/linux/time.c

check_timestamp_SOURCES = \
	../src/timestamp.c\
	check-timestamp.cpp

check_fdt_sw_SOURCES = \
	$(FDT_SRC)\
	check-fdt-sw.cpp

linux_uart_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/driver.c\
	../src/class.c\
	../src/serial.c\
	../src/linux/linux_uart.c\
	../src/linux/linux_serial.c\
	linux_uart.cpp

modbus_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/bus/modbus.c\
	../src/regmap.c\
	../src/driver.c\
	../src/class.c\
	../src/serial.c\
	../src/linux/linux_uart.c\
	../src/linux/linux_serial.c\
	modbus.cpp

fdt_rw_SOURCES = \
	$(FDT_SRC)\
	fdt-rw.cpp

motion_profile_SOURCES = \
	../src/control/motion_profile.c\
	motion_profile.cpp

regmap_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/regmap.c\
	../src/driver.c\
	regmap.cpp

check_cbuf_SOURCES = \
	../src/cbuf.c\
	check-cbuf.cpp

AUTOMAKE_OPTIONS = subdir-objects
