NULL =

SUBDIRS = \
	cattle \
	examples \
	tests \
	docs \
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	cattle-1.0.pc \
	$(NULL)

doc_DATA = \
	AUTHORS.md \
	ChangeLog.pre-0.9.2 \
	ChangeLog \
	COPYING \
	NEWS.md \
	README.md \
	TODO.md \
	$(NULL)

EXTRA_DIST = \
	$(pkgconfig_DATA) \
	$(doc_DATA) \
	$(NULL)

CLEANFILES = \
	$(srcdir)/ChangeLog \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-introspection=yes \
	--enable-gtk-doc \
	$(NULL)

distclean-local:
	if test "$(srcdir)" = "."; then \
	  :; \
	else \
	  rm -f ChangeLog; \
	fi

ChangeLog:
	@echo Creating $@
	@if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git $(top_srcdir)/missing --run git log --stat) > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	  echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	  (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

.PHONY: ChangeLog
