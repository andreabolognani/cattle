NULL =

SUBDIRS = \
	cattle \
	docs \
	examples \
	tests \
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = \
	cattle-1.0.pc \
	$(NULL)

docdir = $(datarootdir)/doc/cattle-1.0
dist_doc_DATA = \
	AUTHORS.markdown \
	ChangeLog \
	ChangeLog.pre-0.9.2 \
	COPYING \
	NEWS.markdown \
	README.markdown \
	TODO.markdown \
	$(NULL)

EXTRA_DIST = \
	INSTALL.markdown \
	$(NULL)

CLEANFILES = \
	ChangeLog \
	$(NULL)

ChangeLog:
	$(AM_V_GEN) \
	if test -d "$(top_srcdir)/.git"; \
	then \
	  GIT_DIR="$(top_srcdir)/.git" \
	  $(top_srcdir)/missing \
	    --run git log --color=never --stat \
	  >$@.tmp \
	  && mv -f $@.tmp $@ \
	  || ( $(RM) -f $@.tmp; false ) \
	fi

.PHONY: ChangeLog

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-introspection=yes \
	--enable-gtk-doc \
	$(NULL)
