/** ggets.h
  *
  * Reimplementations of Chuck B. Falconer's `ggets` and `fggets`.  Unlike the
  * original versions, these grows the allocated buffer exponentially instead
  * of linearly.
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


/** fggets_univ
  *
  *     A version of `fggets` that recognizes CR, LF, or CR-LF as line endings,
  *     regardless of the platform and of `stream`'s mode.
  */
int fggets_univ(char** line, FILE* stream);

#endif /* GGETS_COMPATIBLE_H */
