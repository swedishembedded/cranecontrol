define package/flyingbergman/configure
	PKG_SOURCE_DIR:=$(PROJECT_TOP_DIR)/src/flyingbergman
endef

$(eval $(call DefinePackage,flyingbergman))

define package/firmware/configure
	PKG_SOURCE_DIR:=$(SRC_TOP_DIR)/bossinit
	PKG_VERSION:=1.0
	PKG_DEPENDS:=libfirmware libdriver flyingbergman
endef

define package/firmware/flyingbergman/configure
	PKG_CONFIGURE_OPTIONS:=\
		--with-target=stm32f429zet6\
		--with-devicetree=$(PWD)/dts/flyingbergman.dts\
		--with-drivers="-Wl,-u,flyingbergman_ko,-u,stm32_spi_ko,-u,console_ko,-u,mcp2317_ko,-u,stm32_cpuinfo_ko,-u,stm32_tim_ko,-u,drv8302_ko,-u,stm32_i2c_ko,-u,mcp4461_ko,-u,stm32_adc_ko,-u,lp55231_ko"\
		--with-libs=-lflyingbergman
endef


$(eval $(call DefinePackage,firmware))

package/firmware/flyingbergman/flash: package/firmware/flyingbergman/compile
	$(TARGET_TOOL_PREFIX)objcopy -O binary $(BUILD_DIR)/$(TARGET)/firmware/src/theboss $(BUILD_DIR)/$(TARGET)/firmware/theboss.bin
	$(TARGET_TOOL_PREFIX)size $(BUILD_DIR)/$(TARGET)/firmware/src/theboss
	stm32flash -w $(BUILD_DIR)/$(TARGET)/firmware/theboss.bin -b 115200 -v -g 0x0 /dev/ttyUSB0

package/firmware/flyingbergman/flash-stlink: package/firmware/flyingbergman/compile
	$(TARGET_TOOL_PREFIX)objcopy -O binary $(BUILD_DIR)/$(TARGET)/firmware/src/theboss $(BUILD_DIR)/$(TARGET)/firmware/theboss.bin
	$(TARGET_TOOL_PREFIX)size $(BUILD_DIR)/$(TARGET)/firmware/src/theboss
	#openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program $(BUILD_DIR)/$(TARGET)/firmware/src/theboss verify reset";
	st-flash --reset write $(BUILD_DIR)/$(TARGET)/firmware/theboss.bin 0x8000000


