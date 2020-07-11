/** getline.h
  *
  * Implementations for POSIX `getdelim` and `getline` functions for non-POSIX
  * systems.
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

#ifndef GETLINE_COMPATIBLE_H
#define GETLINE_COMPATIBLE_H

#include <stdio.h>
#include <limits.h>

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

/* If `_WITH_GETLINE` is defined, assume that `getdelim` and `getline` are
 * available from `stdio.h` and should be used instead.
 */
#ifndef _WITH_GETLINE


/** getdelim
  *
  *     See: <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html>
  *
  * Compatibility:
  *     If `EINVAL` is not available, sets `errno` to `EDOM` instead.
  *
  *     If `EOVERFLOW` is not available, sets `errno` to `ERANGE` instead.
  *
  *     If `ENOMEM` is not available, sets `errno` to `ERANGE` instead.
  */
ssize_t getdelim(char** lineptr, size_t* n, int delimiter, FILE* stream);


/** getline
  *
  *     Equivalent to `getdelim(lineptr, n, '\n', stream)`.
  */
ssize_t getline(char** lineptr, size_t* n, FILE* stream);

#endif /* _WITH_GETLINE */


/** getline_univ
  *
  *     A version of `getline` that recognizes CR, LF, or CR-LF as line
  *     endings, regardless of the platform and of `stream`'s mode.
  */
ssize_t getline_univ(char** lineptr, size_t* n, FILE* stream);


#endif /* GETLINE_COMPATIBLE_H */
