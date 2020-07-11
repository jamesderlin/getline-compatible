/** getwline.h
  *
  * `wchar_t` implementations for `getdelim` and `getline`.
  *
  * Copyright (C) 2020 James D. Lin <jamesdlin@berkeley.edu>
  *
  * The latest version of this file can be downloaded from:
  * <https://github.com/jamesderlin/getline-compatible>
  *
  * This software is provided 'as-is', without any express or implied
  * warranty.  In no event will the authors be held liable for any damages
  * arising from the use of this software.
  *
  * Permission is granted to anyone to use this software for any purpose,
  * including commercial applications, and to alter it and redistribute it
  * freely, subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  *    claim that you wrote the original software. If you use this software
  *    in a product, an acknowledgment in the product documentation would be
  *    appreciated but is not required.
  *
  * 2. Altered source versions must be plainly marked as such, and must not be
  *    misrepresented as being the original software.
  *
  * 3. This notice may not be removed or altered from any source distribution.
  */

#ifndef GETWLINE_COMPATIBLE_H
#define GETWLINE_COMPATIBLE_H

#include <stdio.h>
#include <limits.h>
#include <wchar.h>

#if    defined __unix__ \
    || defined __linux__ \
    || (defined __APPLE__ && defined __MACH__) \
    || defined __MINGW32__
    /* For `ssize_t`. */
    #include <sys/types.h>
#endif

#ifndef SSIZE_MAX
    #ifndef SIZE_MAX
        #define SIZE_MAX ((size_t) -1)
    #endif

    #if defined ULLONG_MAX && SIZE_MAX == ULLONG_MAX
        typedef long long ssize_t;
        #define SSIZE_MAX LLONG_MAX
    #elif SIZE_MAX == ULONG_MAX
        typedef long ssize_t;
        #define SSIZE_MAX LONG_MAX
    #elif SIZE_MAX == UINT_MAX
        typedef int ssize_t;
        #define SSIZE_MAX INT_MAX
    #elif SIZE_MAX == USHORT_MAX
        typedef short ssize_t;
        #define SSIZE_MAX SHORT_MAX
    #else
        #error `ssize_t` and `SSIZE_MAX` must be defined first.
    #endif
#endif


/** getwdelim
  *
  *     A `wchar_t` version of `getdelim`.
  *
  *     Unlike `getdelim`, `n` represents the size of `*lineptr` in `wchar_t`s
  *     instead of the number of the number of bytes.  Similarly, returns the
  *     number of `wchar_t`s read instead of the number of bytes.
  */
ssize_t getwdelim(wchar_t** lineptr, size_t* n, wint_t delimiter,
                  FILE* stream);


/** getwline
  *
  *     Equivalent to `getwdelim(lineptr, n, L'\n', stream)`.
  */
ssize_t getwline(wchar_t** lineptr, size_t* n, FILE* stream);


/** getwline_univ
  *
  *     A version of `getwline` that recognizes CR, LF, or CR-LF as line
  *     endings, regardless of the platform and of `stream`'s mode.
  */
ssize_t getwline_univ(wchar_t** lineptr, size_t* n, FILE* stream);


#endif /* GETWLINE_COMPATIBLE_H */
