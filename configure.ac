AC_PREREQ([2.61])
AC_INIT([rocket], [0.1], [mkschreder.uk@gmail.com])
AC_CONFIG_AUX_DIR(aux-dist)
AC_ARG_WITH([devicetree],
    [AC_HELP_STRING([--with-devicetree],
        [Specify device tree source])],
    [],
    [with_devicetree=$srcdir/dts/linux.dts])

AC_SUBST([WITH_DEVICETREE], ["$with_devicetree"])

AC_ARG_WITH([drivers],
    [AC_HELP_STRING([--with-drivers],
        [Specify drivers to include from libraries])],
    [],
    [with_drivers=])

AC_SUBST([WITH_DRIVERS], ["$with_drivers"])

AC_ARG_WITH([libs],
    [AC_HELP_STRING([--with-libs],
        [Specify extra libraries to link])],
    [],
    [])

AC_SUBST([WITH_LIBS], ["$with_libs"])

AC_ARG_WITH([target],
    [AC_HELP_STRING([--with-target],
        [Specify which chip we are compiling for])],
    [],
    [with_target=linux])

AM_INIT_AUTOMAKE

CFLAGS="$CFLAGS -Wall -Wextra -Werror\
	-g \
	-O3 \
	-ffunction-sections -fdata-sections\
	-fsingle-precision-constant\
	-std=gnu11\
	-pedantic \
	-Wno-strict-overflow\
	-Wno-unused-parameter\
	-Wformat-nonliteral\
	-Wformat-security\
	-Wfloat-equal\
	-Wdouble-promotion\
	-Wshadow\
	-Wwrite-strings\
	-Wconversion\
	-Wredundant-decls\
	-Wunreachable-code\
	-Winline"

get_param(){
    echo $(grep "\!+$2: " "$1" | sed 's/\/\/!+'${2}': \(.*\)/\1/g')
}

ROCKET_OUTPUT_FILE=src/flyingbergman.hex
ROCKET_TARGET_DEPENDS=$(get_param ${WITH_DEVICETREE} "DEPENDS")
ROCKET_TARGET_CFLAGS=$(get_param ${WITH_DEVICETREE} "CFLAGS")
ROCKET_TARGET_LDFLAGS=$(get_param ${WITH_DEVICETREE} "LDLAGS")
ROCKET_TARGET_LDADD=$(get_param ${WITH_DEVICETREE} "LDADD")
ROCKET_TARGET_HOST=$(get_param ${WITH_DEVICETREE} "HOST")
ROCKET_TARGET_TARGET=$(get_param ${WITH_DEVICETREE} "TARGET")
ROCKET_TARGET_CMD_FLASH=$(get_param ${WITH_DEVICETREE} "CMD_FLASH")
ROCKET_TARGET_DRIVERS_LDADD="-Wl,"$(grep compatible ${WITH_DEVICETREE} | sed 's+.*".*,\(.*\)";+-u,\1_ko+g' | sort | uniq | paste -sd "," -)

ROCKET_TARGET_CMD_FLASH=$(echo ${ROCKET_TARGET_CMD_FLASH} | sed "s+##FILE##+${ROCKET_OUTPUT_FILE}+g")
AC_SUBST(ROCKET_TARGET_CMD_FLASH)

CFLAGS="$CFLAGS $ROCKET_TARGET_CFLAGS"

AC_SUBST(CFLAGS)

LDADD="$LDADD $ROCKET_TARGET_DRIVERS_LDADD"
AC_SUBST(LDADD)

AC_PROG_MAKE_SET
AC_PROG_CC
AM_PROG_AS
AC_LANG(C,ASM)

CXXFLAGS="-std=c++11 -fPIC"

AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2])
LT_INIT([dlopen shared])

AC_SUBST([WITH_TARGET], ["$with_target"])
AM_CONDITIONAL([WITH_TARGET_LINUX], [test "x$with_target" = xlinux])

AM_CONDITIONAL([WITH_TARGET_STM32F429ZET6], [test "x$with_target" = xstm32f429zet6])
AM_CONDITIONAL([WITH_TARGET_STM32F429_439XX], [test "x$with_target" = xstm32f429_439xx])
AM_CONDITIONAL([WITH_TARGET_STM32F429_439XX], [test "x$with_target" = xstm32f429zet6])
AM_COND_IF([WITH_TARGET_STM32F429ZET6], (AC_DEFINE([WITH_TARGET_STM32F429ZET6], [1], [Build for stm32f429zet6 mcu])))
AM_COND_IF([WITH_TARGET_STM32F429ZET6], (AC_DEFINE([WITH_TARGET_STM32F429_439XX], [1], [Build for stm32f429zet6 mcu])))
AM_COND_IF([WITH_TARGET_STM32F429_439XX], (AC_DEFINE([WITH_TARGET_STM32F429_439XX], [1], [Build for stm32f429_439xx processor])))

AM_CONDITIONAL([WITH_TARGET_STM32F427], [test x$with_target = xstm32f427])
AM_COND_IF([WITH_TARGET_STM32F427], (AC_DEFINE([WITH_TARGET_STM32F427], [1], [Build for stm32f427 processor])))

AM_CONDITIONAL([WITH_TARGET_STM32F40_41XXX], [test "x$with_target" = xstm32f40_41xxx])
AM_COND_IF([WITH_TARGET_STM32F40_41XXX], (AC_DEFINE([WITH_TARGET_STM32F40_41XXX], [1], [Build for stm32f40_41xxx mcu])))

AM_CONDITIONAL([WITH_TARGET_STM32F10X], [test "x$with_target" = xstm32f10x])
AM_COND_IF([WITH_TARGET_STM32F10X], (AC_DEFINE([WITH_TARGET_STM32F10X], [1], [Build for stm32f10x mcu])))

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile])
AC_CONFIG_FILES([src/libfdt/Makefile])
AC_CONFIG_FILES([src/libdriver/Makefile])
AC_CONFIG_FILES([src/libfirmware/Makefile])

AC_OUTPUT

echo "Configuration Parameters:"
echo "target............... ${with_target}"
echo "Device Tree.......... ${with_devicetree}"
echo "Flash Command........ ${ROCKET_TARGET_CMD_FLASH}"
echo "AR_FLAGS............. ${AR_FLAGS}"
echo "CC................... ${CC}"
echo "CFLAGS............... ${CFLAGS}"
echo "CPP.................. ${CPP}"
echo "CPPFLAGS............. ${CPPFLAGS}"
echo "CXX.................. ${CXX}"
echo "CXXFLAGS............. ${CXXFLAGS}"
echo "HOST................. ${HOST}"
echo "HOSTNAME............. ${HOSTNAME}"
echo "LDFLAGS.............. ${LDFLAGS}"
echo "LDADD................ ${LDADD}"
echo "host................. ${host}"
echo "install prefix ...... ${prefix}"

