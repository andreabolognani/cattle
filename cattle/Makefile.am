NULL =

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-DCATTLE_COMPILATION \
	-DG_DISABLE_DEPRECATED \
	-DG_LOG_DOMAIN=\"Cattle\" \
	$(NULL)

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(GLIB_LIBS) \
	-version-info $(CATTLE_CURRENT):$(CATTLE_REVISION):$(CATTLE_AGE) \
	$(NULL)

cattle_headers = \
	cattle.h \
	cattle-buffer.h \
	cattle-configuration.h \
	cattle-constants.h \
	cattle-error.h \
	cattle-instruction.h \
	cattle-interpreter.h \
	cattle-program.h \
	cattle-tape.h \
	$(NULL)

cattle_sources = \
	cattle-buffer.c \
	cattle-configuration.c \
	cattle-constants.c \
	cattle-error.c \
	cattle-instruction.c \
	cattle-interpreter.c \
	cattle-program.c \
	cattle-tape.c \
	cattle-version.c \
	$(NULL)

mkenums_generated_headers = \
	cattle-enums.h \
	$(NULL)

mkenums_generated_sources = \
	cattle-enums.c \
	$(NULL)

cattle_generated_headers = \
	$(mkenums_generated_headers) \
	cattle-version.h \
	$(NULL)

cattle_generated_sources = \
	$(mkenums_generated_sources) \
	$(NULL)

cattleincludedir = $(includedir)/cattle-1.0/cattle
cattleinclude_HEADERS = \
	$(cattle_headers) \
	$(NULL)

nodist_cattleinclude_HEADERS = \
	$(cattle_generated_headers) \
	$(NULL)

lib_LTLIBRARIES = \
	libcattle-1.0.la \
	$(NULL)

libcattle_1_0_la_SOURCES = \
	$(cattle_headers) \
	$(cattle_sources) \
	$(NULL)

nodist_libcattle_1_0_la_SOURCES = \
	$(cattle_generated_headers) \
	$(cattle_generated_sources) \
	$(NULL)

BUILT_SOURCES = \
	$(cattle_generated_headers) \
	$(cattle_generated_sources) \
	$(NULL)

CLEANFILES = \
	$(mkenums_generated_headers) \
	$(mkenums_generated_sources) \
	$(NULL)

EXTRA_DIST = \
	cattle-enums.c.tmpl \
	cattle-enums.h.tmpl \
	$(NULL)

cattle-enums.h: $(cattle_headers) cattle-version.h
	$(AM_V_GEN) \
	$(GLIB_MKENUMS) \
	  --template $(srcdir)/cattle-enums.h.tmpl $^ \
	>$@.tmp \
	&& mv -f $@.tmp $@ \
	|| ( $(RM) -f $@.tmp; false )

cattle-enums.c: $(cattle_headers) cattle-version.h
	$(AM_V_GEN) \
	$(GLIB_MKENUMS) \
	  --template $(srcdir)/cattle-enums.c.tmpl $^ \
	>$@.tmp \
	&& mv -f $@.tmp $@ \
	|| ( $(RM) -f $@.tmp; false )

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = \
	-I $(top_srcdir) \
	-I $(top_builddir) \
	-D CATTLE_COMPILATION \
	--warn-all \
	$(NULL)
INTROSPECTION_COMPILER_ARGS = \
	--includedir=$(srcdir) \
	$(NULL)

if HAVE_INTROSPECTION
introspection_sources = \
	$(cattle_generated_headers) \
	$(cattle_generated_sources) \
	$(cattle_headers) \
	$(cattle_sources) \
	$(NULL)

Cattle-1.0.gir: libcattle-1.0.la
Cattle_1_0_gir_NAMESPACE = Cattle
Cattle_1_0_gir_VERSION = 1.0
Cattle_1_0_gir_INCLUDES = GObject-2.0
Cattle_1_0_gir_LIBS = libcattle-1.0.la
Cattle_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Cattle-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
