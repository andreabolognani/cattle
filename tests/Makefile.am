NULL =

INCLUDES = \
	-I$(top_srcdir) \
    -I$(top_builddir) \
	$(NULL)

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(NULL)

LDADD = \
	$(top_builddir)/cattle/libcattle-1.0.la \
	$(NULL)

noinst_PROGRAMS = \
	tape \
	program \
	interpreter \
	references \
	$(NULL)

tape_SOURCES = \
	tape.c \
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

GTESTER = gtester

test:
	@${GTESTER} --version 2>/dev/null 1>&2 ; \
	test "$$?" != 0 || ${GTESTER} --verbose ${noinst_PROGRAMS}

.PHONY: test
check-local: test
