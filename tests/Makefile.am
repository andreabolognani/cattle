INCLUDES = -I$(top_srcdir) \
           -I$(top_builddir)

noinst_PROGRAMS = tape-limits-test

tape_limits_test_DEPENDENCIES = ${top_builddir}/cattle/libcattle.la
tape_limits_test_SOURCES = tape-limits-test.c
tape_limits_test_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
tape_limits_test_LDFLAGS = @GLIB_LIBS@
tape_limits_test_LDADD = ${top_builddir}/cattle/libcattle.la
