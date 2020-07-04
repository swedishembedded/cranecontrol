SUBDIRS = include src tests
AM_LDFLAGS = -shared -O3
AUTOMAKE_OPTIONS = subdir-objects
include Makefile.am.coverage

flash:
	arm-none-eabi-size $(top_builddir)/src/rtpython
	arm-none-eabi-objcopy -O binary $(top_builddir)/src/rtpython $(top_builddir)/src/rtpython.hex
	$(RTPYTHON_TARGET_CMD_FLASH)
