////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2023 The Octave Project Developers
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

#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "ovl.h"
#include "utils.h"

OCTAVE_BEGIN_NAMESPACE(octave)

DEFUN (rcond, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{c} =} rcond (@var{A})
Compute the 1-norm estimate of the reciprocal condition number as returned
by @sc{lapack}.

If the matrix is well-conditioned then @var{c} will be near 1 and if the
matrix is poorly conditioned it will be close to 0.

The matrix @var{A} must not be sparse.  If the matrix is sparse then
@code{condest (@var{A})} or @code{rcond (full (@var{A}))} should be used
instead.
@seealso{cond, condest}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  octave_value retval;

  if (args(0).issparse ())
    error ("rcond: for sparse matrices use 'rcond (full (a))' or 'condest (a)' instead");

  if (args(0).is_single_type ())
    {
      if (args(0).iscomplex ())
        {
          FloatComplexMatrix m = args(0).float_complex_matrix_value ();
          MatrixType mattyp;
          retval = m.rcond (mattyp);
          args(0).matrix_type (mattyp);
        }
      else
        {
          FloatMatrix m = args(0).float_matrix_value ();
          MatrixType mattyp;
          retval = m.rcond (mattyp);
          args(0).matrix_type (mattyp);
        }
    }
  else if (args(0).iscomplex ())
    {
      ComplexMatrix m = args(0).complex_matrix_value ();
      MatrixType mattyp;
      retval = m.rcond (mattyp);
      args(0).matrix_type (mattyp);
    }
  else
    {
      Matrix m = args(0).matrix_value ();
      MatrixType mattyp;
      retval = m.rcond (mattyp);
      args(0).matrix_type (mattyp);
    }

  return retval;
}

/*
%!assert (rcond (eye (2)), 1)
%!assert (rcond (ones (2)), 0)
%!assert (rcond ([1 1; 2 1]), 1/9)
%!assert (rcond (magic (4)), 0, eps)

%!shared x, sx
%! x = [-5.25, -2.25; -2.25, 1] * eps () + ones (2) / 2;
%! sx = [-5.25, -2.25; -2.25, 1] * eps ("single") + ones (2) / 2;
%!assert (rcond (x) < eps ())
%!assert (rcond (sx) < eps ('single'))
%!assert (rcond (x*i) < eps ())
%!assert (rcond (sx*i) < eps ('single'))

*/

OCTAVE_END_NAMESPACE(octave)
