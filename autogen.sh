#!/bin/sh

#gtkdocize --flavour no-tmpl --copy && \
libtoolize --force --copy && \
aclocal && \
autoheader && \
automake --copy --add-missing && \
autoconf
