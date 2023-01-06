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

#include "ovl.h"
#include "ov.h"
#include "ov-scalar.h"
#include "ov-float.h"
#include "ov-re-mat.h"
#include "ov-flt-re-mat.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

OCTAVE_BEGIN_NAMESPACE(octave)

// scalar by matrix ops.

DEFNDBINOP_OP (add, float_scalar, float_matrix, float_scalar, float_array, +)
DEFNDBINOP_OP (sub, float_scalar, float_matrix, float_scalar, float_array, -)
DEFNDBINOP_OP (mul, float_scalar, float_matrix, float_scalar, float_array, *)

DEFBINOP (div, float_scalar, float_matrix)
{
  const octave_float_scalar& v1 = dynamic_cast<const octave_float_scalar&> (a1);
  const octave_float_matrix& v2 = dynamic_cast<const octave_float_matrix&> (a2);

  FloatMatrix m1 = v1.float_matrix_value ();
  FloatMatrix m2 = v2.float_matrix_value ();
  MatrixType typ = v2.matrix_type ();

  FloatMatrix ret = xdiv (m1, m2, typ);

  v2.matrix_type (typ);
  return ret;
}

DEFBINOP_FN (pow, float_scalar, float_matrix, xpow)

DEFBINOP (ldiv, float_scalar, float_matrix)
{
  const octave_float_scalar& v1 = dynamic_cast<const octave_float_scalar&> (a1);
  const octave_float_matrix& v2 = dynamic_cast<const octave_float_matrix&> (a2);

  return octave_value (v2.float_array_value () / v1.float_value ());
}

DEFNDBINOP_FN (lt, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_lt)
DEFNDBINOP_FN (le, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_le)
DEFNDBINOP_FN (eq, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_eq)
DEFNDBINOP_FN (ge, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_ge)
DEFNDBINOP_FN (gt, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_gt)
DEFNDBINOP_FN (ne, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_ne)

DEFNDBINOP_OP (el_mul, float_scalar, float_matrix, float_scalar,
               float_array, *)
DEFNDBINOP_FN (el_div, float_scalar, float_matrix, float_scalar,
               float_array, elem_xdiv)
DEFNDBINOP_FN (el_pow, float_scalar, float_matrix, float_scalar,
               float_array, elem_xpow)

DEFBINOP (el_ldiv, float_scalar, float_matrix)
{
  const octave_float_scalar& v1 = dynamic_cast<const octave_float_scalar&> (a1);
  const octave_float_matrix& v2 = dynamic_cast<const octave_float_matrix&> (a2);

  return octave_value (v2.float_array_value () / v1.float_value ());
}

DEFNDBINOP_FN (el_and, float_scalar, float_matrix, float_scalar,
               float_array, mx_el_and)
DEFNDBINOP_FN (el_or,  float_scalar, float_matrix, float_scalar,
               float_array, mx_el_or)

DEFNDCATOP_FN (fs_fm, float_scalar, float_matrix, float_array,
               float_array, concat)

DEFNDCATOP_FN (s_fm, scalar, float_matrix, float_array, float_array, concat)

DEFNDCATOP_FN (fs_m, float_scalar, matrix, float_array, float_array, concat)

DEFCONV (matrix_conv, float_scalar, float_matrix)
{
  const octave_float_scalar& v = dynamic_cast<const octave_float_scalar&> (a);

  return new octave_float_matrix (v.float_matrix_value ());
}

void
install_fs_fm_ops (octave::type_info& ti)
{
  INSTALL_BINOP_TI (ti, op_add, octave_float_scalar, octave_float_matrix, add);
  INSTALL_BINOP_TI (ti, op_sub, octave_float_scalar, octave_float_matrix, sub);
  INSTALL_BINOP_TI (ti, op_mul, octave_float_scalar, octave_float_matrix, mul);
  INSTALL_BINOP_TI (ti, op_div, octave_float_scalar, octave_float_matrix, div);
  INSTALL_BINOP_TI (ti, op_pow, octave_float_scalar, octave_float_matrix, pow);
  INSTALL_BINOP_TI (ti, op_ldiv, octave_float_scalar, octave_float_matrix, ldiv);
  INSTALL_BINOP_TI (ti, op_lt, octave_float_scalar, octave_float_matrix, lt);
  INSTALL_BINOP_TI (ti, op_le, octave_float_scalar, octave_float_matrix, le);
  INSTALL_BINOP_TI (ti, op_eq, octave_float_scalar, octave_float_matrix, eq);
  INSTALL_BINOP_TI (ti, op_ge, octave_float_scalar, octave_float_matrix, ge);
  INSTALL_BINOP_TI (ti, op_gt, octave_float_scalar, octave_float_matrix, gt);
  INSTALL_BINOP_TI (ti, op_ne, octave_float_scalar, octave_float_matrix, ne);
  INSTALL_BINOP_TI (ti, op_el_mul, octave_float_scalar, octave_float_matrix,
                    el_mul);
  INSTALL_BINOP_TI (ti, op_el_div, octave_float_scalar, octave_float_matrix,
                    el_div);
  INSTALL_BINOP_TI (ti, op_el_pow, octave_float_scalar, octave_float_matrix,
                    el_pow);
  INSTALL_BINOP_TI (ti, op_el_ldiv, octave_float_scalar, octave_float_matrix,
                    el_ldiv);
  INSTALL_BINOP_TI (ti, op_el_and, octave_float_scalar, octave_float_matrix,
                    el_and);
  INSTALL_BINOP_TI (ti, op_el_or, octave_float_scalar, octave_float_matrix,
                    el_or);

  INSTALL_CATOP_TI (ti, octave_float_scalar, octave_float_matrix, fs_fm);
  INSTALL_CATOP_TI (ti, octave_scalar, octave_float_matrix, s_fm);
  INSTALL_CATOP_TI (ti, octave_float_scalar, octave_matrix, fs_m);

  INSTALL_ASSIGNCONV_TI (ti, octave_float_scalar, octave_float_matrix,
                         octave_float_matrix);
  INSTALL_ASSIGNCONV_TI (ti, octave_scalar, octave_float_matrix, octave_matrix);

  INSTALL_WIDENOP_TI (ti, octave_float_scalar, octave_float_matrix, matrix_conv);
}

OCTAVE_END_NAMESPACE(octave)
