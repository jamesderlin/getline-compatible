/** ggets.c
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

#ifdef GETLINE_USE_WCHAR
    #include "ggetws.h"
#else
    #include "ggets.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "getline.h"


#ifdef GETLINE_USE_WCHAR
    typedef wchar_t TCHAR;

    #define T(x) L ## x
    #define TEOF WEOF
    #define FGGETTS_INTERNAL fggetws_internal
#else
    typedef char TCHAR;

    #define T(x) x
    #define TEOF EOF
    #define FGGETTS_INTERNAL fggets_internal
#endif

typedef ssize_t (*getline_func)(TCHAR** lineptr, size_t* n, FILE* stream);


/** fggets_internal
  *
  *     Internal wrapper around `getline` or `getline_univ`.
  */
static int
FGGETTS_INTERNAL(TCHAR** line, FILE* stream, getline_func getline)
{
    enum { fggets_success, fggets_failure };

    int ret = fggets_failure;
    TCHAR* buffer = NULL;
    size_t bufferSize = 0;
    ssize_t elementsRead;

    assert(line != NULL);
    assert(stream != NULL);

    elementsRead = getline(&buffer, &bufferSize, stream);

    if (elementsRead < 0)
    {
        if (feof(stream))
        {
            ret = TEOF;
        }
    #ifdef ENOMEM
        else if (errno == ENOMEM)
        {
            /* Transfer ownership of the partially-read line. */
            *line = buffer;
            buffer = NULL;
        }
    #endif
        else
        {
            *line = NULL;
        }

        goto exit;
    }

    if (elementsRead > 0 && buffer[elementsRead - 1] == T('\n'))
    {
        elementsRead--;
        buffer[elementsRead] = T('\0');
    }

    /* Shrink the buffer to the minimum size necessary. */
    *line = realloc(buffer, (elementsRead + 1 /* NUL */) * sizeof *buffer);
    if (*line == NULL)
    {
        *line = buffer;
    }

    buffer = NULL;
    ret = fggets_success;

exit:
    free(buffer);
    return ret;
}


#ifndef GETLINE_USE_WCHAR
int
fggets(char** line, FILE* stream)
{
    return fggets_internal(line, stream, getline);
}


int
fggets_univ(char** line, FILE* stream)
{
    return fggets_internal(line, stream, getline_univ);
}


int
ggets(char** line)
{
    return fggets(line, stdin);
}
#endif /* GETLINE_USE_WCHAR */
