////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2003-2021 The Octave Project Developers
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

#if ! defined (octave_ls_oct_binary_h)
#define octave_ls_oct_binary_h 1

#include "octave-config.h"

#include <iosfwd>

#include "mach-info.h"

class octave_value;

extern OCTINTERP_API bool
save_binary_data (std::ostream& os, const octave_value& tc,
                  const std::string& name, const std::string& doc,
                  bool mark_global, bool save_as_floats);

extern OCTINTERP_API std::string
read_binary_data (std::istream& is, bool swap,
                  octave::mach_info::float_format fmt,
                  const std::string& filename, bool& global,
                  octave_value& tc, std::string& doc);

#endif
