NULL =

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(NULL)

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(NULL)

LDADD = \
	$(top_builddir)/cattle/libcattle-1.0.la \
	$(NULL)

noinst_PROGRAMS = \
	run \
	indent \
	minimize \
	$(NULL)

run_SOURCES = \
	common.h \
	common.c \
	run.c \
	$(NULL)

indent_SOURCES = \
	common.h \
	common.c \
	indent.c \
	$(NULL)

minimize_SOURCES = \
	common.h \
	common.c \
	minimize.c \
	$(NULL)
