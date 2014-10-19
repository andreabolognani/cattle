Cattle - Brainfuck language toolkit
===================================

Cattle is a GObject-based library that allows one to inspect Brainfuck
programs in various ways, and to easily embed a full-featured Brainfuck
interpreter into any application.


Features
--------

Cattle is able to load and execute virtually any Brainfuck program; both
the code loader and the interpreter have good error checking, and are
able to catch common coding errors such as unbalanced brackets.

Interpreters support flexible I/O using callbacks, which means it is
possible to load a program's input from any source and send its output
to any source. Examples include loading the program's input from a GTK+
widget and sending its output to a network socket.

The behaviour of an interpreter can be further customized, for example
by enabling or disabling the runtime debugging support.

While program execution is a very important aspect of Cattle, it is
possible to process a loaded programs in many ways, including adding or
removing instructions.


Development status
------------------

The library currently provides enough features to implement a working
interpreter in about 70 lines of code (see examples/run.c); the API is
stable and considered good enough for production use.


Limitations
-----------

The main limitation of Cattle lies in its performances: while it performs
better than many interpreters written in a high-level programming language
and than most naive interpreters written in C, it is easily outperformed
by any optimized interpreter written in C.

The reason is mainly the overhead imposed by the GObject type system; while
it would certainly be possible to dump GObject, I feel the features it
provides are worth the performance hit.


Contact information
-------------------

You can contact me anytime at the mail address <eof@kiyuko.org> if you
need any information about Cattle. Suggestion are greatly appreciated.


License
-------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
