////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2021 The Octave Project Developers
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

#if ! defined (octave_variables_h)
#define octave_variables_h 1

#include "octave-config.h"

class octave_function;
class octave_user_function;

class octave_value;
class octave_value_list;
class octave_builtin;
class string_vector;

namespace octave
{
  class tree_identifier;
}

#include <limits>
#include <string>

#include "lo-ieee.h"

#include "ov-builtin.h"

extern OCTINTERP_API octave_function *
is_valid_function (const octave_value&, const std::string& = "",
                   bool warn = false);

extern OCTINTERP_API octave_function *
is_valid_function (const std::string&, const std::string& = "",
                   bool warn = false);

#if defined (OCTAVE_PROVIDE_DEPRECATED_SYMBOLS)
OCTAVE_DEPRECATED (6, "use 'octave::get_function_handle' instead")
extern OCTINTERP_API octave_function *
extract_function (const octave_value& arg, const std::string& warn_for,
                  const std::string& fname, const std::string& header,
                  const std::string& trailer);
#endif

extern OCTINTERP_API int
symbol_exist (const std::string& name, const std::string& type = "any");

extern OCTINTERP_API std::string
unique_symbol_name (const std::string& basename);

extern OCTINTERP_API octave_value
set_internal_variable (bool& var, const octave_value_list& args,
                       int nargout, const char *nm);

extern OCTINTERP_API octave_value
set_internal_variable (char& var, const octave_value_list& args,
                       int nargout, const char *nm);

extern OCTINTERP_API octave_value
set_internal_variable (int& var, const octave_value_list& args,
                       int nargout, const char *nm,
                       int minval = std::numeric_limits<int>::min (),
                       int maxval = std::numeric_limits<int>::max ());

extern OCTINTERP_API octave_value
set_internal_variable (double& var, const octave_value_list& args,
                       int nargout, const char *nm,
                       double minval = -octave::numeric_limits<double>::Inf (),
                       double maxval = octave::numeric_limits<double>::Inf ());

extern OCTINTERP_API octave_value
set_internal_variable (std::string& var, const octave_value_list& args,
                       int nargout, const char *nm, bool empty_ok = true);

extern OCTINTERP_API octave_value
set_internal_variable (std::string& var, const octave_value_list& args,
                       int nargout, const char *nm, const char **choices);

extern OCTINTERP_API octave_value
set_internal_variable (int& var, const octave_value_list& args,
                       int nargout, const char *nm, const char **choices);

#define SET_INTERNAL_VARIABLE(NM)                       \
  set_internal_variable (V ## NM, args, nargout, #NM)

#define SET_NONEMPTY_INTERNAL_STRING_VARIABLE(NM)               \
  set_internal_variable (V ## NM, args, nargout, #NM, false)

#define SET_INTERNAL_VARIABLE_WITH_LIMITS(NM, MINVAL, MAXVAL)           \
  set_internal_variable (V ## NM, args, nargout, #NM, MINVAL, MAXVAL)

// in the following, CHOICES must be a C string array terminated by null.
#define SET_INTERNAL_VARIABLE_CHOICES(NM, CHOICES)              \
  set_internal_variable (V ## NM, args, nargout, #NM, CHOICES)

extern OCTINTERP_API std::string
maybe_missing_function_hook (const std::string& name);

#endif
