INCLUDES = -I$(top_srcdir) \
           -I$(top_builddir)

noinst_PROGRAMS = tape-tests        \
                  program-tests     \
                  interpreter-tests \
                  references-tests

tape_tests_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
tape_tests_SOURCES = tape-tests.c
tape_tests_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
tape_tests_LDFLAGS = @GLIB_LIBS@
tape_tests_LDADD = ${top_builddir}/cattle/libcattle.la

interpreter_tests_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
interpreter_tests_SOURCES = interpreter-tests.c
interpreter_tests_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
interpreter_tests_LDFLAGS = @GLIB_LIBS@
interpreter_tests_LDADD = ${top_builddir}/cattle/libcattle.la

program_tests_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
program_tests_SOURCES = program-tests.c
program_tests_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
program_tests_LDFLAGS = @GLIB_LIBS@
program_tests_LDADD = ${top_builddir}/cattle/libcattle.la

references_tests_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
references_tests_SOURCES = references-tests.c
references_tests_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@
references_tests_LDFLAGS = @GLIB_LIBS@
references_tests_LDADD = ${top_builddir}/cattle/libcattle.la

GTESTER = gtester

test:
	${GTESTER} --version 2>/dev/null 1>&2 ; \
	test "$$?" != 0 || ${GTESTER} --verbose *-tests

.PHONY: test
check-local: test
