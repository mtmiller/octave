////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2012-2022 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

#if ! defined (octave_lo_cutils_h)
#define octave_lo_cutils_h 1

#include "octave-config.h"

#if defined (__cplusplus)
#include <cstddef>
using std::size_t;
extern "C" {
#else
#include <stddef.h>
#endif

OCTAVE_API void
octave_qsort (void *base, size_t n, size_t size,
              int (*cmp) (const void *, const void *));

OCTAVE_API int
octave_strcasecmp (const char *s1, const char *s2);

OCTAVE_API int
octave_strncasecmp (const char *s1, const char *s2, size_t n);

#if defined (__cplusplus)
}
#endif

#endif
