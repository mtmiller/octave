// f-ifft2.cc                                           -*- C++ -*-
/*

Copyright (C) 1994 John W. Eaton

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
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dMatrix.h"
#include "CMatrix.h"

#include "tree-const.h"
#include "user-prefs.h"
#include "gripes.h"
#include "error.h"
#include "utils.h"
#include "help.h"
#include "defun-dld.h"

// This function should be merged with Ffft2.

DEFUN_DLD ("ifft2", Fifft2, Sifft2, 3, 1,
  "ifft2 (X [, N] [, M])\n\
\n\
two dimensional inverse fast fourier transform of a vector") 
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin < 2 || nargin > 4)
    {
      print_usage ("ifft2");
      return retval;
    }

  tree_constant arg = args(1);

  int n_rows = arg.rows ();
  if (nargin > 2)
    n_rows = NINT (args(2).double_value ());

  if (error_state)
    return retval;

  int n_cols = arg.columns ();
  if (nargin > 3)
    n_cols = NINT (args(3).double_value ());

  if (error_state)
    return retval;

  if (n_rows < 0 || n_cols < 0)
    {
      error ("ifft2: number of points must be greater than zero");
      return retval;
    }

  int arg_is_empty = empty_arg ("ifft2", arg.rows (), arg.columns ());

  if (arg_is_empty < 0)
    return retval;
  else if (arg_is_empty || n_rows == 0 || n_cols == 0)
    return Matrix ();

  if (arg.is_real_type ())
    {
      Matrix m = arg.matrix_value ();

      if (! error_state)
	{
	  m.resize (n_rows, n_cols, 0.0);
	  retval = m.ifourier2d ();
	}
    }
  else if (arg.is_complex_type ())
    {
      ComplexMatrix m = arg.complex_matrix_value ();

      if (! error_state)
	{
	  m.resize (n_rows, n_cols, 0.0);
	  retval = m.ifourier2d ();
	}
    }
  else
    {
      gripe_wrong_type_arg ("ifft2", arg);
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
