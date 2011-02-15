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
	cattle-instruction.h \
	cattle-program.h \
	cattle-tape.h \
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

libcattle_1_0_la_SOURCES = \
	$(BUILT_SOURCES) \
	$(noinst_HEADERS) \
	cattle-version.c \
	cattle-error.c \
	cattle-instruction.c \
	cattle-program.c \
	cattle-tape.c \
	cattle-configuration.c \
	cattle-interpreter.c \
	$(NULL)

GENMARSHAL_GENERATED = \
	cattle-marshal.h \
	cattle-marshal.c \
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
	cattle-marshal.list \
	cattle-enums.h.tmpl \
	cattle-enums.c.tmpl \
	$(NULL)

cattle-marshal.h: cattle-marshal.list
	$(AM_V_GEN)( $(GLIB_GENMARSHAL) --prefix=cattle_marshal       \
	  $(srcdir)/cattle-marshal.list --header > cattle-marshal.tmp \
	&& mv cattle-marshal.tmp cattle-marshal.h )                   \
	|| ( rm -f cattle-marshal.tmp && exit 1 )

cattle-marshal.c: cattle-marshal.h
	$(AM_V_GEN)( ( echo '#include "cattle-marshal.h"';            \
	  $(GLIB_GENMARSHAL) --prefix=cattle_marshal                  \
	  $(srcdir)/cattle-marshal.list --body ) > cattle-marshal.tmp \
	&& mv cattle-marshal.tmp cattle-marshal.c )                   \
	|| ( rm -f cattle-marshal.tmp && exit 1 )

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
