// strfns.cc                                           -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cctype>

#include "dMatrix.h"

#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "help.h"
#include "oct-obj.h"
#include "tree-const.h"
#include "utils.h"

DEFUN ("isstr", Fisstr, Sisstr, 1, 1,
  "isstr (X): return 1 if X is a string, 0 otherwise")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = (double) args(0).is_string ();
  else
    print_usage ("isstr");

  return retval;
}

DEFUN ("setstr", Fsetstr, Ssetstr, 1, 1,
  "setstr (V): convert a vector to a string")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = args(0).convert_to_str ();
  else
    print_usage ("setstr");

  return retval;
}

DEFUN ("toascii", Ftoascii, Stoascii, 1, 1,
  "toascii (STRING): return ASCII representation of STRING in a matrix")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      tree_constant arg = args(0);

      if (arg.is_string ())
	{
	  Octave_str_obj str = args(0).all_strings ();

	  int nr = str.num_strings ();
	  int nc = str.max_length ();

	  // XXX FIXME XXX -- should fill with user-specified value.

	  Matrix m (nr, nc, 0);

	  for (int i = 0; i < nr; i++)
	    {
	      nc = str.elem (i).length ();
	      for (int j = 0; j < nc; j++)
		m (i, j) = toascii (str.elem (i) [j]);
	    }

	  retval = m;
	}
      else
	gripe_wrong_type_arg ("toascii", arg);
    }
  else
    print_usage ("toascii");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
