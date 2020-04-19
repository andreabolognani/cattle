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
	AUTHORS.rst \
	COPYING \
	NEWS.rst \
	README.rst \
	TODO.rst \
	$(NULL)

EXTRA_DIST = \
	CONTRIBUTING.rst \
	INSTALL.rst \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-introspection=yes \
	--enable-gtk-doc \
	$(NULL)
