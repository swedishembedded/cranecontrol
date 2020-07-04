AUTOTEST = $(AUTOM4TE) --language=autotest

TESTS = \
	check_timestamp\
	check_cbuf\
	check_regmap\
	check_modbus\
	check_linux_uart\
	check_fdt_rw\
	check_fdt_sw\
	check_motion_profile
check_PROGRAMS=$(TESTS)

AM_CFLAGS=\
	$(COVERAGE_CFLAGS)\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include\
	-I$(srcdir)/../include/libfirmware/\
	-I$(srcdir)/../include/libfdt
AM_CXXFLAGS=\
	$(COVERAGE_CXXFLAGS)\
	-include gtest/gtest.h -include gmock/gmock.h\
	-ffunction-sections\
	-I$(abs_top_builddir)/include\
	-I$(srcdir)/../include/libfirmware/\
	-I$(srcdir)/../include\
	-L$(abs_top_builddir)/lib\
	-L/usr/local/lib
AM_LDFLAGS=\
	$(COVERAGE_LDFLAGS)\
	-Wl,--gc-sections -lm -lgtest -lgmock -lpthread
FDT_SRC = \
	../src/system/libfdt/fdt.c\
	../src/system/libfdt/fdt_ro.c\
	../src/system/libfdt/fdt_rw.c\
	../src/system/libfdt/fdt_strerror.c\
	../src/system/libfdt/fdt_wip.c\
	../src/system/libfdt/fdt_sw.c\
	../src/system/libfdt/libfdt_env.c

LINUX_THREAD_SRC = \
	../src/system/linux/thread.c\
	../src/system/linux/thread_mutex.c\
	../src/system/linux/thread_queue.c\
	../src/system/linux/thread_sem.c\
	../src/system/linux/thread_work.c\
	../src/system/linux/time.c

check_timestamp_SOURCES = \
	../src/system/timestamp.c\
	system/timestamp.cpp

check_fdt_sw_SOURCES = \
	$(FDT_SRC)\
	system/fdt/fdt-sw.cpp

check_linux_uart_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/system/driver.c\
	../src/system/class.c\
	../src/system/serial.c\
	../src/system/linux/linux_uart.c\
	../src/system/linux/linux_serial.c\
	system/linux/linux_uart.cpp

check_modbus_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/modbus.c\
	../src/system/regmap.c\
	../src/system/driver.c\
	../src/system/class.c\
	../src/system/serial.c\
	../src/system/linux/linux_uart.c\
	../src/system/linux/linux_serial.c\
	mock/crc16.cpp\
	system/modbus.cpp

check_fdt_rw_SOURCES = \
	$(FDT_SRC)\
	system/fdt/fdt-rw.cpp

check_regmap_SOURCES = \
	$(FDT_SRC)\
	$(LINUX_THREAD_SRC)\
	../src/system/regmap.c\
	../src/system/driver.c\
	system/regmap.cpp

check_cbuf_SOURCES = \
	../src/system/cbuf.c\
	system/cbuf.cpp

check_motion_profile_SOURCES=\
	../src/motion_profile.c\
	motion_profile.cpp

AUTOMAKE_OPTIONS=subdir-objects
