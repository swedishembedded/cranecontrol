bin_PROGRAMS = cranecontrol

cranecontrol_SOURCES=main.c devicetree.c
cranecontrol_LDFLAGS=-Wl,--no-undefined -Wl,-Map,cranecontrol.map
cranecontrol_LDADD=$(LDADD) $(WITH_LIBS) \
			 $(top_builddir)/src/driver/libdriver.a\
			 $(top_builddir)/src/system/libfirmware.a\
			 -lm

#cranecontrol_LDADD+=$(top_builddir)/src/py/libpy.a

if WITH_TARGET_LINUX
cranecontrol_LDADD+=-lrt -lpthread
cranecontrol_LDFLAGS+=-Wl,-u,linux_uart_ko,-u,linux_leds_ko
else
cranecontrol_LDFLAGS+=-Wl,-u,stm32_uart_ko,-u,stm32_leds_ko,-u,stm32_gpio_ko,-u,stm32_cpu_ko
endif

if WITH_TARGET_STM32F429ZET6
cranecontrol_SOURCES+=target/stm32f4xx/startup_stm32f429_439xx.s
cranecontrol_LDFLAGS+=-Wl,-T$(srcdir)/target/stm32f4xx/stm32f429zet6.ld
endif

if WITH_TARGET_STM32F407VGT6
cranecontrol_SOURCES+=target/stm32f4xx/startup_stm32f40_41xxx.s
cranecontrol_LDFLAGS+=-T$(srcdir)/target/stm32f4xx/stm32f407vgt6.ld
endif

if WITH_TARGET_STM32F10X
cranecontrol_SOURCES+=target/stm32f10x/startup_stm32f10x.s
cranecontrol_LDFLAGS+=-T$(srcdir)/target/stm32f10x/stm32f10x_128k.ld
endif

cranecontrol_CFLAGS=-I$(top_srcdir)/include
cranecontrol_LDFLAGS+=$(WITH_DRIVERS)
cranecontrol_LDFLAGS+=-L$(top_builddir)/src/driver
cranecontrol_LDFLAGS+=-L$(top_builddir)/src/system

devicetree.c: $(WITH_DEVICETREE) 
	@echo "DTB: $(1)"
	$(Q)$(CPP) $(CFLAGS) $(cranecontrol_CFLAGS) -nostdinc -undef -x assembler-with-cpp $^ > $@.dtspp && dtc -O dtb $@.dtspp -o $@.dtb
	$(Q)echo "unsigned char _devicetree[] = { $$(cat $@.dtb | xxd -i) };" > $@

.PHONY: cranecontrol

cranecontrol_SOURCES += \
	blinky.c\
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
	measured.c\
	syscalls.c\
	ui.c

#rust.c\
#rust/rust.rs

#cranecontrol_SOURCES += python.c modfb.c

SUFFIXES = .rs

#.rs.o:
#	rustc -C opt-level=2 \
#		--target thumbv7em-none-eabihf\
#		-g --emit obj \
#		-L /home/martin/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/thumbv7m-none-eabihf/lib/ -o $@ $<
#

SUBDIRS = system driver
AUTOMAKE_OPTIONS = subdir-objects

