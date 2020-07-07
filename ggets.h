/** ggets.h
  *
  * Reimplementations of Chuck B. Falconer's `ggets` and `fggets`.  Unlike the
  * original versions, these grows the allocated buffer exponentially instead
  * of linearly.
  */

#ifndef GGETS_COMPATIBLE_H
#define GGETS_COMPATIBLE_H

#include <stdio.h>


/** fggets
  *
  *     A version of `fgets` that automatically allocates a sufficiently large
  *     buffer and that automatically strips any trailing newline.
  *
  *     Note that unlike `getline` and `fgets`, `fggets` cannot distinguish
  *     between a final line that is terminated with a newline and a final line
  *     that is unterminated.
  *
  * PARAMETERS:
  *     OUT line      : Set to the allocated line.
  *     IN/OUT stream : The file stream to read from.
  *
  * RETURNS:
  *     Returns 0 on success.
  *
  *     Returns EOF if the end-of-file indicator for `stream` is set.
  *
  *     Returns a positive value on failure.  On memory allocation failure,
  *     `*line` may be set to a partially read line.  For all other types of
  *     failure, `*line` will be set to `NULL`.
  *
  *     If `*line` is non-`NULL`, the caller is responsible for freeing the
  *     memory when done.
  */
int fggets(char** line, FILE* stream);


/** ggets
  *
  *     Equivalent to `fggets(line, stdin)`.
  */
int ggets(char** line);

#endif /* GGETS_COMPATIBLE_H */
