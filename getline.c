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

#ifndef _WITH_GETLINE
enum
{
#ifdef NDEBUG
    defaultBufferSize = 128
#else
    defaultBufferSize = 1
#endif /* NDEBUG */
};


ssize_t
getdelim(char** lineptr, size_t* n, int delimiter, FILE* stream)
{
    ssize_t ret = -1;
    char* buffer = NULL;
    size_t bufferSize;
    size_t bufferPos = 0;

    if (lineptr == NULL || n == NULL)
    {
        errno = EINVAL;
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
                errno = EOVERFLOW;
                goto exit;
            }

            newSize = bufferSize * 2;
            tempBuffer = realloc(buffer, newSize);
            if (tempBuffer == NULL)
            {
                errno = ENOMEM;
                goto exit;
            }

            buffer = tempBuffer;
            bufferSize = newSize;
        }

        buffer[bufferPos++] = (char) (unsigned char) c;
        if (c == delimiter)
        {
            break;
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


ssize_t
getline(char** lineptr, size_t* n, FILE* stream)
{
    return getdelim(lineptr, n, '\n', stream);
}
#endif /* _WITH_GETLINE */
