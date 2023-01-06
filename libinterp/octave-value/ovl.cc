////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2023 The Octave Project Developers
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

#include "error.h"
#include "ovl.h"

// We are likely to have a lot of octave_value_list objects to allocate,
// so make the grow_size large.

octave_value_list::octave_value_list (const std::list<octave_value>& lst)
{
  std::size_t nel = lst.size ();

  if (nel > 0)
    {
      m_data.resize (nel);
      octave_idx_type k = 0;
      for (const auto& ov : lst)
        m_data[k++] = ov;
    }
}

octave_value_list::octave_value_list (const std::list<octave_value_list>& lst)
{
  octave_idx_type n = 0;
  octave_idx_type nel = 0;

  // Determine number.
  for (const auto& ovl : lst)
    {
      n++;
      nel += ovl.length ();
    }

  // Optimize single-element case
  if (n == 1)
    m_data = lst.front ().m_data;
  else if (nel > 0)
    {
      m_data.resize (nel);
      octave_idx_type k = 0;
      for (const auto& ovl : lst)
        {
          for (octave_idx_type i = 0; i < ovl.length (); i++)
            m_data[k++] = ovl(i);
        }

      panic_unless (k == nel);
    }

}

octave_value_list&
octave_value_list::prepend (const octave_value& val)
{
  octave_idx_type n = length ();

  resize (n + 1);

  while (n > 0)
    {
      elem (n) = elem (n - 1);
      n--;
    }

  elem (0) = val;

  return *this;
}

octave_value_list&
octave_value_list::append (const octave_value& val)
{
  octave_idx_type n = length ();

  resize (n + 1);

  elem (n) = val;

  return *this;
}

octave_value_list&
octave_value_list::append (const octave_value_list& lst)
{
  octave_idx_type len = length ();
  octave_idx_type lst_len = lst.length ();

  resize (len + lst_len);

  for (octave_idx_type i = 0; i < lst_len; i++)
    elem (len + i) = lst (i);

  return *this;
}

octave_value_list&
octave_value_list::reverse (void)
{
  octave_idx_type n = length ();

  for (octave_idx_type i = 0; i < n / 2; i++)
    {
      octave_value tmp = elem (i);
      elem (i) = elem (n - i - 1);
      elem (n - i - 1) = tmp;
    }

  return *this;
}

octave_value_list
octave_value_list::splice (octave_idx_type offset, octave_idx_type rep_length,
                           const octave_value_list& lst) const
{
  octave_value_list retval;

  octave_idx_type len = length ();

  if (offset < 0 || offset >= len)
    {
      if (! (rep_length == 0 && offset == len))
        error ("octave_value_list::splice: invalid OFFSET");
    }

  if (rep_length < 0 || rep_length + offset > len)
    error ("octave_value_list::splice: invalid LENGTH");

  octave_idx_type lst_len = lst.length ();

  octave_idx_type new_len = len - rep_length + lst_len;

  retval.resize (new_len);

  octave_idx_type k = 0;

  for (octave_idx_type i = 0; i < offset; i++)
    retval(k++) = elem (i);

  for (octave_idx_type i = 0; i < lst_len; i++)
    retval(k++) = lst (i);

  for (octave_idx_type i = offset + rep_length; i < len; i++)
    retval(k++) = elem (i);

  return retval;
}

bool
octave_value_list::all_strings_p (void) const
{
  octave_idx_type n = length ();

  for (octave_idx_type i = 0; i < n; i++)
    if (! elem(i).is_string ())
      return false;

  return true;
}

bool
octave_value_list::all_scalars (void) const
{
  octave_idx_type n = length ();

  for (octave_idx_type i = 0; i < n; i++)
    {
      dim_vector dv = elem(i).dims ();
      if (! dv.all_ones ())
        return false;
    }

  return true;
}

bool
octave_value_list::any_cell (void) const
{
  octave_idx_type n = length ();

  for (octave_idx_type i = 0; i < n; i++)
    if (elem (i).iscell ())
      return true;

  return false;
}

bool
octave_value_list::has_magic_colon (void) const
{
  octave_idx_type n = length ();

  for (octave_idx_type i = 0; i < n; i++)
    if (elem(i).is_magic_colon ())
      return true;

  return false;
}

string_vector
octave_value_list::make_argv (const std::string& fcn_name) const
{
  string_vector argv;

  if (! all_strings_p ())
    error ("%s: all arguments must be strings", fcn_name.c_str ());

  octave_idx_type len = length ();

  octave_idx_type total_nr = 0;

  for (octave_idx_type i = 0; i < len; i++)
    {
      // An empty std::string ("") has zero columns and zero rows
      // (a change that was made for Matlab contemptibility.

      octave_idx_type n = elem (i).rows ();

      total_nr += n ? n : 1;
    }

  octave_idx_type k = 0;
  if (! fcn_name.empty ())
    {
      argv.resize (total_nr+1);
      argv[0] = fcn_name;
      k = 1;
    }
  else
    argv.resize (total_nr);

  for (octave_idx_type i = 0; i < len; i++)
    {
      octave_idx_type nr = elem (i).rows ();

      if (nr < 2)
        argv[k++] = elem (i).string_value ();
      else
        {
          string_vector tmp = elem (i).string_vector_value ();

          for (octave_idx_type j = 0; j < nr; j++)
            argv[k++] = tmp[j];
        }
    }

  return argv;
}

void
octave_value_list::make_storable_values (void)
{
  octave_idx_type len = length ();
  const std::vector<octave_value>& cdata = m_data;

  for (octave_idx_type i = 0; i < len; i++)
    {
      // This is optimized so that we don't force a copy unless necessary.
      octave_value tmp = cdata[i].storable_value ();
      if (! tmp.is_copy_of (cdata[i]))
        m_data[i] = tmp;
    }
}
