INCLUDES = -I$(top_srcdir)   \
           -I$(top_builddir)

GENMARSHAL_GENERATED = cattle-marshal.h \
                       cattle-marshal.c
MKENUMS_GENERATED = cattle-enums.h \
                    cattle-enums.c

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

BUILT_SOURCES = $(GENMARSHAL_GENERATED) \
                $(MKENUMS_GENERATED)

CLEANFILES = $(GENMARSHAL_GENERATED) \
             $(MKENUMS_GENERATED)

cattleincludedir = $(includedir)/cattle

cattle_headers = cattle-error.h         \
                 cattle-instruction.h   \
                 cattle-program.h       \
                 cattle-tape.h          \
                 cattle-configuration.h \
                 cattle-interpreter.h   \
                 cattle.h

#cattleinclude_HEADERS = $(cattle_headers) \
#                        cattle-enums.h

noinst_HEADERS = $(cattle_headers) \
                 cattle-version.h  \
                 cattle-enums.h

#lib_LTLIBRARIES = libcattle.la

noinst_LTLIBRARIES = libcattle-1.0.la

libcattle_1_0_la_LDFLAGS = -version-info \
                           $(CATTLE_CURRENT):$(CATTLE_REVISION):$(CATTLE_AGE)

libcattle_1_0_la_LIBADD = $(GLIB_LIBS)

libcattle_1_0_la_SOURCES = $(BUILT_SOURCES)       \
                           cattle-version.c       \
                           cattle-error.c         \
                           cattle-instruction.c   \
                           cattle-program.c       \
                           cattle-tape.c          \
                           cattle-configuration.c \
                           cattle-interpreter.c

libcattle_1_0_la_CFLAGS = $(CFLAGS)      \
                      $(GLIB_CFLAGS)

libcattle_1_0_la_CPPFLAGS = -DG_LOG_DOMAIN=\"Cattle\" \
                            -DCATTLE_COMPILATION      \
                            -DG_DISABLE_DEPRECATED

EXTRA_DIST = cattle-marshal.list \
             cattle-enums.h.tmpl \
             cattle-enums.c.tmpl
