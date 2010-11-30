AUTOMAKE_OPTIONS = 1.6

SUBDIRS = cattle   \
          examples \
          tests    \
          docs

EXTRA_DIST = gtk-doc.make \
             cattle.pc.in

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

#pkgconfigdir = $(libdir)/pkgconfig

#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(pkgconfigdir)
#	$(INSTALL) -m 644 cattle.pc $(DESTDIR)$(pkgconfigdir)/

#uninstall-local:
#	rm -f $(DESTDIR)$(pkgconfigdir)/cattle.pc
