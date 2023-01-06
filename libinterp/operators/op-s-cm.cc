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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "mx-s-cm.h"
#include "mx-cm-s.h"
#include "mx-s-cnda.h"
#include "mx-cnda-s.h"

#include "ovl.h"
#include "ov.h"
#include "ov-scalar.h"
#include "ov-float.h"
#include "ov-cx-mat.h"
#include "ov-flt-cx-mat.h"
#include "ov-re-mat.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

OCTAVE_BEGIN_NAMESPACE(octave)

// scalar by complex matrix ops.

DEFNDBINOP_OP (add, scalar, complex_matrix, scalar, complex_array, +)
DEFNDBINOP_OP (sub, scalar, complex_matrix, scalar, complex_array, -)
DEFNDBINOP_OP (mul, scalar, complex_matrix, scalar, complex_array, *)

DEFBINOP (div, scalar, complex_matrix)
{
  const octave_scalar& v1 = dynamic_cast<const octave_scalar&> (a1);
  const octave_complex_matrix& v2
    = dynamic_cast<const octave_complex_matrix&> (a2);

  Matrix m1 = v1.matrix_value ();
  ComplexMatrix m2 = v2.complex_matrix_value ();
  MatrixType typ = v2.matrix_type ();

  ComplexMatrix ret = xdiv (m1, m2, typ);

  v2.matrix_type (typ);
  return ret;
}

DEFBINOP_FN (pow, scalar, complex_matrix, xpow)

DEFBINOP (ldiv, scalar, complex_matrix)
{
  const octave_scalar& v1 = dynamic_cast<const octave_scalar&> (a1);
  const octave_complex_matrix& v2
    = dynamic_cast<const octave_complex_matrix&> (a2);

  return octave_value (v2.complex_array_value () / v1.double_value ());
}

DEFNDCMPLXCMPOP_FN (lt, scalar, complex_matrix, scalar, complex_array, mx_el_lt)
DEFNDCMPLXCMPOP_FN (le, scalar, complex_matrix, scalar, complex_array, mx_el_le)
DEFNDCMPLXCMPOP_FN (eq, scalar, complex_matrix, scalar, complex_array, mx_el_eq)
DEFNDCMPLXCMPOP_FN (ge, scalar, complex_matrix, scalar, complex_array, mx_el_ge)
DEFNDCMPLXCMPOP_FN (gt, scalar, complex_matrix, scalar, complex_array, mx_el_gt)
DEFNDCMPLXCMPOP_FN (ne, scalar, complex_matrix, scalar, complex_array, mx_el_ne)

DEFNDBINOP_OP (el_mul, scalar, complex_matrix, scalar, complex_array, *)
DEFNDBINOP_FN (el_div, scalar, complex_matrix, scalar, complex_array, elem_xdiv)
DEFNDBINOP_FN (el_pow, scalar, complex_matrix, scalar, complex_array, elem_xpow)

DEFBINOP (el_ldiv, scalar, complex_matrix)
{
  const octave_scalar& v1 = dynamic_cast<const octave_scalar&> (a1);
  const octave_complex_matrix& v2
    = dynamic_cast<const octave_complex_matrix&> (a2);

  return octave_value (v2.complex_array_value () / v1.double_value ());
}

DEFNDBINOP_FN (el_and, scalar, complex_matrix, scalar, complex_array, mx_el_and)
DEFNDBINOP_FN (el_or,  scalar, complex_matrix, scalar, complex_array, mx_el_or)

DEFNDCATOP_FN (s_cm, scalar, complex_matrix, array, complex_array, concat)

DEFCONV (complex_matrix_conv, scalar, complex_matrix)
{
  const octave_scalar& v = dynamic_cast<const octave_scalar&> (a);

  return new octave_complex_matrix (ComplexMatrix (v.matrix_value ()));
}

void
install_s_cm_ops (octave::type_info& ti)
{
  INSTALL_BINOP_TI (ti, op_add, octave_scalar, octave_complex_matrix, add);
  INSTALL_BINOP_TI (ti, op_sub, octave_scalar, octave_complex_matrix, sub);
  INSTALL_BINOP_TI (ti, op_mul, octave_scalar, octave_complex_matrix, mul);
  INSTALL_BINOP_TI (ti, op_div, octave_scalar, octave_complex_matrix, div);
  INSTALL_BINOP_TI (ti, op_pow, octave_scalar, octave_complex_matrix, pow);
  INSTALL_BINOP_TI (ti, op_ldiv, octave_scalar, octave_complex_matrix, ldiv);
  INSTALL_BINOP_TI (ti, op_lt, octave_scalar, octave_complex_matrix, lt);
  INSTALL_BINOP_TI (ti, op_le, octave_scalar, octave_complex_matrix, le);
  INSTALL_BINOP_TI (ti, op_eq, octave_scalar, octave_complex_matrix, eq);
  INSTALL_BINOP_TI (ti, op_ge, octave_scalar, octave_complex_matrix, ge);
  INSTALL_BINOP_TI (ti, op_gt, octave_scalar, octave_complex_matrix, gt);
  INSTALL_BINOP_TI (ti, op_ne, octave_scalar, octave_complex_matrix, ne);
  INSTALL_BINOP_TI (ti, op_el_mul, octave_scalar, octave_complex_matrix, el_mul);
  INSTALL_BINOP_TI (ti, op_el_div, octave_scalar, octave_complex_matrix, el_div);
  INSTALL_BINOP_TI (ti, op_el_pow, octave_scalar, octave_complex_matrix, el_pow);
  INSTALL_BINOP_TI (ti, op_el_ldiv, octave_scalar, octave_complex_matrix,
                    el_ldiv);
  INSTALL_BINOP_TI (ti, op_el_and, octave_scalar, octave_complex_matrix, el_and);
  INSTALL_BINOP_TI (ti, op_el_or, octave_scalar, octave_complex_matrix, el_or);

  INSTALL_CATOP_TI (ti, octave_scalar, octave_complex_matrix, s_cm);

  INSTALL_ASSIGNCONV_TI (ti, octave_scalar, octave_complex_matrix,
                         octave_complex_matrix);
  INSTALL_ASSIGNCONV_TI (ti, octave_float_scalar, octave_complex_matrix,
                         octave_float_complex_matrix);

  INSTALL_WIDENOP_TI (ti, octave_scalar, octave_complex_matrix,
                      complex_matrix_conv);
}

OCTAVE_END_NAMESPACE(octave)
