bin_PROGRAMS = rocket

rocket_SOURCES=main.c devicetree.c
rocket_LDFLAGS=-Wl,--no-undefined -Wl,-Map,rocket.map
rocket_LDADD=$(LDADD) $(WITH_LIBS) \
			 $(top_builddir)/src/libdriver/libdriver.a\
			 $(top_builddir)/src/libfirmware/libfirmware.a\
			 $(top_builddir)/src/libfdt/libfdt.a\
			 $(top_builddir)/src/libplc/libplc.a\
			 -lm

if WITH_TARGET_LINUX
rocket_LDADD+=-lrt -lpthread
rocket_LDFLAGS+=-Wl,-u,linux_uart_ko,-u,linux_leds_ko
else
rocket_LDFLAGS+=-Wl,-u,stm32_uart_ko,-u,stm32_leds_ko,-u,stm32_gpio_ko,-u,stm32_cpu_ko
endif

if WITH_TARGET_STM32F429ZET6
rocket_SOURCES+=target/stm32f4xx/startup_stm32f429_439xx.s
rocket_LDFLAGS+=-Wl,-T$(srcdir)/target/stm32f4xx/stm32f429zet6.ld
endif

if WITH_TARGET_STM32F40_41XXX
rocket_SOURCES+=target/stm32f4xx/startup_stm32f40_41xxx.s
rocket_LDFLAGS+=-T$(srcdir)/target/stm32f4xx/stm32f413_320K_1M.ld
endif

if WITH_TARGET_STM32F10X
rocket_SOURCES+=target/stm32f10x/startup_stm32f10x.s
rocket_LDFLAGS+=-T$(srcdir)/target/stm32f10x/stm32f10x_128k.ld
endif

rocket_CFLAGS=-I$(top_srcdir)/include
rocket_LDFLAGS+=$(WITH_DRIVERS)
rocket_LDFLAGS+=-L$(top_builddir)/src/libdriver
rocket_LDFLAGS+=-L$(top_builddir)/src/libfirmware
rocket_LDFLAGS+=-L$(top_builddir)/src/libfdt
rocket_LDFLAGS+=-L$(top_builddir)/src/libplc

devicetree.c: $(WITH_DEVICETREE) 
	@echo "DTB: $(1)"
	$(Q)$(CPP) $(CFLAGS) $(rocket_CFLAGS) -nostdinc -undef -x assembler-with-cpp $^ > $@.dtspp && dtc -O dtb $@.dtspp -o $@.dtb
	$(Q)echo "unsigned char _devicetree[] = { $$(cat $@.dtb | xxd -i) };" > $@

.PHONY: rocket

rocket_SOURCES += \
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
	rocket.c\
	measured.c\
	syscalls.c\
	ui.c

SUBDIRS = libfdt libfirmware libdriver libplc
AUTOMAKE_OPTIONS = subdir-objects

