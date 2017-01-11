Cattle 1.2.2 (2017-01-11)
-------------------------

  * Enable compiler warnings

    - Fix code quality issues uncovered by doing so


Cattle 1.2.1 (2016-09-11)
-------------------------

  * Fix compilation with Clang (Baptiste Fontaine)

  * Install `cattle-enums.h`

  * Build system cleanups and improvements


Cattle 1.2.0 (2014-10-21)
-------------------------

  * `CattleBuffer`:

    - new object used to pass data around.

  * `CattleInterpreter`:

    - input is no longer checked for UTF-8 correctness: all input is
      processed as-is, null bytes included.

  * All offsets are now unsigned long integers.

  * Improved portability: Cattle should now run on most CPU
    architectures (tested on x86_64 and ARMv7).

  * Various bug fixes.


Cattle 1.0.1 (2011-03-27)
-------------------------

  * `CattleInterpreter`:

    - fix a double free for programs with embedded input.


Cattle 1.0.0 (2011-02-28)
-------------------------

  * `CattleInterpreter`:

    - check for unbalanced brackets at runtime;

    - use callbacks instead of signals for I/O;

    - return a generic error if an I/O handler has failed without
      providing detailed information about the error;

    - check input for UTF-8 correctness;

    - check input for non-ASCII characters.

  * `CattleTape`:

    - add the `CattleTape:current-value` property.

  * `CattleError`:

    - merge the `CattleProgramError` and `CattleInterpreterError`
      enumerations into a single enumeration.

  * Enable GObject Introspection support.

  * Various bug fixes.


Cattle 0.9.4 (2010-12-04)
-------------------------

  * `CattleInstruction`:

    - rename `CATTLE_INSTRUCTION_DUMP_TAPE` to `CATTLE_INSTRUCTION_DEBUG`,
      since the corresponding handler might do more than dumping the
      tape's contents.

  * `CattleTape`:

    - add fast bulk increase/decrease/move operations;

    - implement current value wrapping on increase/decrease.

  * `CattleInterpreter`:

    - use bulk operations for speed increase.

  * Add runtime and compile-time version information.

  * Examples:

    - use GIO to read contents of files;

    - add a simple Brainfuck source code minimizer.


Cattle 0.9.3 (2009-05-05)
-------------------------

  * `CattleInterpreter`:

    - provide a default implementations for all the I/O operations;

    - invoke only one handler on I/O signal emission;

    - remove the signal prototypes from the class structure;

    - add an error domain for runtime errors.

  * `CattleProgram`:

    - use a single error code for both unmatched and unbalanced brackets.

  * The caller owns the object returned by a method call, and must
    unreference it when finished to avoid memory leaks.

  * Enforce single header includes.

  * Expand the test suite and use the GTest framework.

  * Various bug fixes.


Cattle 0.9.2 (2008-10-02)
-------------------------

  * `CattleTape`:

    - implement bookmarks.

  * `CattleInterpreter`:

    - add the `CattleInterpreter::debug-request` signal, which behaves like
      the existing I/O signals and obsoletes the `CattleTape.dump()` method.

  * Cache the object's private structure, to avoid looking it up for
    every single method call.

  * Various bug fixes.


Cattle 0.9.1 (2008-08-03)
-------------------------

  * `CattleTape`:

    - use memory chunks instead of allocating each cell separately,
      reducing memory usage and improving performances.

  * `CattleInterpreter`:

    - support flexible I/O using signals and callbacks.

  * Make all structures completely opaque.

  * Some bug fixes.


Cattle 0.9.0 (2008-04-16)
-------------------------

  * First release.
