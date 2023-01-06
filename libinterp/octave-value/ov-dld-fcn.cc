////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2023 The Octave Project Developers
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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "file-ops.h"
#include "oct-shlib.h"

#include "defaults.h"
#include "dynamic-ld.h"
#include "error.h"
#include "interpreter-private.h"
#include "ovl.h"
#include "ov-dld-fcn.h"
#include "ov.h"


DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_dld_function,
                                     "dynamically-linked function",
                                     "dynamically-linked function");

octave_dld_function::octave_dld_function
(octave_builtin::fcn ff, const octave::dynamic_library& shl,
 const std::string& nm, const std::string& ds)
  : octave_builtin (ff, nm, ds), m_sh_lib (shl)
{
  mark_fcn_file_up_to_date (time_parsed ());

  std::string file_name = fcn_file_name ();

  static const std::string canonical_oct_file_dir
    = octave::sys::canonicalize_file_name (octave::config::oct_file_dir ());
  static const std::string oct_file_dir
    = canonical_oct_file_dir.empty () ? octave::config::oct_file_dir ()
      : canonical_oct_file_dir;

  m_system_fcn_file
    = (! file_name.empty ()
       && oct_file_dir == file_name.substr (0, oct_file_dir.length ()));
}

octave_dld_function::octave_dld_function
(octave_builtin::meth mm, const octave::dynamic_library& shl,
 const std::string& nm, const std::string& ds)
  : octave_builtin (mm, nm, ds), m_sh_lib (shl)
{
  mark_fcn_file_up_to_date (time_parsed ());

  std::string file_name = fcn_file_name ();

  static const std::string canonical_oct_file_dir
    = octave::sys::canonicalize_file_name (octave::config::oct_file_dir ());
  static const std::string oct_file_dir
    = canonical_oct_file_dir.empty () ? octave::config::oct_file_dir ()
      : canonical_oct_file_dir;

  m_system_fcn_file
    = (! file_name.empty ()
       && oct_file_dir == file_name.substr (0, oct_file_dir.length ()));
}

octave_dld_function::~octave_dld_function (void)
{
  octave::dynamic_loader& dyn_loader = octave::__get_dynamic_loader__ ();

  dyn_loader.remove_oct (m_name, m_sh_lib);
}

std::string
octave_dld_function::fcn_file_name (void) const
{
  return m_sh_lib.file_name ();
}

octave::sys::time
octave_dld_function::time_parsed (void) const
{
  return m_sh_lib.time_loaded ();
}

// Note: this wrapper around the octave_dld_function constructor is
//       necessary to work around a MSVC limitation handling in
//       virtual destructors that prevents unloading a dynamic module
//       before *all* objects (of class using a virtual dtor) have
//       been fully deleted; indeed, MSVC attaches auto-generated code
//       (scalar deleting destructor) to objects created in a dynamic
//       module, and this code will be executed in the dynamic module
//       context at object deletion; unloading the dynamic module
//       before objects have been deleted will make the "delete" code
//       of objects to point to an invalid code segment.

octave_dld_function *
octave_dld_function::create (octave_builtin::fcn ff,
                             const octave::dynamic_library& shl,
                             const std::string& nm, const std::string& ds)
{
  return new octave_dld_function (ff, shl, nm, ds);
}

octave_dld_function *
octave_dld_function::create (octave_builtin::meth mm,
                             const octave::dynamic_library& shl,
                             const std::string& nm, const std::string& ds)
{
  return new octave_dld_function (mm, shl, nm, ds);
}
