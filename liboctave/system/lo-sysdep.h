////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2020 The Octave Project Developers
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

#if ! defined (octave_lo_sysdep_h)
#define octave_lo_sysdep_h 1

#include "octave-config.h"

#include <fstream>
#include <string>

#include <sys/types.h>

#include "lo-ieee.h"

class string_vector;

namespace octave
{
  namespace sys
  {
    extern std::string getcwd (void);

    extern int chdir (const std::string&);

    extern bool get_dirlist (const std::string& dirname, string_vector& dirlist,
                             std::string& msg);

    extern std::FILE * fopen (const std::string& name, const std::string& mode);

    extern std::fstream fstream (const std::string& name,
                                 const std::ios::openmode mode =
                                   std::ios::in | std::ios::out);

    extern std::ifstream ifstream (const std::string& name,
                                   const std::ios::openmode mode = std::ios::in);

    extern std::ofstream ofstream (const std::string& name,
                                   const std::ios::openmode mode = std::ios::out);

    extern void putenv_wrapper (const std::string& name,
                                const std::string& value);

    extern std::string getenv_wrapper (const std::string&);

    extern int unsetenv_wrapper (const std::string&);

    extern std::wstring u8_to_wstring (const std::string&);

    extern std::string u8_from_wstring (const std::wstring&);

    extern std::string get_ASCII_filename (const std::string& long_file_name);
  }
}

#endif
