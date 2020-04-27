SUBDIRS = include src
AM_LDFLAGS = -shared -O3
AUTOMAKE_OPTIONS = subdir-objects

flash:
	arm-none-eabi-size $(top_builddir)/src/flyingbergman
	arm-none-eabi-objcopy -O binary $(top_builddir)/src/flyingbergman $(top_builddir)/src/flyingbergman.hex
	$(ROCKET_TARGET_CMD_FLASH)
