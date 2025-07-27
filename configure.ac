AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])
AC_INIT([uidresp],[0.1],[crazybrake@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
LT_INIT
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.cpp])

# GTest detection via pkg-config
PKG_CHECK_MODULES([GTEST], [gtest >= 1.10], [],
  [AC_MSG_ERROR([Google Test not found via pkg-config])])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
])
AC_OUTPUT

