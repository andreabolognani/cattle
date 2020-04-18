AC_INIT([Cattle], [1.2.2], [eof@kiyuko.org], [cattle])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl ***********************
dnl *** Release version ***
dnl ***********************

CATTLE_MAJOR_VERSION=1
CATTLE_MINOR_VERSION=2
CATTLE_MICRO_VERSION=2
CATTLE_VERSION=$CATTLE_MAJOR_VERSION.$CATTLE_MINOR_VERSION.$CATTLE_MICRO_VERSION

AC_SUBST([CATTLE_MAJOR_VERSION])
AC_SUBST([CATTLE_MINOR_VERSION])
AC_SUBST([CATTLE_MICRO_VERSION])
AC_SUBST([CATTLE_VERSION])

dnl ***********************
dnl *** Libtool version ***
dnl ***********************

dnl Increment on interface add, remove, or change.
CATTLE_CURRENT=0

dnl Increment on source change. Reset when CURRENT changes.
CATTLE_REVISION=4

dnl Increment on interface addition. Reset on removal.
CATTLE_AGE=0

AC_SUBST([CATTLE_CURRENT])
AC_SUBST([CATTLE_REVISION])
AC_SUBST([CATTLE_AGE])

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_MAKE_SET

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_PATH_PROGS([GLIB_MKENUMS], [glib-mkenums])
if test -z "$GLIB_MKENUMS"
then
    AC_MSG_ERROR([glib-mkenums not found])
fi

dnl ********************************************
dnl *** Check for header files and functions ***
dnl ********************************************

AC_HEADER_STDC

dnl **********************
dnl *** Check for GLib ***
dnl **********************

GLIB_REQUIRED_VERSION=2.32.0

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= $GLIB_REQUIRED_VERSION
                           gobject-2.0 >= $GLIB_REQUIRED_VERSION
                           gio-2.0 >= $GLIB_REQUIRED_VERSION])
AC_SUBST([GLIB_CFLAGS])
AC_SUBST([GLIB_LIBS])

dnl ***************************************
dnl *** Check for GObject Introspection ***
dnl ***************************************

GOBJECT_INTROSPECTION_CHECK([1.32.0])

dnl *************************
dnl *** Check for GTK-Doc ***
dnl *************************

GTK_DOC_CHECK([1.18])

dnl ***********************************
dnl *** Enable compilation warnings ***
dnl ***********************************

WARN_CFLAGS="-Wall -Wextra"

AC_SUBST([WARN_CFLAGS])

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
