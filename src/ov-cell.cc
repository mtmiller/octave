/*

Copyright (C) 1999 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <strstream>

#include "lo-utils.h"

#include "defun.h"
#include "error.h"
#include "ov-cell.h"
#include "oct-obj.h"
#include "unwind-prot.h"
#include "utils.h"
#include "ov-base-mat.h"
#include "ov-base-mat.cc"
#include "ov-re-mat.h"
#include "ov-scalar.h"

template class octave_base_matrix<Cell>;

DEFINE_OCTAVE_ALLOCATOR (octave_cell);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_cell, "cell");

octave_value
octave_cell::subsref (const std::string type,
		      const SLList<octave_value_list>& idx)
{
  octave_value retval;

  switch (type[0])
    {
    case '(':
      retval = do_index_op (idx.front ());
      break;

    case '{':
      {
	octave_value tmp = do_index_op (idx.front ());

	Cell tcell = tmp.cell_value ();

	if (tcell.length () == 1)
	  retval = tcell(0,0);
	else
	  {
	    int nr = tcell.rows ();
	    int nc = tcell.columns ();
	    octave_value_list lst (nr * nc, octave_value ());
	    int k = 0;
	    for (int j = 0; j < nc; j++)
	      for (int i = 0; i < nr; i++)
		lst(k++) = tcell(i,j);
	    retval = lst;
	  }
      }
      break;

    case '.':
      {
	std::string nm = type_name ();
	error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
      }
      break;

    default:
      panic_impossible ();
    }

  return retval.next_subsref (type, idx);
}

octave_value
octave_cell::subsasgn (const std::string type,
		       const SLList<octave_value_list>& idx,
		       const octave_value& rhs)
{
  octave_value retval;

  int n = type.length ();

  octave_value t_rhs = rhs;

  if (n > 1)
    {
      switch (type[0])
	{
	case '(':
	  {
	    octave_value tmp = do_index_op (idx.front (), true);

	    if (! tmp.is_defined ())
	      tmp = octave_value::empty_conv (type.substr (1), rhs);

	    if (! error_state)
	      {
		SLList<octave_value_list> next_idx (idx);

		next_idx.remove_front ();

		t_rhs = tmp.subsasgn (type.substr (1), next_idx, rhs);
	      }
	  }
	  break;

	case '{':
	  {
	    octave_value tmp = do_index_op (idx.front (), true);

	    if (! tmp.is_defined ())
	      tmp = octave_value::empty_conv (type.substr (1), rhs);

	    Cell tcell = tmp.cell_value ();

	    if (! error_state && tcell.length () == 1)
	      {
		tmp = tcell(0,0);

		SLList<octave_value_list> next_idx (idx);

		next_idx.remove_front ();

		t_rhs = tmp.subsasgn (type.substr (1), next_idx, rhs);
	      }
	  }
	  break;

	case '.':
	  {
	    std::string nm = type_name ();
	    error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
	  }
	  break;

	default:
	  panic_impossible ();
	}
    }

  if (! error_state)
    {
      switch (type[0])
	{
	case '(':
	  {
	    octave_value_list i = idx.front ();

	    if (t_rhs.is_cell ())
	      octave_base_matrix<Cell>::assign (i, t_rhs.cell_value ());
	    else
	      octave_base_matrix<Cell>::assign (i, Cell (t_rhs));

	    retval = octave_value (this, count + 1);
	  }
	  break;

	case '{':
	  {
	    octave_value_list i = idx.front ();

	    octave_base_matrix<Cell>::assign (i, Cell (t_rhs));

	    retval = octave_value (this, count + 1);
	  }
	  break;

	case '.':
	  {
	    std::string nm = type_name ();
	    error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
	  }
	  break;

	default:
	  panic_impossible ();
	}
    }

  return retval;
}

void
octave_cell::assign (const octave_value_list& idx, const octave_value& rhs)
{
  if (rhs.is_cell ())
    octave_base_matrix<Cell>::assign (idx, rhs.cell_value ());
  else
    octave_base_matrix<Cell>::assign (idx, Cell (rhs));
}

octave_value_list
octave_cell::list_value (void) const
{
  octave_value_list retval;

  int nr = rows ();
  int nc = columns ();

  if (nr == 1 && nc > 0)
    {
      retval.resize (nc);

      for (int i = 0; i < nc; i++)
	retval(i) = matrix(0,i);
    }
  else if (nc == 1 && nr > 0)
    {
      retval.resize (nr);

      for (int i = 0; i < nr; i++)
	retval(i) = matrix(i,0);
    }
  else
    error ("invalid conversion from cell array to list");

  return retval;
}

void
octave_cell::print (std::ostream& os, bool) const
{
  print_raw (os);
}

void
octave_cell::print_raw (std::ostream& os, bool) const
{
  int nr = rows ();
  int nc = columns ();

  if (nr > 0 && nc > 0)
    {
      indent (os);
      os << "{";
      newline (os);

      increment_indent_level ();

      for (int j = 0; j < nc; j++)
	{
	  for (int i = 0; i < nr; i++)
	    {
	      std::ostrstream buf;
	      buf << "[" << i+1 << "," << j+1 << "]" << std::ends;
	      const char *nm = buf.str ();

	      octave_value val = matrix(i,j);

	      val.print_with_name (os, nm);

	      delete [] nm;
	    }
	}

      decrement_indent_level ();

      indent (os);
      os << "}";
      newline (os);
    }
  else
    os << "{}";
}

bool
octave_cell::print_name_tag (std::ostream& os, const std::string& name) const
{
  indent (os);
  os << name << " =";
  newline (os);
  return false;
}

DEFUN (iscell, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} iscell (@var{x})\n\
Return true if @var{x} is a cell array object.  Otherwise, return\n\
false.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_cell ();
  else
    print_usage ("iscell");

  return retval;
}

DEFUN (cell, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} cell (@var{x})\n\
@deftypefnx {Built-in Function} {} cell (@var{n}, @var{m})\n\
Create a new cell array object.  If invoked with a single scalar\n\
argument, @code{cell} returns a square cell array with the dimension\n\
specified.  If you supply two scalar arguments, @code{cell} takes\n\
them to be the number of rows and columns.  If given a vector with two\n\
elements, @code{cell} uses the values of the elements as the number of\n\
rows and columns, respectively.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  switch (nargin)
    {
    case 1:
      {
	int nr, nc;
	get_dimensions (args(0), "cell", nr, nc);

	if (! error_state)
	  retval = Cell (nr, nc, Matrix ());
      }
      break;

    case 2:
      {
	int nr, nc;
	get_dimensions (args(0), args(1), "cell", nr, nc);

	if (! error_state)
	  retval = Cell (nr, nc, Matrix ());
      }
      break;

    default:
      print_usage ("cell");
      break;
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
