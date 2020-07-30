////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1998-2020 The Octave Project Developers
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

#include "ovl.h"
#include "ov.h"
#include "ov-typeinfo.h"
#include "ov-complex.h"
#include "ops.h"
#include "xpow.h"

#include "sparse-xpow.h"
#include "sparse-xdiv.h"
#include "ov-re-sparse.h"
#include "ov-cx-sparse.h"
#include "smx-cs-sm.h"
#include "smx-sm-cs.h"

// complex by sparse matrix ops.

DEFBINOP_OP (add, complex, sparse_matrix, +)
DEFBINOP_OP (sub, complex, sparse_matrix, -)
DEFBINOP_OP (mul, complex, sparse_matrix, *)

DEFBINOP (div, complex, sparse_matrix)
{
  const octave_complex& v1 = dynamic_cast<const octave_complex&> (a1);
  const octave_sparse_matrix& v2 = dynamic_cast<const octave_sparse_matrix&> (a2);

  if (v2.rows () == 1 && v2.columns () == 1)
    return octave_value (SparseComplexMatrix (1, 1, v1.complex_value () / v2.scalar_value ()));
  else
    {
      MatrixType typ = v2.matrix_type ();
      ComplexMatrix m1 = ComplexMatrix (1, 1, v1.complex_value ());
      SparseMatrix m2 = v2.sparse_matrix_value ();
      ComplexMatrix ret = xdiv (m1, m2, typ);
      v2.matrix_type (typ);
      return ret;
    }
}

DEFBINOP (pow, complex, sparse_matrix)
{
  const octave_complex& v1 = dynamic_cast<const octave_complex&> (a1);
  const octave_sparse_matrix& v2 = dynamic_cast<const octave_sparse_matrix&> (a2);
  return xpow (v1.complex_value (), v2.matrix_value ());
}

DEFBINOP (ldiv, complex, sparse_matrix)
{
  const octave_complex& v1 = dynamic_cast<const octave_complex&> (a1);
  const octave_sparse_matrix& v2 = dynamic_cast<const octave_sparse_matrix&> (a2);

  return octave_value (v2.sparse_matrix_value () / v1.complex_value ());
}

DEFBINOP_FN (lt, complex, sparse_matrix, mx_el_lt)
DEFBINOP_FN (le, complex, sparse_matrix, mx_el_le)
DEFBINOP_FN (eq, complex, sparse_matrix, mx_el_eq)
DEFBINOP_FN (ge, complex, sparse_matrix, mx_el_ge)
DEFBINOP_FN (gt, complex, sparse_matrix, mx_el_gt)
DEFBINOP_FN (ne, complex, sparse_matrix, mx_el_ne)

DEFBINOP_OP (el_mul, complex, sparse_matrix, *)
DEFBINOP_FN (el_div, complex, sparse_matrix, x_el_div)
DEFBINOP_FN (el_pow, complex, sparse_matrix, elem_xpow)

DEFBINOP (el_ldiv, complex, sparse_matrix)
{
  const octave_complex& v1 = dynamic_cast<const octave_complex&> (a1);
  const octave_sparse_matrix& v2 = dynamic_cast<const octave_sparse_matrix&> (a2);

  return octave_value (v2.sparse_matrix_value () / v1.complex_value ());
}

DEFBINOP_FN (el_and, complex, sparse_matrix, mx_el_and)
DEFBINOP_FN (el_or,  complex, sparse_matrix, mx_el_or)

DEFCATOP (cs_sm, sparse_matrix, complex)
{
  const octave_complex& v1 = dynamic_cast<const octave_complex&> (a1);
  const octave_sparse_matrix& v2
    = dynamic_cast<const octave_sparse_matrix&> (a2);
  SparseComplexMatrix tmp (1, 1, v1.complex_value ());
  return octave_value (tmp. concat (v2.sparse_matrix_value (), ra_idx));
}

DEFCONV (sparse_matrix_conv, complex, sparse_matrix)
{
  const octave_complex& v = dynamic_cast<const octave_complex&> (a);

  return new octave_sparse_matrix (SparseMatrix (v.matrix_value ()));
}

void
install_cs_sm_ops (octave::type_info& ti)
{
  INSTALL_BINOP_TI (ti, op_add, octave_complex, octave_sparse_matrix, add);
  INSTALL_BINOP_TI (ti, op_sub, octave_complex, octave_sparse_matrix, sub);
  INSTALL_BINOP_TI (ti, op_mul, octave_complex, octave_sparse_matrix, mul);
  INSTALL_BINOP_TI (ti, op_div, octave_complex, octave_sparse_matrix, div);
  INSTALL_BINOP_TI (ti, op_pow, octave_complex, octave_sparse_matrix, pow);
  INSTALL_BINOP_TI (ti, op_ldiv, octave_complex, octave_sparse_matrix, ldiv);
  INSTALL_BINOP_TI (ti, op_lt, octave_complex, octave_sparse_matrix, lt);
  INSTALL_BINOP_TI (ti, op_le, octave_complex, octave_sparse_matrix, le);
  INSTALL_BINOP_TI (ti, op_eq, octave_complex, octave_sparse_matrix, eq);
  INSTALL_BINOP_TI (ti, op_ge, octave_complex, octave_sparse_matrix, ge);
  INSTALL_BINOP_TI (ti, op_gt, octave_complex, octave_sparse_matrix, gt);
  INSTALL_BINOP_TI (ti, op_ne, octave_complex, octave_sparse_matrix, ne);
  INSTALL_BINOP_TI (ti, op_el_mul, octave_complex, octave_sparse_matrix, el_mul);
  INSTALL_BINOP_TI (ti, op_el_div, octave_complex, octave_sparse_matrix, el_div);
  INSTALL_BINOP_TI (ti, op_el_pow, octave_complex, octave_sparse_matrix, el_pow);
  INSTALL_BINOP_TI (ti, op_el_ldiv, octave_complex, octave_sparse_matrix,
                    el_ldiv);
  INSTALL_BINOP_TI (ti, op_el_and, octave_complex, octave_sparse_matrix, el_and);
  INSTALL_BINOP_TI (ti, op_el_or, octave_complex, octave_sparse_matrix, el_or);

  INSTALL_CATOP_TI (ti, octave_complex, octave_sparse_matrix, cs_sm);

  INSTALL_ASSIGNCONV_TI (ti, octave_complex, octave_sparse_matrix,
                         octave_complex_matrix);

  INSTALL_WIDENOP_TI (ti, octave_complex, octave_sparse_matrix,
                      sparse_matrix_conv);
}
