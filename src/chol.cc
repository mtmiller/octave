// f-chol.cc                                           -*- C++ -*-
/*

Copyright (C) 1993, 1994 John W. Eaton

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

#include "dbleCHOL.h"
#include "CmplxCHOL.h"

#include "tree-const.h"
#include "user-prefs.h"
#include "gripes.h"
#include "error.h"
#include "help.h"
#include "defun-dld.h"

DEFUN_DLD ("chol", Fchol, Schol, 2, 1,
  "R = chol (X): cholesky factorization")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin != 2 || nargout > 1)
    {
      print_usage ("chol");
      return retval;
    }

  tree_constant tmp = args(1).make_numeric ();
    
  int nr = tmp.rows ();
  int nc = tmp.columns ();

  if (nr == 0 || nc == 0)
    {
      int flag = user_pref.propagate_empty_matrices;
      if (flag != 0)
	{
	  if (flag < 0)
	    gripe_empty_arg ("chol", 0);

	  retval.resize (1, Matrix ());
	}
      else
	gripe_empty_arg ("chol", 1);

      return retval;
    }

  if (tmp.is_real_matrix ())
    {
      Matrix m = tmp.matrix_value ();
      int info;
      CHOL fact (m, info);
      if (info != 0)
	error ("chol: matrix not positive definite");
      else
	retval = fact.chol_matrix ();
    }
  else if (tmp.is_complex_matrix ())
    {
      ComplexMatrix m = tmp.complex_matrix_value ();
      int info;
      ComplexCHOL fact (m, info);
      if (info != 0)
	error ("chol: matrix not positive definite");
      else
	retval = fact.chol_matrix ();
    }
  else if (tmp.is_real_scalar ())
    {
      double d = tmp.double_value ();
      retval = d;
    }
  else if (tmp.is_complex_scalar ())
    {
      Complex c = tmp.complex_value ();
      retval = c;
    }
  else
    {
      gripe_wrong_type_arg ("chol", tmp);
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/

