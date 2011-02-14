INCLUDES = -I$(top_srcdir)   \
           -I$(top_builddir)

noinst_PROGRAMS = run      \
                  indent   \
                  minimize

run_SOURCES = run.c common.c common.h
run_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@ @GIO_CFLAGS@
run_LDFLAGS = @GLIB_LIBS@ @GIO_LIBS@
run_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

indent_SOURCES = indent.c common.c common.h
indent_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@ @GIO_CFLAGS@
indent_LDFLAGS = @GLIB_LIBS@ @GIO_LIBS@
indent_LDADD = $(top_builddir)/cattle/libcattle-1.0.la

minimize_SOURCES = minimize.c common.c common.h
minimize_CPPFLAGS = @CFLAGS@ @GLIB_CFLAGS@ @GIO_CFLAGS@
minimize_LDFLAGS = @GLIB_LIBS@ @GIO_LIBS@
minimize_LDADD = $(top_builddir)/cattle/libcattle-1.0.la
