////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016-2024 The Octave Project Developers
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

#if ! defined (octave_uname_wrapper_h)
#define octave_uname_wrapper_h 1

#if defined __cplusplus
extern "C" {
#endif

extern OCTAVE_API int
octave_uname_wrapper (char **sysname, char **nodename,
                      char **release, char **version, char **machine);

#if defined __cplusplus
}
#endif

#endif
