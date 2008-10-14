INCLUDES = -I$(top_srcdir) \
           -I$(top_builddir)

noinst_PROGRAMS = interpreter	\
				  indent

interpreter_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
interpreter_SOURCES = interpreter.c
interpreter_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
interpreter_LDFLAGS = @GLIB_LIBS@
interpreter_LDADD = ${top_builddir}/cattle/libcattle.la

indent_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
indent_SOURCES = indent.c
indent_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
indent_LDFLAGS = @GLIB_LIBS@
indent_LDADD = ${top_builddir}/cattle/libcattle.la
