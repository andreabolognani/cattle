AUTOMAKE_OPTIONS = 1.6

# The name of the module.
DOC_MODULE = cattle

# The top-level SGML file.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

# The directory containing the source code. Relative to $(srcdir).
DOC_SOURCE_DIR = $(top_srcdir)/cattle

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS =

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS = --rebuild-types

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS = --sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-mktmpl
MKTMPL_OPTIONS =

# Extra options to supply to gtkdoc-fixref. Not normally needed.
FIXXREF_OPTIONS =

# Used for dependencies. The docs will be rebuilt if any of these change.
HFILE_GLOB = $(top_srcdir)/cattle/*.h
CFILE_GLOB = $(top_srcdir)/cattle/*.c

# Header files to ignore when scanning.
IGNORE_HFILES = cattle-enums.h \
                cattle-marshal.h \
                cattle.h

# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = \
	overview.xml \
	io-handling.xml \
	version.xml \
	$(NULL)

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded.
# e.g. expand_content_files=running.sgml
expand_content_files =

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
GTKDOC_CFLAGS = -I$(top_srcdir) 	\
                -I$(top_builddir)	\
                $(GLIB_CFLAGS)
GTKDOC_LIBS = $(top_builddir)/cattle/libcattle-1.0.la	\
              $(GLIB_LIBS)

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST +=
