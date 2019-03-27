define target/flyingbergman/configure
    $(eval $(call target/stm32f429_439xx/configure))
    TARGET_CFLAGS+=-DSTM32F429_439xx -DHSE_VALUE=8000000
endef

$(eval $(call DefineTarget,flyingbergman))


