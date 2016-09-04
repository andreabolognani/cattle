NULL =

SUBDIRS = \
	cattle \
	docs \
	examples \
	tests \
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	cattle-1.0.pc \
	$(NULL)

doc_DATA = \
	AUTHORS.md \
	ChangeLog \
	ChangeLog.pre-0.9.2 \
	COPYING \
	NEWS.md \
	README.md \
	TODO.md \
	$(NULL)

EXTRA_DIST = \
	$(doc_DATA) \
	$(pkgconfig_DATA) \
	$(NULL)

CLEANFILES = \
	ChangeLog \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-introspection=yes \
	--enable-gtk-doc \
	$(NULL)

ChangeLog:
	$(AM_V_GEN)\
	( \
	  if test -d "$(top_srcdir)/.git"; \
	  then \
	    ( \
	      GIT_DIR="$(top_srcdir)/.git" \
	      $(top_srcdir)/missing --run git log --color=never --stat \
	    ) > "$@.tmp" && \
	    mv -f "$@.tmp" "$@"; \
	  fi \
	)

.PHONY: ChangeLog
