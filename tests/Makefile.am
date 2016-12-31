NULL =

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(NULL)

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(NULL)

LDADD = \
	$(top_builddir)/cattle/libcattle-1.0.la \
	$(NULL)

noinst_PROGRAMS = \
	buffer \
	interpreter \
	program \
	references \
	tape \
	$(NULL)

buffer_SOURCES = \
	buffer.c \
	$(NULL)

interpreter_SOURCES = \
	interpreter.c \
	$(NULL)

program_SOURCES = \
	program.c \
	$(NULL)

references_SOURCES = \
	references.c \
	$(NULL)

tape_SOURCES = \
	tape.c \
	$(NULL)

GTESTER = gtester

check-local:
	@$(GTESTER) --version >/dev/null 2>&1 && \
	 $(GTESTER) --verbose $(noinst_PROGRAMS)
