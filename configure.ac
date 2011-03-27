AC_INIT([Cattle], [1.0.1], [eof@kiyuko.org], [cattle])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl ***********************
dnl *** Release version ***
dnl ***********************

CATTLE_MAJOR_VERSION=1
CATTLE_MINOR_VERSION=0
CATTLE_MICRO_VERSION=1
CATTLE_VERSION=$CATTLE_MAJOR_VERSION.$CATTLE_MINOR_VERSION.$CATTLE_MICRO_VERSION

AC_SUBST(CATTLE_MAJOR_VERSION)
AC_SUBST(CATTLE_MINOR_VERSION)
AC_SUBST(CATTLE_MICRO_VERSION)
AC_SUBST(CATTLE_VERSION)

dnl ***********************
dnl *** Libtool version ***
dnl ***********************

dnl Increment on interface add, remove, or change.
CATTLE_CURRENT=0

dnl Increment on source change. Reset when CURRENT changes.
CATTLE_REVISION=1

dnl Increment on interface addition. Reset on removal.
CATTLE_AGE=0

AC_SUBST(CATTLE_CURRENT)
AC_SUBST(CATTLE_REVISION)
AC_SUBST(CATTLE_AGE)

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_MAKE_SET

AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl ********************************************
dnl *** Check for header files and functions ***
dnl ********************************************

AC_HEADER_STDC

dnl ***************************************
dnl *** Check for required GLib modules ***
dnl ***************************************

GLIB_REQUIRED_VERSION=2.16.0
GLIB_REQUIRED_MODULES="gobject"

AM_PATH_GLIB_2_0($GLIB_REQUIRED_VERSION,,
                 AC_MSG_ERROR([GLib version $GLIB_REQUIRED_VERSION required]),
                 $GLIB_REQUIRED_MODULES)

# GIO is only required for the example programs
PKG_CHECK_MODULES(GIO, gio-2.0 >= $GLIB_REQUIRED_VERSION)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)

dnl ***************************************
dnl *** Check for GObject Introspection ***
dnl ***************************************

GOBJECT_INTROSPECTION_CHECK([0.6.7])

dnl *************************
dnl *** Check for GTK-Doc ***
dnl *************************

GTK_DOC_CHECK([1.15])

dnl *******************************
dnl *** Create the output files ***
dnl *******************************

AC_CONFIG_FILES([Makefile
                 cattle-1.0.pc
                 cattle/Makefile
                 cattle/cattle-version.h
                 docs/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 examples/Makefile
                 tests/Makefile])

AC_OUTPUT
