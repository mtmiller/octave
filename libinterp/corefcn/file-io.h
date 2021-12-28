////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2022 The Octave Project Developers
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

// Written by John C. Campbell <jcc@bevo.che.wisc.edu>

#if ! defined (octave_file_io_h)
#define octave_file_io_h 1

#include "octave-config.h"

#include <string>

OCTAVE_NAMESPACE_BEGIN

// Use this function internally until the function that uses it is
// removed.  Remove when corresponding global deprecated function is
// removed.
extern void mark_for_deletion_deprecated (const std::string&);

// Use this function internally until the function that uses it is
// removed.  Remove when corresponding global deprecated function is
// removed.
extern void cleanup_tmp_files_deprecated (void);

OCTAVE_NAMESPACE_END

#if defined (OCTAVE_PROVIDE_DEPRECATED_SYMBOLS)

OCTAVE_DEPRECATED (6, "use 'interpreter::mark_for_deletion' instead")
inline void mark_for_deletion (const std::string& fname)
{
  octave::mark_for_deletion_deprecated (fname);
}

OCTAVE_DEPRECATED (6, "use 'interpreter::cleanup_tmp_files' instead")
inline void cleanup_tmp_files (void)
{
  octave::cleanup_tmp_files_deprecated ();
}

#endif

#endif
