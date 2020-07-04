bin_PROGRAMS = rtpython

rtpython_SOURCES=main.c devicetree.c
rtpython_LDFLAGS=-Wl,--no-undefined -Wl,-Map,rtpython.map
rtpython_LDADD=$(LDADD) $(WITH_LIBS) \
			 $(top_builddir)/src/driver/libdriver.a\
			 $(top_builddir)/src/system/libfirmware.a\
			 $(top_builddir)/src/py/libpy.a\
			 -lm

if WITH_TARGET_LINUX
rtpython_LDADD+=-lrt -lpthread
rtpython_LDFLAGS+=-Wl,-u,linux_uart_ko,-u,linux_leds_ko
else
rtpython_LDFLAGS+=-Wl,-u,stm32_uart_ko,-u,stm32_leds_ko,-u,stm32_gpio_ko,-u,stm32_cpu_ko
endif

if WITH_TARGET_STM32F429ZET6
rtpython_SOURCES+=target/stm32f4xx/startup_stm32f429_439xx.s
rtpython_LDFLAGS+=-Wl,-T$(srcdir)/target/stm32f4xx/stm32f429zet6.ld
endif

if WITH_TARGET_STM32F40_41XXX
rtpython_SOURCES+=target/stm32f4xx/startup_stm32f40_41xxx.s
rtpython_LDFLAGS+=-T$(srcdir)/target/stm32f4xx/stm32f413_320K_1M.ld
endif

if WITH_TARGET_STM32F10X
rtpython_SOURCES+=target/stm32f10x/startup_stm32f10x.s
rtpython_LDFLAGS+=-T$(srcdir)/target/stm32f10x/stm32f10x_128k.ld
endif

rtpython_CFLAGS=-I$(top_srcdir)/include
rtpython_LDFLAGS+=$(WITH_DRIVERS)
rtpython_LDFLAGS+=-L$(top_builddir)/src/driver
rtpython_LDFLAGS+=-L$(top_builddir)/src/system

devicetree.c: $(WITH_DEVICETREE) 
	@echo "DTB: $(1)"
	$(Q)$(CPP) $(CFLAGS) $(rtpython_CFLAGS) -nostdinc -undef -x assembler-with-cpp $^ > $@.dtspp && dtc -O dtb $@.dtspp -o $@.dtb
	$(Q)echo "unsigned char _devicetree[] = { $$(cat $@.dtb | xxd -i) };" > $@

.PHONY: rtpython

rtpython_SOURCES += \
	modfb.c\
	fb.c\
	fb_can.c\
	fb_cmd.c\
	fb_config.c\
	fb_control.c\
	fb_filter.c\
	fb_leds.c\
	fb_output.c\
	fb_preset.c\
	fb_state_operational.c\
	fb_inputs.c\
	rtpython.c\
	measured.c\
	syscalls.c\
	ui.c

SUBDIRS = system driver py
AUTOMAKE_OPTIONS = subdir-objects

