AC_PREREQ([2.61])
AC_INIT([libflyingbergman], [0.1], [mkschreder.uk@gmail.com])
AC_CONFIG_AUX_DIR(aux-dist)
AM_INIT_AUTOMAKE

AC_PROG_MAKE_SET
AC_LANG(C)
AC_PROG_CXX

AC_ARG_ENABLE(coverage, ["Enable coverage"], [AX_COVERAGE], [])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_TESTDIR([tests])

LT_PREREQ([2.2])
LT_INIT([dlopen shared])

AC_CONFIG_FILES([src/Makefile include/Makefile Makefile])
AC_CONFIG_FILES([tests/Makefile])

AC_OUTPUT


