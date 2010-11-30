INCLUDES = -I$(top_srcdir)   \
           -I$(top_builddir)

noinst_PROGRAMS = interpreter \
                  indent

interpreter_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la common.h
interpreter_SOURCES = interpreter.c common.c
interpreter_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
interpreter_LDFLAGS = @GLIB_LIBS@
interpreter_LDADD = ${top_builddir}/cattle/libcattle.la

indent_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la common.h
indent_SOURCES = indent.c common.c
indent_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
indent_LDFLAGS = @GLIB_LIBS@
indent_LDADD = ${top_builddir}/cattle/libcattle.la

EXTRA_DIST = common.h
