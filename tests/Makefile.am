INCLUDES = -I$(top_srcdir)   \
           -I$(top_builddir)

noinst_PROGRAMS = tape        \
                  program     \
                  interpreter \
                  references

tape_SOURCES = tape.c
tape_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
tape_LDFLAGS = @GLIB_LIBS@
tape_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

interpreter_SOURCES = interpreter.c
interpreter_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
interpreter_LDFLAGS = @GLIB_LIBS@
interpreter_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

program_SOURCES = program.c
program_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
program_LDFLAGS = @GLIB_LIBS@
program_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

references_SOURCES = references.c
references_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
references_LDFLAGS = @GLIB_LIBS@
references_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

GTESTER = gtester

test:
	@${GTESTER} --version 2>/dev/null 1>&2 ; \
	test "$$?" != 0 || ${GTESTER} --verbose ${noinst_PROGRAMS}

.PHONY: test
check-local: test
