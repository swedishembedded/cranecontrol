AC_PREREQ([2.61])
AC_INIT([cranecontrol], [0.1], [mkschreder.uk@gmail.com])
AC_CONFIG_AUX_DIR(aux-dist)
AM_INIT_AUTOMAKE

AC_ARG_WITH([devicetree],
    [AC_HELP_STRING([--with-devicetree],
        [Specify device tree source])],
	[], [with_devicetree=$srcdir/dts/linux.dts])

AC_SUBST([WITH_DEVICETREE], ["$with_devicetree"])

CFLAGS="$CFLAGS -Wall -Wextra -Werror\
	-g \
	-O3 \
	-ffunction-sections -fdata-sections\
	-fsingle-precision-constant\
	-std=gnu11\
	-Wno-strict-overflow\
	-Wno-unused-parameter\
	-Wformat-nonliteral\
	-Wformat-security\
	-Wfloat-equal\
	-Wdouble-promotion\
	-Wshadow\
	-Wwrite-strings\
	-Wunreachable-code\
"

#-Wredundant-decls\
#-pedantic
#-Wconversion
#-Winline\

get_param(){
    echo $(grep "\!+$2: " "$1" | sed 's/\/\/!+'${2}': \(.*\)/\1/g')
}

CRANECONTROL_OUTPUT_FILE=src/rtpython.hex
CRANECONTROL_TARGET_DEPENDS=$(get_param ${WITH_DEVICETREE} "DEPENDS")
CRANECONTROL_TARGET_CFLAGS=$(get_param ${WITH_DEVICETREE} "CFLAGS")
CRANECONTROL_TARGET_LDFLAGS=$(get_param ${WITH_DEVICETREE} "LDLAGS")
CRANECONTROL_TARGET_LDADD=$(get_param ${WITH_DEVICETREE} "LDADD")
CRANECONTROL_TARGET_HOST=$(get_param ${WITH_DEVICETREE} "HOST")
CRANECONTROL_TARGET=$(get_param ${WITH_DEVICETREE} "TARGET")
CRANECONTROL_TARGET_CMD_FLASH=$(get_param ${WITH_DEVICETREE} "CMD_FLASH")
CRANECONTROL_TARGET_DRIVERS_LDADD="-Wl,"$(grep compatible ${WITH_DEVICETREE} | sed 's+.*".*,\(.*\)";+-u,\1_ko+g' | sort | uniq | paste -sd "," -)

CRANECONTROL_TARGET_CMD_FLASH=$(echo ${CRANECONTROL_TARGET_CMD_FLASH} | sed "s+##HEX_FILE##+src/rtpython.hex+g")
CRANECONTROL_TARGET_CMD_FLASH=$(echo ${CRANECONTROL_TARGET_CMD_FLASH} | sed "s+##FILE##+src/rtpython+g")
AC_SUBST(CRANECONTROL_TARGET_CMD_FLASH)

CFLAGS="$CFLAGS $CRANECONTROL_TARGET_CFLAGS"

AC_SUBST(CFLAGS)

LDADD="$LDADD $CRANECONTROL_TARGET_DRIVERS_LDADD"
AC_SUBST(LDADD)

AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_LANG(C,ASM)

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [Build Rust code with debugging information [default=no]]),
              [debug_release=$enableval],
              [debug_release=no])

AC_MSG_CHECKING(whether to build Rust code with debugging information)
if test "x$debug_release" = "xyes" ; then
    AC_MSG_RESULT(yes)
    RUST_TARGET_SUBDIR=debug
else
    AC_MSG_RESULT(no)
    RUST_TARGET_SUBDIR=release
fi
AM_CONDITIONAL([DEBUG_RELEASE], [test "x$debug_release" = "xyes"])
AC_SUBST([RUST_TARGET_SUBDIR])

AX_COVERAGE

CXXFLAGS="-std=c++11 -fPIC"

AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2])
LT_INIT([dlopen shared])

AC_SUBST([WITH_TARGET], ["$CRANECONTROL_TARGET"])
AM_CONDITIONAL([WITH_TARGET_LINUX], [test "x$CRANECONTROL_TARGET" = xlinux])

AM_CONDITIONAL([WITH_TARGET_STM32F429ZET6], [test "x$CRANECONTROL_TARGET" = xstm32f429zet6])
AM_COND_IF([WITH_TARGET_STM32F429ZET6], (AC_DEFINE([WITH_TARGET_STM32F429ZET6], [1], [Build for stm32f429zet6 mcu])))

AM_CONDITIONAL([WITH_TARGET_STM32F429_439XX], [test "x$CRANECONTROL_TARGET" = xstm32f429_439xx])
AM_COND_IF([WITH_TARGET_STM32F429ZET6], (AC_DEFINE([WITH_TARGET_STM32F429_439XX], [1], [Build for stm32f429zet6 mcu])))

AM_CONDITIONAL([WITH_TARGET_STM32F427], [test x$CRANECONTROL_TARGET = xstm32f427])
AM_COND_IF([WITH_TARGET_STM32F427], (AC_DEFINE([WITH_TARGET_STM32F427], [1],
		   [Build for stm32f427 processor])))

AM_CONDITIONAL([WITH_TARGET_STM32F40_41XXX], [test "x$CRANECONTROL_TARGET" = xstm32f40_41xxx])
AM_COND_IF([WITH_TARGET_STM32F40_41XXX], (AC_DEFINE([WITH_TARGET_STM32F40_41XXX], [1], [Build for stm32f40_41xxx mcu])))

AM_CONDITIONAL([WITH_TARGET_STM32F407VGT6], [test "x$CRANECONTROL_TARGET" = xstm32f407vgt6])
AM_COND_IF([WITH_TARGET_STM32F407VGT6], (AC_DEFINE([WITH_TARGET_STM32F407VGT6], [1], [Build for stm32f407vgt6 mcu])))

AM_CONDITIONAL([WITH_TARGET_STM32F10X], [test "x$CRANECONTROL_TARGET" = xstm32f10x])
AM_COND_IF([WITH_TARGET_STM32F10X], (AC_DEFINE([WITH_TARGET_STM32F10X], [1], [Build for stm32f10x mcu])))

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile])
AC_CONFIG_FILES([src/driver/Makefile])
AC_CONFIG_FILES([src/py/Makefile])
AC_CONFIG_FILES([src/system/Makefile])
AC_CONFIG_FILES([tests/Makefile])

AC_OUTPUT

echo "Configuration Parameters:"
echo "target............... ${CRANECONTROL_TARGET}"
echo "Device Tree.......... ${with_devicetree}"
echo "Flash Command........ ${CRANECONTROL_TARGET_CMD_FLASH}"
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

