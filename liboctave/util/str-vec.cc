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

/*

The function string_vector::list_in_columns was adapted from a similar
function distributed in the GNU file utilities, copyright (C) 85, 88,
90, 91, 95, 1996 Free Software Foundation, Inc.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <ostream>
#include <string>

#include "cmd-edit.h"
#include "lo-utils.h"
#include "str-vec.h"

// Create a string vector from a NULL terminated list of C strings.

string_vector::string_vector (const char * const *s)
  : m_data ()
{
  octave_idx_type n = 0;

  if (s)
    {
      const char * const *t = s;

      while (*t++)
        n++;
    }

  resize (n);

  for (octave_idx_type i = 0; i < n; i++)
    elem (i) = s[i];
}

// Create a string vector from up to N C strings.  Assumes that N is
// nonnegative.

string_vector::string_vector (const char * const *s, octave_idx_type n)
  : m_data (dim_vector (n, 1))
{
  for (octave_idx_type i = 0; i < n; i++)
    elem (i) = s[i];
}

string_vector&
string_vector::sort (bool make_uniq)
{
  // Don't use Array<std::string>::sort () to allow sorting in place.
  octave_sort<std::string> lsort;
  lsort.sort (m_data.fortran_vec (), numel ());

  if (make_uniq)
    uniq ();

  return *this;
}
string_vector&
string_vector::uniq (void)
{
  octave_idx_type len = numel ();

  if (len > 0)
    {
      octave_idx_type k = 0;

      for (octave_idx_type i = 1; i < len; i++)
        if (elem (i) != elem (k))
          if (++k != i)
            elem (k) = elem (i);

      if (len != ++k)
        resize (k);
    }

  return *this;
}

string_vector&
string_vector::append (const std::string& s)
{
  octave_idx_type len = numel ();

  resize (len + 1);

  elem (len) = s;

  return *this;
}

string_vector&
string_vector::append (const string_vector& sv)
{
  octave_idx_type len = numel ();
  octave_idx_type sv_len = sv.numel ();
  octave_idx_type new_len = len + sv_len;

  resize (new_len);

  for (octave_idx_type i = 0; i < sv_len; i++)
    elem (len + i) = sv[i];

  return *this;
}

std::string
string_vector::join (const std::string& sep) const
{
  std::string retval;

  octave_idx_type len = numel ();

  if (len > 0)
    {
      octave_idx_type i;

      for (i = 0; i < len - 1; i++)
        retval += elem (i) + sep;

      retval += elem (i);
    }

  return retval;
}

char **
string_vector::c_str_vec (void) const
{
  octave_idx_type len = numel ();

  char **retval = new char * [len + 1];

  retval[len] = nullptr;

  for (octave_idx_type i = 0; i < len; i++)
    retval[i] = octave::strsave (elem (i).c_str ());

  return retval;
}

std::list<std::string>
string_vector::std_list (void) const
{
  octave_idx_type len = numel ();

  std::list<std::string> retval;

  for (octave_idx_type i = 0; i < len; i++)
    retval.push_back (elem (i));

  return retval;
}

void
string_vector::delete_c_str_vec (const char * const *v)
{
  if (! v)
    return;

  const char * const *p = v;

  while (*p)
    delete [] *p++;

  delete [] v;
}

// Format a list in neat columns.

std::ostream&
string_vector::list_in_columns (std::ostream& os, int width,
                                const std::string& prefix) const
{
  // Compute the maximum name length.

  octave_idx_type max_name_length = 0;
  octave_idx_type total_names = numel ();

  if (total_names == 0)
    {
      // List empty, remember to end output with a newline.

      os << "\n";
      return os;
    }

  for (octave_idx_type i = 0; i < total_names; i++)
    {
      octave_idx_type name_length = elem (i).length ();
      if (name_length > max_name_length)
        max_name_length = name_length;
    }

  // Allow at least two spaces between names.

  max_name_length += 2;

  // Calculate the maximum number of columns that will fit.

  octave_idx_type line_length
    = ((width <= 0 ? octave::command_editor::terminal_cols () : width)
       - prefix.length ());

  octave_idx_type nc = line_length / max_name_length;
  if (nc == 0)
    nc = 1;

  // Calculate the number of rows that will be in each column except
  // possibly for a short column on the right.

  octave_idx_type nr = total_names / nc + (total_names % nc != 0);

  octave_idx_type count;
  for (octave_idx_type row = 0; row < nr; row++)
    {
      count = row;

      // Print the next row.

      os << prefix;

      while (1)
        {
          std::string nm = elem (count);

          os << nm;
          octave_idx_type name_length = nm.length ();

          count += nr;
          if (count >= total_names)
            break;

          octave_idx_type spaces_to_pad = max_name_length - name_length;
          for (octave_idx_type i = 0; i < spaces_to_pad; i++)
            os << ' ';
        }
      os << "\n";
    }

  return os;
}
