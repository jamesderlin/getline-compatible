/** getwline.c
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

#define GETLINE_USE_WCHAR
#include "getline.c"


ssize_t
getwdelim(wchar_t** lineptr, size_t* n, wint_t delimiter, FILE* stream)
{
    return getwdelimof(lineptr, n, &delimiter, 1, stream);
}


ssize_t
getwline(wchar_t** lineptr, size_t* n, FILE* stream)
{
    wint_t delimiter = L'\n';
    return getwdelimof(lineptr, n, &delimiter, 1, stream);
}
