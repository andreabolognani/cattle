AC_INIT([Cattle], [1.4.0],
        [eof@kiyuko.org], [], [https://kiyuko.org/software/cattle])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])

dnl *****************************
dnl *** Reject in-tree builds ***
dnl *****************************

if test "$srcdir" = "."
then
    AC_MSG_ERROR([Build directory must be different from source directory])
fi

AM_INIT_AUTOMAKE([foreign tar-pax dist-xz no-dist-gzip])
AM_SILENT_RULES([yes])

dnl ***********************
dnl *** Release version ***
dnl ***********************

CATTLE_MAJOR_VERSION=1
CATTLE_MINOR_VERSION=4
CATTLE_MICRO_VERSION=0
CATTLE_VERSION=$CATTLE_MAJOR_VERSION.$CATTLE_MINOR_VERSION.$CATTLE_MICRO_VERSION

AC_SUBST([CATTLE_MAJOR_VERSION])
AC_SUBST([CATTLE_MINOR_VERSION])
AC_SUBST([CATTLE_MICRO_VERSION])
AC_SUBST([CATTLE_VERSION])

dnl ***********************
dnl *** Libtool version ***
dnl ***********************

dnl Increment on interface add, remove, or change.
CATTLE_CURRENT=1

dnl Increment on source change. Reset when CURRENT changes.
CATTLE_REVISION=0

dnl Increment on interface addition. Reset on removal.
CATTLE_AGE=1

AC_SUBST([CATTLE_CURRENT])
AC_SUBST([CATTLE_REVISION])
AC_SUBST([CATTLE_AGE])

dnl ***********************
dnl *** Check for tools ***
dnl ***********************

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PATH_PROGS([GLIB_MKENUMS], [glib-mkenums])
if test -z "$GLIB_MKENUMS"
then
    AC_MSG_ERROR([glib-mkenums not found])
fi

dnl **********************
dnl *** Check for GLib ***
dnl **********************

GLIB_REQUIRED_VERSION=2.38.0

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

dnl ***************************
dnl *** Create output files ***
dnl ***************************

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
