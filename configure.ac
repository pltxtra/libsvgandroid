AC_PREREQ([2.69])

AC_INIT([libsvgandroid], [1.0], [pltxtra@vuknob.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_PROG_AR
AC_CONFIG_MACRO_DIRS([m4])
PKG_INSTALLDIR
PKG_PROG_PKG_CONFIG

AC_PROG_CXX
AC_PROG_CC

LT_INIT

PKG_CHECK_MODULES([expat], [expat])
PKG_CHECK_MODULES([libjpeg], [libjpeg])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 libsvgandroid.pc
 libsvg/Makefile
])

AC_OUTPUT
