# getline-compatible

Portable implementations of POSIX `getdelim` and `getline` functions for
non-POSIX systems, along with some other line-reading utilities.

## `ggets`/`fggets`

Provides reimplementations of [Chuck Falconer's `ggets` and `fggets`
functions][archived-ggets], versions of `gets` and `fgets` that automatically
allocate a buffer of the necessary size and that always strip any trailing
newline.  These have friendlier syntax than `getline`.

(Note that because they always strip a trailing newline, callers will not be
able to distinguish a final line that contains a newline from a final line that
does not.  Some consider this to be a bug; others consider this to be a
feature.)

## Universal newlines

Additionally provides `getline_univ` and `fggets_univ`, versions that recognize
any of `CR`, `LF`, or `CR-LF` as line endings, regardless of the platform and
of whether the stream has been opened in text or binary modes.  These provide
behavior similar to universal newline support in Python.

## Portability

To try to maximize portability, code is written in C89.

## `wchar_t` support

Optionally provides `wchar_t` versions for systems that do not support UTF-8.
Note that `wchar_t` versions require `wchar.h`, which is not available in C89.

---

Questions?  Comments?  Bugs?  I welcome feedback. [Contact me].

Copyright © 2020 James D. Lin

[archived-ggets]: http://www.taenarum.com/software/ggets/
[Contact me]: http://www.taenarum.com/contact.html
