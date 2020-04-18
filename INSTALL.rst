Building and installing Cattle
==============================

This document explains how to build and install Cattle from source.


Prerequisites
-------------

Cattle depends on the following libraries and programs:

* the GLib library, with its GObject module;

* optionally, GObject Introspection, if you want to access Cattle
  from languages other than C;

* optionally, the GTK-Doc tools, if you wish to build the API
  reference.


Building and installing
-----------------------

In-tree builds are not supported, so first of all you'll need to
create a build directory:

::

   $ mkdir build && cd build

If you're building from a release archive, you can use the standard
command sequence:

::

   $ ../configure
   $ make
   $ make check
   $ sudo make install

If you're building from a git clone, you'll need to generate the
build system first: doing so is as easy as running

::

   $ ../autogen.sh

instead of ``configure`` as the first command; everything else
remains the same.
