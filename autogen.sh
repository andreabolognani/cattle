#!/bin/sh

#gtkdocize --flavour no-tmpl --copy
#if [ "$?" -ne 0 ]; then echo "gtkdocize failed" >&2; exit 1; fi

libtoolize --force --copy
if [ "$?" -ne 0 ]; then echo "libtoolize failed" >&2; exit 1; fi

aclocal
if [ "$?" -ne 0 ]; then echo "aclocal failed" >&2; exit 1; fi

autoheader
if [ "$?" -ne 0 ]; then echo "autoheader failed" >&2; exit 1; fi

automake --copy --add-missing --foreign
if [ "$?" -ne 0 ]; then echo "automake failed" >&2; exit 1; fi

autoconf
if [ "$?" -ne 0 ]; then echo "autoconf failed" >&2; exit 1; fi
