#include "ggets.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "getline.h"


int
fggets(char** line, FILE* stream)
{
    enum { fggets_success, fggets_failure };

    int ret = fggets_failure;
    char* buffer = NULL;
    size_t bufferSize = 0;
    ssize_t bytesRead;

    assert(line != NULL);
    assert(stream != NULL);

    bytesRead = getline(&buffer, &bufferSize, stream);

    if (bytesRead < 0)
    {
        if (feof(stream))
        {
            ret = EOF;
        }
        else if (errno == ENOMEM)
        {
            /* Transfer ownership of the partially-read line. */
            *line = buffer;
            buffer = NULL;
        }
        else
        {
            *line = NULL;
        }

        goto exit;
    }

    if (bytesRead > 0 && buffer[bytesRead - 1] == '\n')
    {
        bytesRead--;
        buffer[bytesRead] = '\0';
    }

    *line = realloc(buffer, bytesRead + 1 /* NUL */);
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


int
ggets(char** line)
{
    return fggets(line, stdin);
}
