/** getline.h
  *
  * Implementations for POSIX `getdelim` and `getline` functions for non-POSIX
  * systems.
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
    typedef int ssize_t;
    #define SSIZE_MAX INT_MAX
#endif /* SSIZE_MAX */

/* If `_WITH_GETLINE` is defined, assume that `getdelim` and `getline` are
 * available from `stdio.h` and should be used instead.
 */
#ifndef _WITH_GETLINE


/** getdelim
  *
  * Reference:
  *     <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html>
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
