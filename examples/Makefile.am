NULL =

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(NULL)

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(NULL)

LDADD = \
	$(top_builddir)/cattle/libcattle-1.0.la \
	$(NULL)

common_headers = \
	common.h \
	$(NULL)

common_sources = \
	common.c \
	$(NULL)

noinst_PROGRAMS = \
	indent \
	minimize \
	run \
	$(NULL)

indent_SOURCES = \
	$(common_headers) \
	$(common_sources) \
	indent.c \
	$(NULL)

minimize_SOURCES = \
	$(common_headers) \
	$(common_sources) \
	minimize.c \
	$(NULL)

run_SOURCES = \
	$(common_headers) \
	$(common_sources) \
	run.c \
	$(NULL)
