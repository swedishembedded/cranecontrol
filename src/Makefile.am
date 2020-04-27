bin_PROGRAMS = flyingbergman

flyingbergman_SOURCES=fault.c main.c devicetree.c
flyingbergman_LDFLAGS=-Wl,--no-undefined -Wl,-Map,rocket.map
flyingbergman_LDADD=$(LDADD) $(WITH_LIBS) -ldriver -lfirmware -lfdt -lm

if WITH_TARGET_LINUX
flyingbergman_LDADD+=-lrt -lpthread
flyingbergman_LDFLAGS+=-Wl,-u,linux_uart_ko,-u,linux_leds_ko
else
flyingbergman_LDFLAGS+=-Wl,-u,stm32_uart_ko,-u,stm32_leds_ko,-u,stm32_gpio_ko,-u,stm32_cpu_ko
endif

if WITH_TARGET_STM32F429ZET6
flyingbergman_SOURCES+=target/stm32f4xx/startup_stm32f429_439xx.s
flyingbergman_LDFLAGS+=-Wl,-T$(srcdir)/target/stm32f4xx/stm32f429zet6.ld
endif

if WITH_TARGET_STM32F40_41XXX
flyingbergman_SOURCES+=target/stm32f4xx/startup_stm32f40_41xxx.s
flyingbergman_LDFLAGS+=-T$(srcdir)/target/stm32f4xx/stm32f413_320K_1M.ld
endif

if WITH_TARGET_STM32F10X
flyingbergman_SOURCES+=target/stm32f10x/startup_stm32f10x.s
flyingbergman_LDFLAGS+=-T$(srcdir)/target/stm32f10x/stm32f10x_128k.ld
endif

flyingbergman_CFLAGS=-I$(top_srcdir)/include
flyingbergman_LDFLAGS+=$(WITH_DRIVERS)
flyingbergman_LDFLAGS+=-L$(top_builddir)/src/libdriver
flyingbergman_LDFLAGS+=-L$(top_builddir)/src/libfirmware
flyingbergman_LDFLAGS+=-L$(top_builddir)/src/libfdt

devicetree.c: $(WITH_DEVICETREE) 
	@echo "DTB: $(1)"
	$(Q)$(CPP) $(CFLAGS) $(flyingbergman_CFLAGS) -nostdinc -undef -x assembler-with-cpp $^ > $@.dtspp && dtc -O dtb $@.dtspp -o $@.dtb
	$(Q)echo "unsigned char _devicetree[] = { $$(cat $@.dtb | xxd -i) };" > $@

.PHONY: rocket

flyingbergman_SOURCES += \
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
	flyingbergman.c\
	measured.c\
	motion_profile.c\
	ui.c

SUBDIRS = libfdt libfirmware libdriver
AUTOMAKE_OPTIONS = subdir-objects

