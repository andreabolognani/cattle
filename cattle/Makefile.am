NULL =

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(NULL)

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Cattle\" \
	-DCATTLE_COMPILATION \
	-DG_DISABLE_DEPRECATED \
	-DG_DISABLE_SINGLE_INCLUDES \
	$(GLIB_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	-version-info $(CATTLE_CURRENT):$(CATTLE_REVISION):$(CATTLE_AGE) \
	$(GLIB_LIBS) \
	$(NULL)

cattle_headers = \
	cattle-error.h \
	cattle-constants.h \
	cattle-buffer.h \
	cattle-tape.h \
	cattle-instruction.h \
	cattle-program.h \
	cattle-configuration.h \
	cattle-interpreter.h \
	cattle.h \
	$(NULL)

cattleincludedir = $(includedir)/cattle-1.0/cattle
cattleinclude_HEADERS = \
	$(cattle_headers) \
	cattle-version.h \
	$(NULL)

lib_LTLIBRARIES = \
	libcattle-1.0.la \
	$(NULL)

cattle_sources = \
	cattle-version.c \
	cattle-constants.c \
	cattle-error.c \
	cattle-buffer.c \
	cattle-tape.c \
	cattle-instruction.c \
	cattle-program.c \
	cattle-configuration.c \
	cattle-interpreter.c \
	$(NULL)

libcattle_1_0_la_SOURCES = \
	$(cattle_sources) \
	$(cattle_headers) \
	cattle-version.h \
	$(BUILT_SOURCES) \
	$(NULL)

GENMARSHAL_GENERATED = \
	$(NULL)

MKENUMS_GENERATED = \
	cattle-enums.h \
	cattle-enums.c \
	$(NULL)

BUILT_SOURCES = \
	$(GENMARSHAL_GENERATED) \
	$(MKENUMS_GENERATED) \
	$(NULL)

CLEANFILES = \
	$(GENMARSHAL_GENERATED) \
	$(MKENUMS_GENERATED) \
	$(NULL)

EXTRA_DIST = \
	cattle-enums.h.tmpl \
	cattle-enums.c.tmpl \
	$(NULL)

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
	$(cattle_sources) \
	$(cattle_headers) \
	cattle-version.h \
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

cattle-enums.h: $(cattle_headers)
	$(AM_V_GEN)( cd $(srcdir) &&                     \
	  $(GLIB_MKENUMS) --template cattle-enums.h.tmpl \
	  $(cattle_headers) ) > cattle-enums.h.tmp       \
	&& mv cattle-enums.h.tmp cattle-enums.h          \
	|| rm -f cattle-enums.h.tmp

cattle-enums.c: $(cattle_headers)
	$(AM_V_GEN)( cd $(srcdir) &&                     \
	  $(GLIB_MKENUMS) --template cattle-enums.c.tmpl \
	  $(cattle_headers) ) > cattle-enums.c.tmp       \
	&& mv cattle-enums.c.tmp cattle-enums.c          \
	|| rm -f cattle-enums.c.tmp
