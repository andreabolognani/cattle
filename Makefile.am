AUTOMAKE_OPTIONS = 1.6

SUBDIRS = cattle   \
          examples \
          tests    \
          docs

EXTRA_DIST = ChangeLog.pre-0.9.2 \
             gtk-doc.make        \
             cattle-1.0.pc.in

CLEANFILES = $(srcdir)/ChangeLog

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

distclean-local:
	if test "$(srcdir)" = "."; then \
	  :; \
	else \
	  rm -f ChangeLog; \
	fi

ChangeLog:
	@echo Creating $@
	@if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./missing --run git log --stat) > $@.tmp \
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

#pkgconfigdir = $(libdir)/pkgconfig

#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(pkgconfigdir)
#	$(INSTALL) -m 644 cattle.pc $(DESTDIR)$(pkgconfigdir)/

#uninstall-local:
#	rm -f $(DESTDIR)$(pkgconfigdir)/cattle.pc
