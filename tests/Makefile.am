INCLUDES = -I$(top_srcdir) \
           -I$(top_builddir)

noinst_PROGRAMS = tape-tests

tape_tests_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
tape_tests_SOURCES = tape-tests.c
tape_tests_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
tape_tests_LDFLAGS = @GLIB_LIBS@
tape_tests_LDADD = ${top_builddir}/cattle/libcattle.la
