////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2021 The Octave Project Developers
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

#include "error.h"
#include "oct-fstrm.h"

octave::stream
octave_fstream::create (const std::string& nm_arg, std::ios::openmode arg_md,
                        octave::mach_info::float_format ff)
{
  return octave::stream (new octave_fstream (nm_arg, arg_md, ff));
}

octave_fstream::octave_fstream (const std::string& nm_arg,
                                std::ios::openmode arg_md,
                                octave::mach_info::float_format ff)
  : octave::base_stream (arg_md, ff), m_name (nm_arg)
{
  m_fstream.open (m_name.c_str (), arg_md);

  if (! m_fstream)
    // Note: error is inherited from octave::base_stream, not ::error.
    error (std::strerror (errno));
}

// Position a stream at OFFSET relative to ORIGIN.

int
octave_fstream::seek (off_t, int)
{
  // Note: error is inherited from octave::base_stream, not ::error.
  // This error function does not halt execution so "return ..." must exist.
  error ("fseek: invalid_operation");
  return -1;
}

// Return current stream position.

off_t
octave_fstream::tell (void)
{
  // Note: error is inherited from octave::base_stream, not ::error.
  // This error function does not halt execution so "return ..." must exist.
  error ("ftell: invalid_operation");
  return -1;
}

// Return nonzero if EOF has been reached on this stream.

bool
octave_fstream::eof (void) const
{
  return m_fstream.eof ();
}

void
octave_fstream::do_close (void)
{
  m_fstream.close ();
}

std::istream *
octave_fstream::input_stream (void)
{
  std::istream *retval = nullptr;

  if (mode () & std::ios::in)
    retval = &m_fstream;

  return retval;
}

std::ostream *
octave_fstream::output_stream (void)
{
  std::ostream *retval = nullptr;

  if (mode () & std::ios::out)
    retval = &m_fstream;

  return retval;
}
