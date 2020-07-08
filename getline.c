/** getline.c
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

#include "getline.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#else
    typedef enum { false, true } bool;
#endif

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof *(a))

enum
{
#ifdef NDEBUG
    defaultBufferSize = 128
#else
    defaultBufferSize = 1
#endif /* NDEBUG */
};


/** get_delimited_of
  *
  *     Like `getdelim`, but retrieves the next line from `stream` delimited by
  *     any of the characters (each represented as an `unsigned char`) from
  *     `delimiters`.
  */
static ssize_t
get_delimited_of(char** lineptr, size_t* n,
                 const int* delimiters, size_t numDelimiters,
                 FILE* stream)
{
    ssize_t ret = -1;
    char* buffer = NULL;
    size_t bufferSize;
    size_t bufferPos = 0;

    if (   lineptr == NULL || n == NULL
        || delimiters == NULL || numDelimiters == 0)
    {
        assert(false);
    #ifdef EINVAL
        errno = EINVAL;
    #else
        errno = EDOM;
    #endif
        goto exit;
    }

    if (feof(stream))
    {
        goto exit;
    }

    buffer = *lineptr;
    bufferSize = *n;

    if (buffer == NULL)
    {
        if (bufferSize == 0)
        {
            bufferSize = defaultBufferSize;
        }

        buffer = malloc(bufferSize);
        if (buffer == NULL)
        {
            errno = ENOMEM;
            goto exit;
        }
    }

    while (true)
    {
        int c = fgetc(stream);
        if (c == EOF)
        {
            if (bufferPos == 0 || ferror(stream))
            {
                goto exit;
            }

            if (feof(stream))
            {
                break;
            }
        }

        if (bufferPos + 1 == bufferSize)
        {
            size_t newSize;
            char* tempBuffer;
            if (bufferSize > (size_t) (SSIZE_MAX / 2))
            {
            #ifdef EOVERFLOW
                errno = EOVERFLOW;
            #else
                errno = ERANGE;
            #endif
                goto exit;
            }

            newSize = bufferSize * 2;
            tempBuffer = realloc(buffer, newSize);
            if (tempBuffer == NULL)
            {
            #ifdef EOVERFLOW
                errno = ENOMEM;
            #else
                errno = ERANGE;
            #endif
                goto exit;
            }

            buffer = tempBuffer;
            bufferSize = newSize;
        }

        buffer[bufferPos++] = (char) (unsigned char) c;

        {
            size_t i;
            for (i = 0; i < numDelimiters && delimiters[i] != c; i++) { }
            if (i != numDelimiters)
            {
                /* We found a delimiter. */
                break;
            }
        }
    }

    assert(bufferPos < (size_t) SSIZE_MAX);
    ret = (ssize_t) bufferPos;

exit:
    if (buffer != NULL)
    {
        /* Set output parameters even if we fail.  The `getdelim` specification
         * states that `*lineptr` and `*n` should be updated for any
         * allocation, not just if `getdelim` succeeds.
         */
        assert(bufferPos < bufferSize);
        buffer[bufferPos] = '\0';
        *lineptr = buffer;
        *n = bufferSize;
    }
    return ret;
}


#ifndef _WITH_GETLINE
ssize_t
getdelim(char** lineptr, size_t* n, int delimiter, FILE* stream)
{
    return get_delimited_of(lineptr, n, &delimiter, 1, stream);
}


ssize_t
getline(char** lineptr, size_t* n, FILE* stream)
{
    int delimiter = '\n';
    return get_delimited_of(lineptr, n, &delimiter, 1, stream);
}
#endif /* _WITH_GETLINE */


ssize_t
getline_univ(char** lineptr, size_t* n, FILE* stream)
{
    char* line;
    int delimiters[] = { '\r', '\n' };
    ssize_t bytesRead = get_delimited_of(lineptr, n,
                                         delimiters, ARRAY_LENGTH(delimiters),
                                         stream);
    if (bytesRead <= 0)
    {
        return bytesRead;
    }

    line = *lineptr;
    assert(line[bytesRead] == '\0');
    if (line[bytesRead - 1] == '\r')
    {
        int next;

        line[bytesRead - 1] = '\n';

        next = fgetc(stream);
        if (next == EOF)
        {
            clearerr(stream);
        }
        else if (next != '\n')
        {
            ungetc(next, stream);
        }
    }
    return bytesRead;
}
