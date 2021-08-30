////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2021 The Octave Project Developers
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

#include <cerrno>
#include <cstring>

#include "oct-uname.h"
#include "uname-wrapper.h"

namespace octave
{
  namespace sys
  {
    void
    uname::init (void)
    {
      char *sysname, *nodename, *release, *version, *machine;

      m_err = octave_uname_wrapper (&sysname, &nodename, &release,
                                    &version, &machine);

      if (m_err < 0)
        m_msg = std::strerror (errno);
      else
        {
          m_sysname = sysname;
          m_nodename = nodename;
          m_release = release;
          m_version = version;
          m_machine = machine;
          m_msg = "";
        }
    }
  }
}
