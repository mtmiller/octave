////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2020 The Octave Project Developers
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

#include "data-conv.h"
#include "quit.h"
#include "str-vec.h"

#include "ovl.h"
#include "oct-stream.h"
#include "ov.h"
#include "ov-base.h"
#include "ov-bool.h"
#include "ov-bool-mat.h"
#include "ov-cell.h"
#include "ov-scalar.h"
#include "ov-float.h"
#include "ov-re-mat.h"
#include "ov-flt-re-mat.h"
#include "ov-re-diag.h"
#include "ov-flt-re-diag.h"
#include "ov-perm.h"
#include "ov-bool-sparse.h"
#include "ov-cx-sparse.h"
#include "ov-re-sparse.h"
#include "ov-int8.h"
#include "ov-int16.h"
#include "ov-int32.h"
#include "ov-int64.h"
#include "ov-uint8.h"
#include "ov-uint16.h"
#include "ov-uint32.h"
#include "ov-uint64.h"
#include "ov-complex.h"
#include "ov-flt-complex.h"
#include "ov-cx-mat.h"
#include "ov-flt-cx-mat.h"
#include "ov-cx-diag.h"
#include "ov-flt-cx-diag.h"
#include "ov-ch-mat.h"
#include "ov-str-mat.h"
#include "ov-range.h"
#include "ov-struct.h"
#include "ov-class.h"
#include "ov-classdef.h"
#include "ov-oncleanup.h"
#include "ov-cs-list.h"
#include "ov-colon.h"
#include "ov-builtin.h"
#include "ov-dld-fcn.h"
#include "ov-usr-fcn.h"
#include "ov-fcn-handle.h"
#include "ov-typeinfo.h"
#include "ov-magic-int.h"
#include "ov-null-mat.h"
#include "ov-lazy-idx.h"
#include "ov-java.h"

#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "interpreter-private.h"
#include "pager.h"
#include "parse.h"
#include "pr-flt-fmt.h"
#include "pr-output.h"
#include "symtab.h"
#include "utils.h"
#include "variables.h"

// We are likely to have a lot of octave_value objects to allocate, so
// make the grow_size large.

// If TRUE, don't create special diagonal matrix objects.

static bool Vdisable_diagonal_matrix = false;

// If TRUE, don't create special permutation matrix objects.

static bool Vdisable_permutation_matrix = false;

// If TRUE, don't create special range objects.

static bool Vdisable_range = false;

// FIXME

// Octave's value type.

octave_base_value *
octave_value::nil_rep (void)
{
  static octave_base_value nr;
  return &nr;
}

std::string
octave_value::unary_op_as_string (unary_op op)
{
  switch (op)
    {
    case op_not:
      return "!";

    case op_uplus:
      return "+";

    case op_uminus:
      return "-";

    case op_transpose:
      return ".'";

    case op_hermitian:
      return "'";

    case op_incr:
      return "++";

    case op_decr:
      return "--";

    default:
      return "<unknown>";
    }
}

std::string
octave_value::unary_op_fcn_name (unary_op op)
{
  switch (op)
    {
    case op_not:
      return "not";

    case op_uplus:
      return "uplus";

    case op_uminus:
      return "uminus";

    case op_transpose:
      return "transpose";

    case op_hermitian:
      return "ctranspose";

    default:
      return "<unknown>";
    }
}

std::string
octave_value::binary_op_as_string (binary_op op)
{
  switch (op)
    {
    case op_add:
      return "+";

    case op_sub:
      return "-";

    case op_mul:
      return "*";

    case op_div:
      return "/";

    case op_pow:
      return "^";

    case op_ldiv:
      return R"(\)";

    case op_lt:
      return "<";

    case op_le:
      return "<=";

    case op_eq:
      return "==";

    case op_ge:
      return ">=";

    case op_gt:
      return ">";

    case op_ne:
      return "!=";

    case op_el_mul:
      return ".*";

    case op_el_div:
      return "./";

    case op_el_pow:
      return ".^";

    case op_el_ldiv:
      return R"(.\)";

    case op_el_and:
      return "&";

    case op_el_or:
      return "|";

    case op_struct_ref:
      return ".";

    default:
      return "<unknown>";
    }
}

std::string
octave_value::binary_op_fcn_name (binary_op op)
{
  switch (op)
    {
    case op_add:
      return "plus";

    case op_sub:
      return "minus";

    case op_mul:
      return "mtimes";

    case op_div:
      return "mrdivide";

    case op_pow:
      return "mpower";

    case op_ldiv:
      return "mldivide";

    case op_lt:
      return "lt";

    case op_le:
      return "le";

    case op_eq:
      return "eq";

    case op_ge:
      return "ge";

    case op_gt:
      return "gt";

    case op_ne:
      return "ne";

    case op_el_mul:
      return "times";

    case op_el_div:
      return "rdivide";

    case op_el_pow:
      return "power";

    case op_el_ldiv:
      return "ldivide";

    case op_el_and:
      return "and";

    case op_el_or:
      return "or";

    default:
      return "<unknown>";
    }
}

std::string
octave_value::binary_op_fcn_name (compound_binary_op op)
{
  switch (op)
    {
    case op_trans_mul:
      return "transtimes";

    case op_mul_trans:
      return "timestrans";

    case op_herm_mul:
      return "hermtimes";

    case op_mul_herm:
      return "timesherm";

    case op_trans_ldiv:
      return "transldiv";

    case op_herm_ldiv:
      return "hermldiv";

    case op_el_and_not:
      return "andnot";

    case op_el_or_not:
      return "ornot";

    case op_el_not_and:
      return "notand";

    case op_el_not_or:
      return "notor";

    default:
      return "<unknown>";
    }
}

std::string
octave_value::assign_op_as_string (assign_op op)
{
  switch (op)
    {
    case op_asn_eq:
      return "=";

    case op_add_eq:
      return "+=";

    case op_sub_eq:
      return "-=";

    case op_mul_eq:
      return "*=";

    case op_div_eq:
      return "/=";

    case op_ldiv_eq:
      return R"(\=)";

    case op_pow_eq:
      return "^=";

    case op_el_mul_eq:
      return ".*=";

    case op_el_div_eq:
      return "./=";

    case op_el_ldiv_eq:
      return R"(.\=)";

    case op_el_pow_eq:
      return ".^=";

    case op_el_and_eq:
      return "&=";

    case op_el_or_eq:
      return "|=";

    default:
      return "<unknown>";
    }
}

octave_value::binary_op
octave_value::assign_op_to_binary_op (assign_op op)
{
  switch (op)
    {
    case op_add_eq:
      return op_add;

    case op_sub_eq:
      return op_sub;

    case op_mul_eq:
      return op_mul;

    case op_div_eq:
      return op_div;

    case op_ldiv_eq:
      return op_ldiv;

    case op_pow_eq:
      return op_pow;

    case op_el_mul_eq:
      return op_el_mul;

    case op_el_div_eq:
      return op_el_div;

    case op_el_ldiv_eq:
      return op_el_ldiv;

    case op_el_pow_eq:
      return op_el_pow;

    case op_el_and_eq:
      return op_el_and;

    case op_el_or_eq:
      return op_el_or;

    default:
      return unknown_binary_op;
    }
}

octave_value::assign_op
octave_value::binary_op_to_assign_op (binary_op op)
{
  switch (op)
    {
    case op_add:
      return op_add_eq;

    case op_sub:
      return op_sub_eq;

    case op_mul:
      return op_mul_eq;

    case op_div:
      return op_div_eq;

    case op_el_mul:
      return op_el_mul_eq;

    case op_el_div:
      return op_el_div_eq;

    case op_el_and:
      return op_el_and_eq;

    case op_el_or:
      return op_el_or_eq;

    default:
      return unknown_assign_op;
    }
}

octave_value::octave_value (short int i)
  : rep (new octave_scalar (i))
{ }

octave_value::octave_value (unsigned short int i)
  : rep (new octave_scalar (i))
{ }

octave_value::octave_value (int i)
  : rep (new octave_scalar (i))
{ }

octave_value::octave_value (unsigned int i)
  : rep (new octave_scalar (i))
{ }

octave_value::octave_value (long int i)
  : rep (new octave_scalar (i))
{ }

octave_value::octave_value (unsigned long int i)
  : rep (new octave_scalar (i))
{ }

#if defined (OCTAVE_HAVE_LONG_LONG_INT)
octave_value::octave_value (long long int i)
  : rep (new octave_scalar (i))
{ }
#endif

#if defined (OCTAVE_HAVE_UNSIGNED_LONG_LONG_INT)
octave_value::octave_value (unsigned long long int i)
  : rep (new octave_scalar (i))
{ }
#endif

octave_value::octave_value (octave::sys::time t)
  : rep (new octave_scalar (t.double_value ()))
{ }

octave_value::octave_value (double d)
  : rep (new octave_scalar (d))
{ }

octave_value::octave_value (float d)
  : rep (new octave_float_scalar (d))
{ }

octave_value::octave_value (const Cell& c, bool is_csl)
  : rep (is_csl
         ? dynamic_cast<octave_base_value *> (new octave_cs_list (c))
         : dynamic_cast<octave_base_value *> (new octave_cell (c)))
{ }

octave_value::octave_value (const Array<octave_value>& a, bool is_csl)
  : rep (is_csl
         ? dynamic_cast<octave_base_value *> (new octave_cs_list (Cell (a)))
         : dynamic_cast<octave_base_value *> (new octave_cell (Cell (a))))
{ }

octave_value::octave_value (const Matrix& m, const MatrixType& t)
  : rep (new octave_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatMatrix& m, const MatrixType& t)
  : rep (new octave_float_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const NDArray& a)
  : rep (new octave_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatNDArray& a)
  : rep (new octave_float_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<double>& a)
  : rep (new octave_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<float>& a)
  : rep (new octave_float_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagArray2<double>& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_matrix (Matrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagArray2<float>& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_float_matrix (FloatMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_float_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagArray2<Complex>& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_complex_matrix (ComplexMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_complex_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagArray2<FloatComplex>& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_float_complex_matrix (FloatComplexMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_float_complex_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagMatrix& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_matrix (Matrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatDiagMatrix& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_float_matrix (FloatMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_float_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const RowVector& v)
  : rep (new octave_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatRowVector& v)
  : rep (new octave_float_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const ColumnVector& v)
  : rep (new octave_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatColumnVector& v)
  : rep (new octave_float_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const Complex& C)
  : rep (new octave_complex (C))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplex& C)
  : rep (new octave_float_complex (C))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexMatrix& m, const MatrixType& t)
  : rep (new octave_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplexMatrix& m, const MatrixType& t)
  : rep (new octave_float_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexNDArray& a)
  : rep (new octave_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplexNDArray& a)
  : rep (new octave_float_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<Complex>& a)
  : rep (new octave_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<FloatComplex>& a)
  : rep (new octave_float_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexDiagMatrix& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_complex_matrix (ComplexMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_complex_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplexDiagMatrix& d)
  : rep (Vdisable_diagonal_matrix
         ? dynamic_cast<octave_base_value *> (new octave_float_complex_matrix (FloatComplexMatrix (d)))
         : dynamic_cast<octave_base_value *> (new octave_float_complex_diag_matrix (d)))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexRowVector& v)
  : rep (new octave_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplexRowVector& v)
  : rep (new octave_float_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexColumnVector& v)
  : rep (new octave_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const FloatComplexColumnVector& v)
  : rep (new octave_float_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const PermMatrix& p)
  : rep (Vdisable_permutation_matrix
         ? dynamic_cast<octave_base_value *> (new octave_matrix (Matrix (p)))
         : dynamic_cast<octave_base_value *> (new octave_perm_matrix (p)))
{
  maybe_mutate ();
}

octave_value::octave_value (bool b)
  : rep (new octave_bool (b))
{ }

octave_value::octave_value (const boolMatrix& bm, const MatrixType& t)
  : rep (new octave_bool_matrix (bm, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const boolNDArray& bnda)
  : rep (new octave_bool_matrix (bnda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<bool>& bnda)
  : rep (new octave_bool_matrix (bnda))
{
  maybe_mutate ();
}

octave_value::octave_value (char c, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (c)
         : new octave_char_matrix_sq_str (c))
{
  maybe_mutate ();
}

octave_value::octave_value (const char *s, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (s)
         : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const std::string& s, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (s)
         : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const string_vector& s, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (s)
         : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const charMatrix& chm, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (chm)
         : new octave_char_matrix_sq_str (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const charNDArray& chm, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (chm)
         : new octave_char_matrix_sq_str (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<char>& chm, char type)
  : rep (type == '"'
         ? new octave_char_matrix_dq_str (chm)
         : new octave_char_matrix_sq_str (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseMatrix& m, const MatrixType& t)
  : rep (new octave_sparse_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const Sparse<double>& m, const MatrixType& t)
  : rep (new octave_sparse_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseComplexMatrix& m, const MatrixType& t)
  : rep (new octave_sparse_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const Sparse<Complex>& m, const MatrixType& t)
  : rep (new octave_sparse_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseBoolMatrix& bm, const MatrixType& t)
  : rep (new octave_sparse_bool_matrix (bm, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const Sparse<bool>& bm, const MatrixType& t)
  : rep (new octave_sparse_bool_matrix (bm, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int8& i)
  : rep (new octave_int8_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint8& i)
  : rep (new octave_uint8_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int16& i)
  : rep (new octave_int16_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint16& i)
  : rep (new octave_uint16_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int32& i)
  : rep (new octave_int32_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint32& i)
  : rep (new octave_uint32_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int64& i)
  : rep (new octave_int64_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint64& i)
  : rep (new octave_uint64_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const int8NDArray& inda)
  : rep (new octave_int8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_int8>& inda)
  : rep (new octave_int8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint8NDArray& inda)
  : rep (new octave_uint8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_uint8>& inda)
  : rep (new octave_uint8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int16NDArray& inda)
  : rep (new octave_int16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_int16>& inda)
  : rep (new octave_int16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint16NDArray& inda)
  : rep (new octave_uint16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_uint16>& inda)
  : rep (new octave_uint16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int32NDArray& inda)
  : rep (new octave_int32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_int32>& inda)
  : rep (new octave_int32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint32NDArray& inda)
  : rep (new octave_uint32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_uint32>& inda)
  : rep (new octave_uint32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int64NDArray& inda)
  : rep (new octave_int64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_int64>& inda)
  : rep (new octave_int64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint64NDArray& inda)
  : rep (new octave_uint64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_uint64>& inda)
  : rep (new octave_uint64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const Array<octave_idx_type>& inda, bool zero_based,
                            bool cache_index)
  : rep (new octave_matrix (inda, zero_based, cache_index))
{
  maybe_mutate ();
}

octave_value::octave_value (const idx_vector& idx, bool lazy)
  : rep ()
{
  double scalar;
  Range range;
  NDArray array;
  boolNDArray mask;
  idx_vector::idx_class_type idx_class;

  if (lazy)
    {
      // Only make lazy indices out of ranges and index vectors.
      switch (idx.idx_class ())
        {
        case idx_vector::class_range:
        case idx_vector::class_vector:
          rep = new octave_lazy_index (idx);
          maybe_mutate ();
          return;

        default:
          break;
        }
    }

  idx.unconvert (idx_class, scalar, range, array, mask);

  switch (idx_class)
    {
    case idx_vector::class_colon:
      rep = new octave_magic_colon ();
      break;

    case idx_vector::class_range:
      rep = new octave_range (range, idx);
      break;

    case idx_vector::class_scalar:
      rep = new octave_scalar (scalar);
      break;

    case idx_vector::class_vector:
      rep = new octave_matrix (array, idx);
      break;

    case idx_vector::class_mask:
      rep = new octave_bool_matrix (mask, idx);
      break;

    default:
      panic_impossible ();
      break;
    }

  // FIXME: needed?
  maybe_mutate ();
}

octave_value::octave_value (const Array<std::string>& cellstr)
  : rep (new octave_cell (cellstr))
{
  maybe_mutate ();
}

octave_value::octave_value (double base, double limit, double inc)
  : rep (new octave_range (base, limit, inc))
{
  maybe_mutate ();
}

octave_value::octave_value (const Range& r, bool force_range)
  : rep (nullptr)
{
  if (! force_range && ! r.ok ())
    error ("invalid range");

  if (force_range || ! Vdisable_range)
    rep = dynamic_cast<octave_base_value *> (new octave_range (r));
  else
    rep = dynamic_cast<octave_base_value *> (new octave_matrix (r.matrix_value ()));

  maybe_mutate ();
}

octave_value::octave_value (const octave_map& m)
  : rep (new octave_struct (m))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_scalar_map& m)
  : rep (new octave_scalar_struct (m))
{ }

octave_value::octave_value (const std::map<std::string, octave_value>& m)
  : rep (new octave_scalar_struct (m))
{ }

octave_value::octave_value (const octave_map& m, const std::string& id,
                            const std::list<std::string>& plist)
  : rep (new octave_class (m, id, plist))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_scalar_map& m, const std::string& id,
                            const std::list<std::string>& plist)
  : rep (new octave_class (m, id, plist))
{ }

octave_value::octave_value (const octave_value_list& l)
  : rep (new octave_cs_list (l))
{ }

octave_value::octave_value (octave_value::magic_colon)
  : rep (new octave_magic_colon ())
{ }

octave_value::octave_value (octave_base_value *new_rep, bool borrow)
  : rep (new_rep)
{
  if (borrow)
    rep->count++;
}

octave_base_value *
octave_value::clone (void) const
{
  return rep->clone ();
}

void
octave_value::maybe_mutate (void)
{
  octave_base_value *tmp = rep->try_narrowing_conversion ();

  if (tmp && tmp != rep)
    {
      if (--rep->count == 0)
        delete rep;

      rep = tmp;
    }
}

DEFUN (double, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} double (@var{x})
Convert @var{x} to double precision type.
@seealso{single}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return ovl (args(0).as_double ());
}

/*
%!assert (class (double (single (1))), "double")
%!assert (class (double (single (1 + i))), "double")
%!assert (class (double (int8 (1))), "double")
%!assert (class (double (uint8 (1))), "double")
%!assert (class (double (int16 (1))), "double")
%!assert (class (double (uint16 (1))), "double")
%!assert (class (double (int32 (1))), "double")
%!assert (class (double (uint32 (1))), "double")
%!assert (class (double (int64 (1))), "double")
%!assert (class (double (uint64 (1))), "double")
%!assert (class (double (true)), "double")
%!assert (class (double ("A")), "double")
%!test
%! x = sparse (logical ([1 0; 0 1]));
%! y = double (x);
%! assert (class (x), "logical");
%! assert (class (y), "double");
%! assert (issparse (y));
%!test
%! x = diag (single ([1 3 2]));
%! y = double (x);
%! assert (class (x), "single");
%! assert (class (y), "double");
%!test
%! x = diag (single ([i 3 2]));
%! y = double (x);
%! assert (class (x), "single");
%! assert (class (y), "double");
*/

DEFUN (single, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} single (@var{x})
Convert @var{x} to single precision type.
@seealso{double}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_single ();

  return ovl ();
}

/*
%!assert (class (single (1)), "single")
%!assert (class (single (1 + i)), "single")
%!assert (class (single (int8 (1))), "single")
%!assert (class (single (uint8 (1))), "single")
%!assert (class (single (int16 (1))), "single")
%!assert (class (single (uint16 (1))), "single")
%!assert (class (single (int32 (1))), "single")
%!assert (class (single (uint32 (1))), "single")
%!assert (class (single (int64 (1))), "single")
%!assert (class (single (uint64 (1))), "single")
%!assert (class (single (true)), "single")
%!assert (class (single ("A")), "single")
%!error (single (sparse (1)))
%!test
%! x = diag ([1 3 2]);
%! y = single (x);
%! assert (class (x), "double");
%! assert (class (y), "single");
%!test
%! x = diag ([i 3 2]);
%! y = single (x);
%! assert (class (x), "double");
%! assert (class (y), "single");
*/

DEFUN (int8, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} int8 (@var{x})
Convert @var{x} to 8-bit integer type.
@seealso{uint8, int16, uint16, int32, uint32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_int8 ();
}

/*
%!assert (class (int8 (1)), "int8")
%!assert (int8 (1.25), int8 (1))
%!assert (int8 (1.5), int8 (2))
%!assert (int8 (-1.5), int8 (-2))
%!assert (int8 (2^9), int8 (2^8-1))
%!assert (int8 (-2^9), int8 (-2^8))
*/

DEFUN (int16, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} int16 (@var{x})
Convert @var{x} to 16-bit integer type.
@seealso{int8, uint8, uint16, int32, uint32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_int16 ();
}

/*
%!assert (class (int16 (1)), "int16")
%!assert (int16 (1.25), int16 (1))
%!assert (int16 (1.5), int16 (2))
%!assert (int16 (-1.5), int16 (-2))
%!assert (int16 (2^17), int16 (2^16-1))
%!assert (int16 (-2^17), int16 (-2^16))
*/

DEFUN (int32, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} int32 (@var{x})
Convert @var{x} to 32-bit integer type.
@seealso{int8, uint8, int16, uint16, uint32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_int32 ();
}

/*
%!assert (class (int32 (1)), "int32")
%!assert (int32 (1.25), int32 (1))
%!assert (int32 (1.5), int32 (2))
%!assert (int32 (-1.5), int32 (-2))
%!assert (int32 (2^33), int32 (2^32-1))
%!assert (int32 (-2^33), int32 (-2^32))
*/

DEFUN (int64, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} int64 (@var{x})
Convert @var{x} to 64-bit integer type.
@seealso{int8, uint8, int16, uint16, int32, uint32, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_int64 ();
}

/*
%!assert (class (int64 (1)), "int64")
%!assert (int64 (1.25), int64 (1))
%!assert (int64 (1.5), int64 (2))
%!assert (int64 (-1.5), int64 (-2))
%!assert (int64 (2^65), int64 (2^64-1))
%!assert (int64 (-2^65), int64 (-2^64))
*/

DEFUN (uint8, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint8 (@var{x})
Convert @var{x} to unsigned 8-bit integer type.
@seealso{int8, int16, uint16, int32, uint32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_uint8 ();
}

/*
%!assert (class (uint8 (1)), "uint8")
%!assert (uint8 (1.25), uint8 (1))
%!assert (uint8 (1.5), uint8 (2))
%!assert (uint8 (-1.5), uint8 (0))
%!assert (uint8 (2^9), uint8 (2^8-1))
%!assert (uint8 (-2^9), uint8 (0))
*/

DEFUN (uint16, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint16 (@var{x})
Convert @var{x} to unsigned 16-bit integer type.
@seealso{int8, uint8, int16, int32, uint32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_uint16 ();
}

/*
%!assert (class (uint16 (1)), "uint16")
%!assert (uint16 (1.25), uint16 (1))
%!assert (uint16 (1.5), uint16 (2))
%!assert (uint16 (-1.5), uint16 (0))
%!assert (uint16 (2^17), uint16 (2^16-1))
%!assert (uint16 (-2^17), uint16 (0))
*/

DEFUN (uint32, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint32 (@var{x})
Convert @var{x} to unsigned 32-bit integer type.
@seealso{int8, uint8, int16, uint16, int32, int64, uint64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_uint32 ();
}

/*
%!assert (class (uint32 (1)), "uint32")
%!assert (uint32 (1.25), uint32 (1))
%!assert (uint32 (1.5), uint32 (2))
%!assert (uint32 (-1.5), uint32 (0))
%!assert (uint32 (2^33), uint32 (2^32-1))
%!assert (uint32 (-2^33), uint32 (0))
*/

DEFUN (uint64, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint64 (@var{x})
Convert @var{x} to unsigned 64-bit integer type.
@seealso{int8, uint8, int16, uint16, int32, uint32, int64}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return args(0).as_uint64 ();
}

/*
%!assert (class (uint64 (1)), "uint64")
%!assert (uint64 (1.25), uint64 (1))
%!assert (uint64 (1.5), uint64 (2))
%!assert (uint64 (-1.5), uint64 (0))
%!assert (uint64 (2^65), uint64 (2^64-1))
%!assert (uint64 (-2^65), uint64 (0))
*/

octave_value
octave_value::single_subsref (const std::string& type,
                              const octave_value_list& idx)
{
  std::list<octave_value_list> i;

  i.push_back (idx);

  return rep->subsref (type, i);
}

octave_value_list
octave_value::subsref (const std::string& type,
                       const std::list<octave_value_list>& idx, int nargout)
{
  return rep->subsref (type, idx, nargout);
}

octave_value
octave_value::next_subsref (const std::string& type,
                            const std::list<octave_value_list>& idx,
                            size_t skip)
{
  if (idx.size () > skip)
    {
      std::list<octave_value_list> new_idx (idx);
      for (size_t i = 0; i < skip; i++)
        new_idx.erase (new_idx.begin ());
      return subsref (type.substr (skip), new_idx);
    }
  else
    return *this;
}

octave_value_list
octave_value::next_subsref (int nargout, const std::string& type,
                            const std::list<octave_value_list>& idx,
                            size_t skip)
{
  if (idx.size () > skip)
    {
      std::list<octave_value_list> new_idx (idx);
      for (size_t i = 0; i < skip; i++)
        new_idx.erase (new_idx.begin ());
      return subsref (type.substr (skip), new_idx, nargout);
    }
  else
    return *this;
}

octave_value
octave_value::next_subsref (bool auto_add, const std::string& type,
                            const std::list<octave_value_list>& idx,
                            size_t skip)
{
  if (idx.size () > skip)
    {
      std::list<octave_value_list> new_idx (idx);
      for (size_t i = 0; i < skip; i++)
        new_idx.erase (new_idx.begin ());
      return subsref (type.substr (skip), new_idx, auto_add);
    }
  else
    return *this;
}

octave_value
octave_value::subsasgn (const std::string& type,
                        const std::list<octave_value_list>& idx,
                        const octave_value& rhs)
{
  return rep->subsasgn (type, idx, rhs);
}

octave_value
octave_value::undef_subsasgn (const std::string& type,
                              const std::list<octave_value_list>& idx,
                              const octave_value& rhs)
{
  return rep->undef_subsasgn (type, idx, rhs);
}

octave_value&
octave_value::assign (assign_op op, const std::string& type,
                      const std::list<octave_value_list>& idx,
                      const octave_value& rhs)
{
  make_unique ();

  octave_value t_rhs = rhs;

  if (op != op_asn_eq)
    {
      if (! is_defined ())
        error ("in computed assignment A(index) OP= X, A must be defined first");

      octave_value t = subsref (type, idx);

      binary_op binop = op_eq_to_binary_op (op);

      t_rhs = octave::binary_op (binop, t, rhs);
    }

  *this = subsasgn (type, idx, t_rhs);

  return *this;
}

octave_value&
octave_value::assign (assign_op op, const octave_value& rhs)
{
  if (op == op_asn_eq)
    // Regularize a null matrix if stored into a variable.
    operator = (rhs.storable_value ());
  else if (is_defined ())
    {
      octave::type_info::assign_op_fcn f = nullptr;

      // Only attempt to operate in-place if this variable is unshared.
      if (rep->count == 1)
        {
          int tthis = this->type_id ();
          int trhs = rhs.type_id ();

          octave::type_info& ti
            = octave::__get_type_info__ ("octave_value::assign");

          f = ti.lookup_assign_op (op, tthis, trhs);
        }

      if (f)
        {
          f (*rep, octave_value_list (), rhs.get_rep ());
          // Usually unnecessary, but may be needed (complex arrays).
          maybe_mutate ();
        }
      else
        {

          binary_op binop = op_eq_to_binary_op (op);

          octave_value t = octave::binary_op (binop, *this, rhs);

          operator = (t);
        }
    }
  else
    error ("in computed assignment A OP= X, A must be defined first");

  return *this;
}

// FIXME: This is a bit of a kluge.  We'd like to just use val.dims()
// and if val is an object, expect that dims will call size if it is
// overloaded by a user-defined method.  But there are currently some
// unresolved const issues that prevent that solution from working.

std::string
octave_value::get_dims_str (void) const
{
  octave_value tmp = *this;

  Matrix sz = tmp.size ();

  dim_vector dv = dim_vector::alloc (sz.numel ());

  for (octave_idx_type i = 0; i < dv.ndims (); i++)
    dv(i) = sz(i);

  return dv.str ();
}

octave_idx_type
octave_value::length (void) const
{
  octave_idx_type retval = 0;

  const dim_vector dv = dims ();

  for (int i = 0; i < dv.ndims (); i++)
    {
      if (dv(i) == 0)
        {
          retval = 0;
          break;
        }

      if (dv(i) > retval)
        retval = dv(i);
    }

  return retval;
}

bool
octave_value::is_equal (const octave_value& test) const
{
  bool retval = false;

  // If there is no op_eq for these types, we can't compare values.

  if (rows () == test.rows () && columns () == test.columns ())
    {
      octave_value tmp = octave::binary_op (octave_value::op_eq, *this, test);

      // Empty array also means a match.
      if (tmp.is_defined ())
        {
          if (tmp.isempty ())
            retval = true;
          else
            {
              // Reshape into a vector and call all() explicitly,
              // to avoid Octave:array-as-logical warning.
              tmp = tmp.reshape (dim_vector (tmp.numel (), 1));
              retval = tmp.all ().is_true ();
            }
        }
    }

  return retval;
}

// Define the idx_type_value function here instead of in ov.h to avoid
// needing definitions for the SIZEOF_X macros in ov.h.

octave_idx_type
octave_value::idx_type_value (bool req_int, bool frc_str_conv) const
{
#if defined (OCTAVE_ENABLE_64)
  return int64_value (req_int, frc_str_conv);
#else
  return int_value (req_int, frc_str_conv);
#endif
}

Cell
octave_value::cell_value (void) const
{
  return rep->cell_value ();
}

octave_map
octave_value::map_value (void) const
{
  return rep->map_value ();
}

octave_scalar_map
octave_value::scalar_map_value (void) const
{
  return rep->scalar_map_value ();
}

octave_function *
octave_value::function_value (bool silent) const
{
  return rep->function_value (silent);
}

octave_classdef *
octave_value::classdef_object_value (bool silent) const
{
  return rep->classdef_object_value (silent);
}

octave_user_function *
octave_value::user_function_value (bool silent) const
{
  return rep->user_function_value (silent);
}

octave_user_script *
octave_value::user_script_value (bool silent) const
{
  return rep->user_script_value (silent);
}

octave_user_code *
octave_value::user_code_value (bool silent) const
{
  return rep->user_code_value (silent);
}

octave_fcn_handle *
octave_value::fcn_handle_value (bool silent) const
{
  return rep->fcn_handle_value (silent);
}

octave_value_list
octave_value::list_value (void) const
{
  return rep->list_value ();
}

static dim_vector
make_vector_dims (const dim_vector& dv, bool force_vector_conversion,
                  const std::string& my_type, const std::string& wanted_type)
{
  dim_vector retval (dv);
  retval.chop_trailing_singletons ();
  octave_idx_type nel = dv.numel ();

  if (retval.ndims () > 2 || (retval(0) != 1 && retval(1) != 1))
    {
      if (! force_vector_conversion)
        warn_implicit_conversion ("Octave:array-to-vector",
                                  my_type.c_str (), wanted_type.c_str ());
      retval = dim_vector (nel, 1);
    }

  return retval;
}

ColumnVector
octave_value::column_vector_value (bool force_string_conv,
                                   bool frc_vec_conv) const
{
  return ColumnVector (vector_value (force_string_conv,
                                     frc_vec_conv));
}

ComplexColumnVector
octave_value::complex_column_vector_value (bool force_string_conv,
                                           bool frc_vec_conv) const
{
  return ComplexColumnVector (complex_vector_value (force_string_conv,
                                                    frc_vec_conv));
}

RowVector
octave_value::row_vector_value (bool force_string_conv,
                                bool frc_vec_conv) const
{
  return RowVector (vector_value (force_string_conv,
                                  frc_vec_conv));
}

ComplexRowVector
octave_value::complex_row_vector_value (bool force_string_conv,
                                        bool frc_vec_conv) const
{
  return ComplexRowVector (complex_vector_value (force_string_conv,
                                                 frc_vec_conv));
}

Array<double>
octave_value::vector_value (bool force_string_conv,
                            bool force_vector_conversion) const
{
  Array<double> retval = array_value (force_string_conv);

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "real vector"));
}

template <typename T>
static Array<int>
convert_to_int_array (const Array<octave_int<T>>& A)
{
  Array<int> retval (A.dims ());
  octave_idx_type n = A.numel ();

  for (octave_idx_type i = 0; i < n; i++)
    retval.xelem (i) = octave_int<int> (A.xelem (i));

  return retval;
}

Array<int>
octave_value::int_vector_value (bool require_int, bool force_string_conv,
                                bool force_vector_conversion) const
{
  Array<int> retval;

  if (isinteger ())
    {
      if (is_int32_type ())
        retval = convert_to_int_array (int32_array_value ());
      else if (is_int64_type ())
        retval = convert_to_int_array (int64_array_value ());
      else if (is_int16_type ())
        retval = convert_to_int_array (int16_array_value ());
      else if (is_int8_type ())
        retval = convert_to_int_array (int8_array_value ());
      else if (is_uint32_type ())
        retval = convert_to_int_array (uint32_array_value ());
      else if (is_uint64_type ())
        retval = convert_to_int_array (uint64_array_value ());
      else if (is_uint16_type ())
        retval = convert_to_int_array (uint16_array_value ());
      else if (is_uint8_type ())
        retval = convert_to_int_array (uint8_array_value ());
      else
        retval = array_value (force_string_conv);
    }
  else
    {
      const NDArray a = array_value (force_string_conv);

      if (require_int)
        {
          retval.resize (a.dims ());
          for (octave_idx_type i = 0; i < a.numel (); i++)
            {
              double ai = a.elem (i);
              int v = static_cast<int> (ai);
              if (ai == v)
                retval.xelem (i) = v;
              else
                {
                  error_with_cfn ("conversion of %g to int value failed", ai);
                  break;
                }
            }
        }
      else
        retval = Array<int> (a);
    }

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "integer vector"));
}

template <typename T>
static Array<octave_idx_type>
convert_to_octave_idx_type_array (const Array<octave_int<T>>& A)
{
  Array<octave_idx_type> retval (A.dims ());
  octave_idx_type n = A.numel ();

  for (octave_idx_type i = 0; i < n; i++)
    retval.xelem (i) = octave_int<octave_idx_type> (A.xelem (i));

  return retval;
}

Array<octave_idx_type>
octave_value::octave_idx_type_vector_value (bool require_int,
                                            bool force_string_conv,
                                            bool force_vector_conversion) const
{
  Array<octave_idx_type> retval;

  if (isinteger ())
    {
      if (is_int32_type ())
        retval = convert_to_octave_idx_type_array (int32_array_value ());
      else if (is_int64_type ())
        retval = convert_to_octave_idx_type_array (int64_array_value ());
      else if (is_int16_type ())
        retval = convert_to_octave_idx_type_array (int16_array_value ());
      else if (is_int8_type ())
        retval = convert_to_octave_idx_type_array (int8_array_value ());
      else if (is_uint32_type ())
        retval = convert_to_octave_idx_type_array (uint32_array_value ());
      else if (is_uint64_type ())
        retval = convert_to_octave_idx_type_array (uint64_array_value ());
      else if (is_uint16_type ())
        retval = convert_to_octave_idx_type_array (uint16_array_value ());
      else if (is_uint8_type ())
        retval = convert_to_octave_idx_type_array (uint8_array_value ());
      else
        retval = array_value (force_string_conv);
    }
  else
    {
      const NDArray a = array_value (force_string_conv);

      if (require_int)
        {
          retval.resize (a.dims ());
          for (octave_idx_type i = 0; i < a.numel (); i++)
            {
              double ai = a.elem (i);
              octave_idx_type v = static_cast<octave_idx_type> (ai);
              if (ai == v)
                retval.xelem (i) = v;
              else
                {
                  error_with_cfn ("conversion of %g to octave_idx_type value failed", ai);
                  break;
                }
            }
        }
      else
        retval = Array<octave_idx_type> (a);
    }

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "integer vector"));
}

Array<Complex>
octave_value::complex_vector_value (bool force_string_conv,
                                    bool force_vector_conversion) const
{
  Array<Complex> retval = complex_array_value (force_string_conv);

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "complex vector"));
}

FloatColumnVector
octave_value::float_column_vector_value (bool force_string_conv,
                                         bool frc_vec_conv) const
{
  return FloatColumnVector (float_vector_value (force_string_conv,
                                                frc_vec_conv));
}

FloatComplexColumnVector
octave_value::float_complex_column_vector_value (bool force_string_conv,
                                                 bool frc_vec_conv) const
{
  return
    FloatComplexColumnVector (float_complex_vector_value (force_string_conv,
                                                          frc_vec_conv));
}

FloatRowVector
octave_value::float_row_vector_value (bool force_string_conv,
                                      bool frc_vec_conv) const
{
  return FloatRowVector (float_vector_value (force_string_conv,
                                             frc_vec_conv));
}

FloatComplexRowVector
octave_value::float_complex_row_vector_value (bool force_string_conv,
                                              bool frc_vec_conv) const
{
  return FloatComplexRowVector (float_complex_vector_value (force_string_conv,
                                                           frc_vec_conv));
}

Array<float>
octave_value::float_vector_value (bool force_string_conv,
                                  bool force_vector_conversion) const
{
  Array<float> retval = float_array_value (force_string_conv);

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "real vector"));
}

Array<FloatComplex>
octave_value::float_complex_vector_value (bool force_string_conv,
                                          bool force_vector_conversion) const
{
  Array<FloatComplex> retval = float_complex_array_value (force_string_conv);

  return retval.reshape (make_vector_dims (retval.dims (),
                                           force_vector_conversion,
                                           type_name (), "complex vector"));
}

// NAME can't always be "x ## FCN" because some of the original
// value extraction functions perform implicit type conversions that we
// wish to avoid for these functions.

#define XVALUE_EXTRACTOR(TYPE, NAME, FCN)               \
  TYPE                                                  \
  octave_value::NAME (const char *fmt, ...) const       \
  {                                                     \
    TYPE retval;                                        \
                                                        \
    try                                                 \
      {                                                 \
        retval = FCN ();                                \
      }                                                 \
    catch (octave::execution_exception& e)               \
      {                                                 \
        if (fmt)                                        \
          {                                             \
            va_list args;                               \
            va_start (args, fmt);                       \
            verror (e, fmt, args);                      \
            va_end (args);                              \
          }                                             \
                                                        \
        throw e;                                        \
      }                                                 \
                                                        \
    return retval;                                      \
  }

XVALUE_EXTRACTOR (short int, xshort_value, short_value)

XVALUE_EXTRACTOR (unsigned short int, xushort_value, ushort_value)

XVALUE_EXTRACTOR (int, xint_value, int_value)

XVALUE_EXTRACTOR (unsigned int, xuint_value, uint_value)

XVALUE_EXTRACTOR (int, xnint_value, nint_value)

XVALUE_EXTRACTOR (long int, xlong_value, long_value)

XVALUE_EXTRACTOR (unsigned long int, xulong_value, ulong_value)

XVALUE_EXTRACTOR (int64_t, xint64_value, int64_value)

XVALUE_EXTRACTOR (uint64_t, xuint64_value, uint64_value)

XVALUE_EXTRACTOR (octave_idx_type, xidx_type_value, idx_type_value)

XVALUE_EXTRACTOR (double, xdouble_value, double_value)
XVALUE_EXTRACTOR (float, xfloat_value, float_value)

XVALUE_EXTRACTOR (double, xscalar_value, scalar_value)
XVALUE_EXTRACTOR (float, xfloat_scalar_value, float_scalar_value)

XVALUE_EXTRACTOR (Matrix, xmatrix_value, matrix_value)
XVALUE_EXTRACTOR (FloatMatrix, xfloat_matrix_value, float_matrix_value)

XVALUE_EXTRACTOR (NDArray, xarray_value, array_value)
XVALUE_EXTRACTOR (FloatNDArray, xfloat_array_value, float_array_value)

XVALUE_EXTRACTOR (Complex, xcomplex_value, complex_value)
XVALUE_EXTRACTOR (FloatComplex, xfloat_complex_value, float_complex_value)

XVALUE_EXTRACTOR (ComplexMatrix, xcomplex_matrix_value, complex_matrix_value)
XVALUE_EXTRACTOR (FloatComplexMatrix, xfloat_complex_matrix_value, float_complex_matrix_value)

XVALUE_EXTRACTOR (ComplexNDArray, xcomplex_array_value, complex_array_value)
XVALUE_EXTRACTOR (FloatComplexNDArray, xfloat_complex_array_value, float_complex_array_value)

XVALUE_EXTRACTOR (bool, xbool_value, bool_value)
XVALUE_EXTRACTOR (boolMatrix, xbool_matrix_value, bool_matrix_value)
XVALUE_EXTRACTOR (boolNDArray, xbool_array_value, bool_array_value)

XVALUE_EXTRACTOR (charMatrix, xchar_matrix_value, char_matrix_value)
XVALUE_EXTRACTOR (charNDArray, xchar_array_value, char_array_value)

XVALUE_EXTRACTOR (SparseMatrix, xsparse_matrix_value, sparse_matrix_value)
XVALUE_EXTRACTOR (SparseComplexMatrix, xsparse_complex_matrix_value, sparse_complex_matrix_value)
XVALUE_EXTRACTOR (SparseBoolMatrix, xsparse_bool_matrix_value, sparse_bool_matrix_value)

XVALUE_EXTRACTOR (DiagMatrix, xdiag_matrix_value, diag_matrix_value)
XVALUE_EXTRACTOR (FloatDiagMatrix, xfloat_diag_matrix_value, float_diag_matrix_value)
XVALUE_EXTRACTOR (ComplexDiagMatrix, xcomplex_diag_matrix_value, complex_diag_matrix_value)
XVALUE_EXTRACTOR (FloatComplexDiagMatrix, xfloat_complex_diag_matrix_value, float_complex_diag_matrix_value)

XVALUE_EXTRACTOR (PermMatrix, xperm_matrix_value, perm_matrix_value)

XVALUE_EXTRACTOR (octave_int8, xint8_scalar_value, int8_scalar_value)
XVALUE_EXTRACTOR (octave_int16, xint16_scalar_value, int16_scalar_value)
XVALUE_EXTRACTOR (octave_int32, xint32_scalar_value, int32_scalar_value)
XVALUE_EXTRACTOR (octave_int64, xint64_scalar_value, int64_scalar_value)

XVALUE_EXTRACTOR (octave_uint8, xuint8_scalar_value, uint8_scalar_value)
XVALUE_EXTRACTOR (octave_uint16, xuint16_scalar_value, uint16_scalar_value)
XVALUE_EXTRACTOR (octave_uint32, xuint32_scalar_value, uint32_scalar_value)
XVALUE_EXTRACTOR (octave_uint64, xuint64_scalar_value, uint64_scalar_value)

XVALUE_EXTRACTOR (int8NDArray, xint8_array_value, int8_array_value)
XVALUE_EXTRACTOR (int16NDArray, xint16_array_value, int16_array_value)
XVALUE_EXTRACTOR (int32NDArray, xint32_array_value, int32_array_value)
XVALUE_EXTRACTOR (int64NDArray, xint64_array_value, int64_array_value)

XVALUE_EXTRACTOR (uint8NDArray, xuint8_array_value, uint8_array_value)
XVALUE_EXTRACTOR (uint16NDArray, xuint16_array_value, uint16_array_value)
XVALUE_EXTRACTOR (uint32NDArray, xuint32_array_value, uint32_array_value)
XVALUE_EXTRACTOR (uint64NDArray, xuint64_array_value, uint64_array_value)

XVALUE_EXTRACTOR (std::string, xstring_value, rep->xstring_value)
XVALUE_EXTRACTOR (string_vector, xstring_vector_value, string_vector_value)

XVALUE_EXTRACTOR (Cell, xcell_value, cell_value)
XVALUE_EXTRACTOR (Array<std::string>, xcellstr_value, cellstr_value)

XVALUE_EXTRACTOR (Range, xrange_value, range_value)

XVALUE_EXTRACTOR (octave_map, xmap_value, map_value)
XVALUE_EXTRACTOR (octave_scalar_map, xscalar_map_value, scalar_map_value)

XVALUE_EXTRACTOR (ColumnVector, xcolumn_vector_value, column_vector_value)
XVALUE_EXTRACTOR (ComplexColumnVector, xcomplex_column_vector_value, complex_column_vector_value)

XVALUE_EXTRACTOR (RowVector, xrow_vector_value, row_vector_value)
XVALUE_EXTRACTOR (ComplexRowVector, xcomplex_row_vector_value, complex_row_vector_value)

XVALUE_EXTRACTOR (FloatColumnVector, xfloat_column_vector_value, float_column_vector_value)
XVALUE_EXTRACTOR (FloatComplexColumnVector, xfloat_complex_column_vector_value, float_complex_column_vector_value)

XVALUE_EXTRACTOR (FloatRowVector, xfloat_row_vector_value, float_row_vector_value)
XVALUE_EXTRACTOR (FloatComplexRowVector, xfloat_complex_row_vector_value, float_complex_row_vector_value)

XVALUE_EXTRACTOR (Array<int>, xint_vector_value, int_vector_value)
XVALUE_EXTRACTOR (Array<octave_idx_type>, xoctave_idx_type_vector_value, octave_idx_type_vector_value)

XVALUE_EXTRACTOR (Array<double>, xvector_value, vector_value)
XVALUE_EXTRACTOR (Array<Complex>, xcomplex_vector_value, complex_vector_value)

XVALUE_EXTRACTOR (Array<float>, xfloat_vector_value, float_vector_value)
XVALUE_EXTRACTOR (Array<FloatComplex>, xfloat_complex_vector_value, float_complex_vector_value)

XVALUE_EXTRACTOR (octave_function *, xfunction_value, function_value)
XVALUE_EXTRACTOR (octave_user_function *, xuser_function_value, user_function_value)
XVALUE_EXTRACTOR (octave_user_script *, xuser_script_value, user_script_value)
XVALUE_EXTRACTOR (octave_user_code *, xuser_code_value, user_code_value)
XVALUE_EXTRACTOR (octave_fcn_handle *, xfcn_handle_value, fcn_handle_value)

XVALUE_EXTRACTOR (octave_value_list, xlist_value, list_value)

#undef XVALUE_EXTRACTOR

octave_value
octave_value::storable_value (void) const
{
  octave_value retval = *this;
  if (isnull ())
    retval = octave_value (rep->empty_clone ());
  else if (is_magic_int ())
    retval = octave_value (rep->double_value ());
  else
    retval.maybe_economize ();

  return retval;
}

void
octave_value::make_storable_value (void)
{
  if (isnull ())
    {
      octave_base_value *rc = rep->empty_clone ();
      if (--rep->count == 0)
        delete rep;
      rep = rc;
    }
  else if (is_magic_int ())
    {
      octave_base_value *rc = new octave_scalar (rep->double_value ());
      if (--rep->count == 0)
        delete rep;
      rep = rc;
    }
  else
    maybe_economize ();
}

float_display_format
octave_value::get_edit_display_format (void) const
{
  return rep->get_edit_display_format ();
}

int
octave_value::write (octave::stream& os, int block_size,
                     oct_data_conv::data_type output_type, int skip,
                     octave::mach_info::float_format flt_fmt) const
{
  return rep->write (os, block_size, output_type, skip, flt_fmt);
}

void
octave_value::print_info (std::ostream& os, const std::string& prefix) const
{
  os << prefix << "type_name: " << type_name () << "\n"
     << prefix << "count:     " << get_count () << "\n"
     << prefix << "rep info:  ";

  rep->print_info (os, prefix + ' ');
}

void *
octave_value::mex_get_data (mxClassID class_id, mxComplexity complexity) const
{
  // If class_id is set to mxUNKNOWN_CLASS, return data for any type.
  // Otherwise, require that REP matches the requested type and
  // complexity.

  if (class_id != mxUNKNOWN_CLASS)
    {
      bool type_ok = false;

      switch (class_id)
        {
        case mxDOUBLE_CLASS:
          type_ok = is_double_type ();
          break;

        case mxSINGLE_CLASS:
          type_ok = is_single_type ();
          break;

        case mxINT8_CLASS:
          type_ok = is_int8_type ();
          break;

        case mxINT16_CLASS:
          type_ok = is_int16_type ();
          break;

        case mxINT32_CLASS:
          type_ok = is_int32_type ();
          break;

        case mxINT64_CLASS:
          type_ok = is_int64_type ();
          break;

        case mxUINT8_CLASS:
          type_ok = is_uint8_type ();
          break;

        case mxUINT16_CLASS:
          type_ok = is_uint16_type ();
          break;

        case mxUINT32_CLASS:
          type_ok = is_uint32_type ();
          break;

        case mxUINT64_CLASS:
          type_ok = is_uint64_type ();
          break;

        default:
          // We only expect to see numeric types explicitly requested.
          error ("mex_get_data: unexpected type requested");
          break;
        }

      if (! type_ok)
        error ("mex_get_data: type mismatch");

      if (complexity == mxCOMPLEX && ! iscomplex ())
        error ("mex_get_data: objectis not complex as requested");
    }

  return rep->mex_get_data ();
}

OCTAVE_NORETURN static void
err_unary_op_conversion_failed (const std::string& op,
                                const std::string& tn)
{
  error ("operator %s: type conversion for '%s' failed",
         op.c_str (), tn.c_str ());
}

OCTAVE_NORETURN static void
err_unary_op (const std::string& on, const std::string& tn)
{
  error ("unary operator '%s' not implemented for '%s' operands",
         on.c_str (), tn.c_str ());
}

octave_value&
octave_value::non_const_unary_op (unary_op op)
{
  if (op == op_incr || op == op_decr)
    {
      // We want the error just here, because in the other branch this should
      // not happen, and if it did anyway (internal error), the message would
      // be confusing.
      if (is_undefined ())
        {
          std::string op_str = unary_op_as_string (op);
          error ("in x%s or %sx, x must be defined first",
                 op_str.c_str (), op_str.c_str ());
          return *this;
        }

      // Genuine.
      int t = type_id ();

      octave::type_info& ti = octave::__get_type_info__ ("non_const_unary_op");

      octave::type_info::non_const_unary_op_fcn f
        = ti.lookup_non_const_unary_op (op, t);

      if (f)
        {
          make_unique ();

          f (*rep);
        }
      else
        {
          octave_base_value::type_conv_fcn cf = numeric_conversion_function ();

          if (! cf)
            err_unary_op (octave_value::unary_op_as_string (op), type_name ());

          octave_base_value *tmp = cf (*rep);

          if (! tmp)
            err_unary_op_conversion_failed
              (octave_value::unary_op_as_string (op), type_name ());

          octave_base_value *old_rep = rep;
          rep = tmp;

          t = type_id ();

          f = ti.lookup_non_const_unary_op (op, t);

          if (f)
            {
              f (*rep);

              if (old_rep && --old_rep->count == 0)
                delete old_rep;
            }
          else
            {
              if (old_rep)
                {
                  if (--rep->count == 0)
                    delete rep;

                  rep = old_rep;
                }

              err_unary_op (octave_value::unary_op_as_string (op),
                            type_name ());
            }
        }
    }
  else
    {
      // Non-genuine.
      int t = type_id ();

      octave::type_info::non_const_unary_op_fcn f = nullptr;

      // Only attempt to operate in-place if this variable is unshared.
      if (rep->count == 1)
        {
          octave::type_info& ti
            = octave::__get_type_info__ ("non_const_unary_op");

          f = ti.lookup_non_const_unary_op (op, t);
        }

      if (f)
        f (*rep);
      else
        *this = octave::unary_op (op, *this);
    }

  return *this;
}

octave_value&
octave_value::non_const_unary_op (unary_op op, const std::string& type,
                                  const std::list<octave_value_list>& idx)
{
  if (idx.empty ())
    non_const_unary_op (op);
  else
    {
      // FIXME: only do the following stuff if we can't find a
      // specific function to call to handle the op= operation for the
      // types we have.

      assign_op assop = unary_op_to_assign_op (op);

      assign (assop, type, idx, 1.0);
    }

  return *this;
}

octave_value::assign_op
octave_value::unary_op_to_assign_op (unary_op op)
{
  switch (op)
    {
    case op_incr:
      return op_add_eq;

    case op_decr:
      return op_sub_eq;

    default:
      {
        std::string on = unary_op_as_string (op);
        error ("operator %s: no assign operator found", on.c_str ());
      }
    }
}

octave_value::binary_op
octave_value::op_eq_to_binary_op (assign_op op)
{
  switch (op)
    {
    case op_add_eq:
      return op_add;

    case op_sub_eq:
      return op_sub;

    case op_mul_eq:
      return op_mul;

    case op_div_eq:
      return op_div;

    case op_ldiv_eq:
      return op_ldiv;

    case op_pow_eq:
      return op_pow;

    case op_el_mul_eq:
      return op_el_mul;

    case op_el_div_eq:
      return op_el_div;

    case op_el_ldiv_eq:
      return op_el_ldiv;

    case op_el_pow_eq:
      return op_el_pow;

    case op_el_and_eq:
      return op_el_and;

    case op_el_or_eq:
      return op_el_or;

    default:
      {
        std::string on = assign_op_as_string (op);
        error ("operator %s: no binary operator found", on.c_str ());
      }
    }
}

octave_value
octave_value::empty_conv (const std::string& type, const octave_value& rhs)
{
  if (type.length () > 0)
    {
      switch (type[0])
        {
        case '(':
          if (type.length () > 1 && type[1] == '.')
            return octave_map ();
          else
            return octave_value (rhs.empty_clone ());

        case '{':
          return Cell ();

        case '.':
          return octave_scalar_map ();

        default:
          panic_impossible ();
        }
    }
  else
    return octave_value (rhs.empty_clone ());
}

namespace octave
{
  OCTAVE_NORETURN static void
  err_binary_op (const std::string& on, const std::string& tn1,
                 const std::string& tn2)
  {
    error ("binary operator '%s' not implemented for '%s' by '%s' operations",
           on.c_str (), tn1.c_str (), tn2.c_str ());
  }

  OCTAVE_NORETURN static void
  err_binary_op_conv (const std::string& on)
  {
    error ("type conversion failed for binary operator '%s'", on.c_str ());
  }

  octave_value
  binary_op (type_info& ti, octave_value::binary_op op,
             const octave_value& v1, const octave_value& v2)
  {
    octave_value retval;

    int t1 = v1.type_id ();
    int t2 = v2.type_id ();

    if (t1 == octave_class::static_type_id ()
        || t2 == octave_class::static_type_id ()
        || t1 == octave_classdef::static_type_id ()
        || t2 == octave_classdef::static_type_id ())
      {
        type_info::binary_class_op_fcn f = ti.lookup_binary_class_op (op);

        if (! f)
          err_binary_op (octave_value::binary_op_as_string (op),
                         v1.class_name (), v2.class_name ());

        retval = f (v1, v2);
      }
    else
      {
        // FIXME: we need to handle overloading operators for built-in
        // classes (double, char, int8, etc.)

        type_info::binary_op_fcn f
          = ti.lookup_binary_op (op, t1, t2);

        if (f)
          retval = f (v1.get_rep (), v2.get_rep ());
        else
          {
            octave_value tv1;
            octave_base_value::type_conv_info cf1
              = v1.numeric_conversion_function ();

            octave_value tv2;
            octave_base_value::type_conv_info cf2
              = v2.numeric_conversion_function ();

            // Try biased (one-sided) conversions first.
            if (cf2.type_id () >= 0
                && ti.lookup_binary_op (op, t1, cf2.type_id ()))
              cf1 = nullptr;
            else if (cf1.type_id () >= 0
                     && ti.lookup_binary_op (op, cf1.type_id (), t2))
              cf2 = nullptr;

            if (cf1)
              {
                octave_base_value *tmp = cf1 (v1.get_rep ());

                if (! tmp)
                  err_binary_op_conv (octave_value::binary_op_as_string (op));

                tv1 = octave_value (tmp);
                t1 = tv1.type_id ();
              }
            else
              tv1 = v1;

            if (cf2)
              {
                octave_base_value *tmp = cf2 (v2.get_rep ());

                if (! tmp)
                  err_binary_op_conv (octave_value::binary_op_as_string (op));

                tv2 = octave_value (tmp);
                t2 = tv2.type_id ();
              }
            else
              tv2 = v2;

            if (cf1 || cf2)
              {
                retval = binary_op (op, tv1, tv2);
              }
            else
              {
                //demote double -> single and try again
                cf1 = tv1.numeric_demotion_function ();

                cf2 = tv2.numeric_demotion_function ();

                // Try biased (one-sided) conversions first.
                if (cf2.type_id () >= 0
                    && ti.lookup_binary_op (op, t1, cf2.type_id ()))
                  cf1 = nullptr;
                else if (cf1.type_id () >= 0
                         && ti.lookup_binary_op (op, cf1.type_id (), t2))
                  cf2 = nullptr;

                if (cf1)
                  {
                    octave_base_value *tmp = cf1 (tv1.get_rep ());

                    if (! tmp)
                      err_binary_op_conv (octave_value::binary_op_as_string (op));

                    tv1 = octave_value (tmp);
                    t1 = tv1.type_id ();
                  }

                if (cf2)
                  {
                    octave_base_value *tmp = cf2 (tv2.get_rep ());

                    if (! tmp)
                      err_binary_op_conv (octave_value::binary_op_as_string (op));

                    tv2 = octave_value (tmp);
                    t2 = tv2.type_id ();
                  }

                if (! cf1 && ! cf2)
                  err_binary_op (octave_value::binary_op_as_string (op),
                                 v1.type_name (), v2.type_name ());

                f = ti.lookup_binary_op (op, t1, t2);

                if (! f)
                  err_binary_op (octave_value::binary_op_as_string (op),
                                 v1.type_name (), v2.type_name ());

                retval = f (tv1.get_rep (), tv2.get_rep ());
              }
          }
      }

    return retval;
  }

  octave_value
  binary_op (octave_value::binary_op op, const octave_value& v1,
             const octave_value& v2)
  {
    type_info& ti = __get_type_info__ ("binary_op");

    return binary_op (ti, op, v1, v2);
  }

  static octave_value
  decompose_binary_op (type_info& ti, octave_value::compound_binary_op op,
                       const octave_value& v1, const octave_value& v2)
  {
    switch (op)
      {
      case octave_value::op_trans_mul:
        return binary_op (octave_value::op_mul,
                          unary_op (octave_value::op_transpose, v1), v2);

      case octave_value::op_mul_trans:
        return binary_op (ti, octave_value::op_mul,
                          v1, unary_op (octave_value::op_transpose, v2));

      case octave_value::op_herm_mul:
        return binary_op (ti, octave_value::op_mul,
                          unary_op (octave_value::op_hermitian, v1), v2);

      case octave_value::op_mul_herm:
        return binary_op (ti, octave_value::op_mul,
                          v1, unary_op (octave_value::op_hermitian, v2));

      case octave_value::op_trans_ldiv:
        return binary_op (ti, octave_value::op_ldiv,
                          unary_op (octave_value::op_transpose, v1), v2);

      case octave_value::op_herm_ldiv:
        return binary_op (ti, octave_value::op_ldiv,
                          unary_op (octave_value::op_hermitian, v1), v2);

      case octave_value::op_el_not_and:
        return binary_op (ti, octave_value::op_el_and,
                          unary_op (octave_value::op_not, v1), v2);

      case octave_value::op_el_not_or:
        return binary_op (ti, octave_value::op_el_or,
                          unary_op (octave_value::op_not, v1), v2);

      case octave_value::op_el_and_not:
        return binary_op (ti, octave_value::op_el_and,
                          v1, unary_op (octave_value::op_not, v2));

      case octave_value::op_el_or_not:
        return binary_op (ti, octave_value::op_el_or,
                          v1, unary_op (octave_value::op_not, v2));

      default:
        error ("invalid compound operator");
      }
  }

  octave_value
  binary_op (type_info& ti, octave_value::compound_binary_op op,
             const octave_value& v1, const octave_value& v2)
  {
    octave_value retval;

    int t1 = v1.type_id ();
    int t2 = v2.type_id ();

    if (t1 == octave_class::static_type_id ()
        || t2 == octave_class::static_type_id ()
        || t1 == octave_classdef::static_type_id ()
        || t2 == octave_classdef::static_type_id ())
      {
        type_info::binary_class_op_fcn f = ti.lookup_binary_class_op (op);

        if (f)
          retval = f (v1, v2);
        else
          retval = decompose_binary_op (ti, op, v1, v2);
      }
    else
      {
        type_info::binary_op_fcn f = ti.lookup_binary_op (op, t1, t2);

        if (f)
          retval = f (v1.get_rep (), v2.get_rep ());
        else
          retval = decompose_binary_op (ti, op, v1, v2);
      }

    return retval;
  }

  octave_value
  binary_op (octave_value::compound_binary_op op,
             const octave_value& v1, const octave_value& v2)
  {
    type_info& ti = __get_type_info__ ("binary_op");

    return binary_op (ti, op, v1, v2);
  }

  OCTAVE_NORETURN static void
  err_cat_op (const std::string& tn1, const std::string& tn2)
  {
    error ("concatenation operator not implemented for '%s' by '%s' operations",
           tn1.c_str (), tn2.c_str ());
  }

  OCTAVE_NORETURN static void
  err_cat_op_conv (void)
  {
    error ("type conversion failed for concatenation operator");
  }

  octave_value
  cat_op (type_info& ti, const octave_value& v1,
          const octave_value& v2, const Array<octave_idx_type>& ra_idx)
  {
    octave_value retval;

    // Can't rapid return for concatenation with an empty object here as
    // something like cat(1,[],single([]) must return the correct type.

    int t1 = v1.type_id ();
    int t2 = v2.type_id ();

    type_info::cat_op_fcn f = ti.lookup_cat_op (t1, t2);

    if (f)
      retval = f (v1.get_rep (), v2.get_rep (), ra_idx);
    else
      {
        octave_value tv1;
        octave_base_value::type_conv_info cf1 = v1.numeric_conversion_function ();

        octave_value tv2;
        octave_base_value::type_conv_info cf2 = v2.numeric_conversion_function ();

        // Try biased (one-sided) conversions first.
        if (cf2.type_id () >= 0 && ti.lookup_cat_op (t1, cf2.type_id ()))
          cf1 = nullptr;
        else if (cf1.type_id () >= 0 && ti.lookup_cat_op (cf1.type_id (), t2))
          cf2 = nullptr;

        if (cf1)
          {
            octave_base_value *tmp = cf1 (v1.get_rep ());

            if (! tmp)
              err_cat_op_conv ();

            tv1 = octave_value (tmp);
            t1 = tv1.type_id ();
          }
        else
          tv1 = v1;

        if (cf2)
          {
            octave_base_value *tmp = cf2 (v2.get_rep ());

            if (! tmp)
              err_cat_op_conv ();

            tv2 = octave_value (tmp);
            t2 = tv2.type_id ();
          }
        else
          tv2 = v2;

        if (! cf1 && ! cf2)
          err_cat_op (v1.type_name (), v2.type_name ());

        retval = cat_op (ti, tv1, tv2, ra_idx);
      }

    return retval;
  }

  octave_value
  cat_op (const octave_value& v1, const octave_value& v2,
          const Array<octave_idx_type>& ra_idx)
  {
    type_info& ti = __get_type_info__ ("cat_op");

    return cat_op (ti, v1, v2, ra_idx);
  }

  octave_value
  colon_op (const octave_value& base, const octave_value& increment,
            const octave_value& limit, bool is_for_cmd_expr)
  {
    octave_value retval;

    if (base.isobject () || increment.isobject () || limit.isobject ())
      {
        octave_value_list tmp1;

        if (increment.is_defined ())
          {
            tmp1(2) = limit;
            tmp1(1) = increment;
            tmp1(0) = base;
          }
        else
          {
            tmp1(1) = limit;
            tmp1(0) = base;
          }

        interpreter& interp = __get_interpreter__ ("colon_op");

        symbol_table& symtab = interp.get_symbol_table ();

        octave_value fcn = symtab.find_function ("colon", tmp1);

        if (fcn.is_defined ())
          {
            octave_value_list tmp2 = interp.feval (fcn, tmp1, 1);

            return tmp2 (0);
          }
      }

    bool result_is_str = (base.is_string () && limit.is_string ());
    bool dq_str = (base.is_dq_string () || limit.is_dq_string ());

    if (base.numel () > 1 || limit.numel () > 1
        || (increment.is_defined () && increment.numel () > 1))
      warning_with_id ("Octave:colon-nonscalar-argument",
                       "colon arguments should be scalars");

    if (base.iscomplex () || limit.iscomplex ()
        || (increment.is_defined () && increment.iscomplex ()))
      warning_with_id ("Octave:colon-complex-argument",
                       "imaginary part of complex colon arguments is ignored");

    Matrix m_base, m_limit, m_increment;

    try
      {
        m_base = base.matrix_value (true);
      }
    catch (execution_exception& e)
      {
        error (e, "invalid base value in colon expression");
      }

    try
      {
        m_limit = limit.matrix_value (true);
      }
    catch (execution_exception& e)
      {
        error (e, "invalid limit value in colon expression");
      }

    try
      {
        m_increment = (increment.is_defined ()
                       ? increment.matrix_value (true)
                       : Matrix (1, 1, 1.0));
      }
    catch (execution_exception& e)
      {
        error (e, "invalid increment value in colon expression");
      }

    bool base_empty = m_base.isempty ();
    bool limit_empty = m_limit.isempty ();
    bool increment_empty = m_increment.isempty ();

    if (base_empty || limit_empty || increment_empty)
      retval = Range ();
    else
      {
        Range r (m_base(0), m_limit(0), m_increment(0));

        // For compatibility with Matlab, don't allow the range used in
        // a FOR loop expression to be converted to a Matrix.

        retval = octave_value (r, is_for_cmd_expr);

        if (result_is_str)
          retval = (retval.convert_to_str (false, true, dq_str ? '"' : '\''));
      }

    return retval;
  }

  OCTAVE_NORETURN static void
  err_unary_op_conv (const std::string& on)
  {
    error ("type conversion failed for unary operator '%s'", on.c_str ());
  }

  octave_value
  unary_op (type_info& ti, octave_value::unary_op op,
            const octave_value& v)
  {
    octave_value retval;

    int t = v.type_id ();

    if (t == octave_class::static_type_id ()
        || t == octave_classdef::static_type_id ())
      {
        type_info::unary_class_op_fcn f = ti.lookup_unary_class_op (op);

        if (! f)
          err_unary_op (octave_value::unary_op_as_string (op), v.class_name ());

        retval = f (v);
      }
    else
      {
        // FIXME: we need to handle overloading operators for built-in
        // classes (double, char, int8, etc.)

        type_info::unary_op_fcn f = ti.lookup_unary_op (op, t);

        if (f)
          retval = f (v.get_rep ());
        else
          {
            octave_value tv;
            octave_base_value::type_conv_fcn cf
              = v.numeric_conversion_function ();

            if (! cf)
              err_unary_op (octave_value::unary_op_as_string (op),
                            v.type_name ());

            octave_base_value *tmp = cf (v.get_rep ());

            if (! tmp)
              err_unary_op_conv (octave_value::unary_op_as_string (op));

            tv = octave_value (tmp);
            retval = unary_op (op, tv);
          }
      }

    return retval;
  }

  octave_value
  unary_op (octave_value::unary_op op, const octave_value& v)
  {
    type_info& ti = __get_type_info__ ("unary_op");

    return unary_op (ti, op, v);
  }
}

void
install_types (octave::type_info& ti)
{
  octave_base_value::register_type (ti);
  octave_cell::register_type (ti);
  octave_scalar::register_type (ti);
  octave_complex::register_type (ti);
  octave_matrix::register_type (ti);
  octave_diag_matrix::register_type (ti);
  octave_complex_matrix::register_type (ti);
  octave_complex_diag_matrix::register_type (ti);
  octave_range::register_type (ti);
  octave_bool::register_type (ti);
  octave_bool_matrix::register_type (ti);
  octave_char_matrix_str::register_type (ti);
  octave_char_matrix_sq_str::register_type (ti);
  octave_int8_scalar::register_type (ti);
  octave_int16_scalar::register_type (ti);
  octave_int32_scalar::register_type (ti);
  octave_int64_scalar::register_type (ti);
  octave_uint8_scalar::register_type (ti);
  octave_uint16_scalar::register_type (ti);
  octave_uint32_scalar::register_type (ti);
  octave_uint64_scalar::register_type (ti);
  octave_int8_matrix::register_type (ti);
  octave_int16_matrix::register_type (ti);
  octave_int32_matrix::register_type (ti);
  octave_int64_matrix::register_type (ti);
  octave_uint8_matrix::register_type (ti);
  octave_uint16_matrix::register_type (ti);
  octave_uint32_matrix::register_type (ti);
  octave_uint64_matrix::register_type (ti);
  octave_sparse_bool_matrix::register_type (ti);
  octave_sparse_matrix::register_type (ti);
  octave_sparse_complex_matrix::register_type (ti);
  octave_struct::register_type (ti);
  octave_scalar_struct::register_type (ti);
  octave_class::register_type (ti);
  octave_cs_list::register_type (ti);
  octave_magic_colon::register_type (ti);
  octave_builtin::register_type (ti);
  octave_user_function::register_type (ti);
  octave_dld_function::register_type (ti);
  octave_fcn_handle::register_type (ti);
  octave_float_scalar::register_type (ti);
  octave_float_complex::register_type (ti);
  octave_float_matrix::register_type (ti);
  octave_float_diag_matrix::register_type (ti);
  octave_float_complex_matrix::register_type (ti);
  octave_float_complex_diag_matrix::register_type (ti);
  octave_perm_matrix::register_type (ti);
  octave_magic_int::register_type (ti);
  octave_magic_uint::register_type (ti);
  octave_null_matrix::register_type (ti);
  octave_null_str::register_type (ti);
  octave_null_sq_str::register_type (ti);
  octave_lazy_index::register_type (ti);
  octave_oncleanup::register_type (ti);
  octave_java::register_type (ti);
}

DEFUN (sizeof, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} sizeof (@var{val})
Return the size of @var{val} in bytes.
@seealso{whos}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return ovl (args(0).byte_size ());
}

/*
%!assert (sizeof (uint64 (ones (3))), 72)
%!assert (sizeof (double (zeros (2,4))), 64)
%!assert (sizeof ({"foo", "bar", "baaz"}), 10)
*/

static void
decode_subscripts (const char *name, const octave_value& arg,
                   std::string& type_string,
                   std::list<octave_value_list>& idx)
{
  const octave_map m = arg.xmap_value ("%s: second argument must be a structure with fields 'type' and 'subs'", name);

  if (m.nfields () != 2 || ! m.contains ("type") || ! m.contains ("subs"))
    error ("%s: second argument must be a structure with fields 'type' and 'subs'",
           name);

  octave_idx_type nel = m.numel ();

  type_string = std::string (nel, '\0');
  idx = std::list<octave_value_list> ();

  if (nel == 0)
    return;

  const Cell type = m.contents ("type");
  const Cell subs = m.contents ("subs");

  for (int k = 0; k < nel; k++)
    {
      std::string item = type(k).xstring_value ("%s: type(%d) must be a string", name, k+1);

      if (item == "{}")
        type_string[k] = '{';
      else if (item == "()")
        type_string[k] = '(';
      else if (item == ".")
        type_string[k] = '.';
      else
        error ("%s: invalid indexing type '%s'", name, item.c_str ());

      octave_value_list idx_item;

      if (subs(k).is_string ())
        idx_item(0) = subs(k);
      else if (subs(k).iscell ())
        {
          Cell subs_cell = subs(k).cell_value ();

          for (int n = 0; n < subs_cell.numel (); n++)
            {
              if (subs_cell(n).is_string ()
                  && subs_cell(n).string_value () == ":")
                idx_item(n) = octave_value(octave_value::magic_colon_t);
              else
                idx_item(n) = subs_cell(n);
            }
        }
      else
        error ("%s: subs(%d) must be a string or cell array", name, k+1);

      idx.push_back (idx_item);
    }
}

DEFUN (subsref, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn {} {} subsref (@var{val}, @var{idx})
Perform the subscripted element selection operation on @var{val} according
to the subscript specified by @var{idx}.

The subscript @var{idx} must be a structure array with fields @samp{type}
and @samp{subs}.  Valid values for @samp{type} are @qcode{"()"},
@qcode{"@{@}"}, and @qcode{"."}.  The @samp{subs} field may be either
@qcode{":"} or a cell array of index values.

The following example shows how to extract the first two columns of a matrix

@example
@group
val = magic (3)
    @result{} val = [ 8   1   6
               3   5   7
               4   9   2 ]
idx.type = "()";
idx.subs = @{":", 1:2@};
subsref (val, idx)
     @result{} [ 8   1
          3   5
          4   9 ]
@end group
@end example

@noindent
Note that this is the same as writing @code{val(:, 1:2)}.

If @var{idx} is an empty structure array with fields @samp{type} and
@samp{subs}, return @var{val}.
@seealso{subsasgn, substruct}
@end deftypefn */)
{
  if (args.length () != 2)
    print_usage ();

  std::string type;
  std::list<octave_value_list> idx;

  decode_subscripts ("subsref", args(1), type, idx);

  octave_value arg0 = args(0);

  if (type.empty ())
    return ovl (arg0);
  else
    return arg0.subsref (type, idx, nargout);
}

DEFUN (subsasgn, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} subsasgn (@var{val}, @var{idx}, @var{rhs})
Perform the subscripted assignment operation according to the subscript
specified by @var{idx}.

The subscript @var{idx} must be a structure array with fields @samp{type}
and @samp{subs}.  Valid values for @samp{type} are @qcode{"()"},
@qcode{"@{@}"}, and @qcode{"."}.  The @samp{subs} field may be either
@qcode{":"} or a cell array of index values.

The following example shows how to set the two first columns of a 3-by-3
matrix to zero.

@example
@group
val = magic (3);
idx.type = "()";
idx.subs = @{":", 1:2@};
subsasgn (val, idx, 0)
     @result{}  [ 0   0   6
           0   0   7
           0   0   2 ]
@end group
@end example

Note that this is the same as writing @code{val(:, 1:2) = 0}.

If @var{idx} is an empty structure array with fields @samp{type} and
@samp{subs}, return @var{rhs}.
@seealso{subsref, substruct, optimize_subsasgn_calls}
@end deftypefn */)
{
  if (args.length () != 3)
    print_usage ();

  std::string type;
  std::list<octave_value_list> idx;

  decode_subscripts ("subsasgn", args(1), type, idx);

  if (type.empty ())
    {
      // Regularize a null matrix if stored into a variable.
      return ovl (args(2).storable_value ());
    }
  else
    {
      octave_value arg0 = args(0);
      octave_value arg2 = args(2);

      arg0.make_unique ();

      bool arg2_null = arg2.is_zero_by_zero () && arg2.is_double_type ();

      return ovl (arg0.subsasgn (type, idx, (arg2_null
                                             ? octave_null_matrix::instance
                                             : arg2)));
    }
}

/*
%!test
%! a = reshape ([1:25], 5,5);
%! idx1 = substruct ("()", {3, 3});
%! idx2 = substruct ("()", {2:2:5, 2:2:5});
%! idx3 = substruct ("()", {":", [1,5]});
%! idx4 = struct ("type", {}, "subs", {});
%! assert (subsref (a, idx1), 13);
%! assert (subsref (a, idx2), [7 17; 9 19]);
%! assert (subsref (a, idx3), [1:5; 21:25]');
%! assert (subsref (a, idx4), a);
%! a = subsasgn (a, idx1, 0);
%! a = subsasgn (a, idx2, 0);
%! a = subsasgn (a, idx3, 0);
%!# a = subsasgn (a, idx4, 0);
%! b = [0    6   11   16    0
%!      0    0   12    0    0
%!      0    8    0   18    0
%!      0    0   14    0    0
%!      0   10   15   20    0];
%! assert (a, b);

%!test
%! x = 1:10;
%! assert (subsasgn (x, substruct ("()", {1}), zeros (0, 0)), 2:10);

%!test
%! c = num2cell (reshape ([1:25],5,5));
%! idx1 = substruct  ("{}", {3, 3});
%! idx2 = substruct  ("()", {2:2:5, 2:2:5});
%! idx3 = substruct  ("()", {":", [1,5]});
%! idx2p = substruct ("{}", {2:2:5, 2:2:5});
%! idx3p = substruct ("{}", {":", [1,5]});
%! idx4 = struct ("type", {}, "subs", {});
%! assert ({ subsref(c, idx1) }, {13});
%! assert ({ subsref(c, idx2p) }, {7 9 17 19});
%! assert ({ subsref(c, idx3p) }, num2cell ([1:5, 21:25]));
%! assert (subsref (c, idx4), c);
%! c = subsasgn (c, idx1, 0);
%! c = subsasgn (c, idx2, 0);
%! c = subsasgn (c, idx3, 0);
%!# c = subsasgn (c, idx4, 0);
%! d = {0    6   11   16    0
%!      0    0   12    0    0
%!      0    8    0   18    0
%!      0    0   14    0    0
%!      0   10   15   20    0};
%! assert (c, d);

%!test
%! s.a = "ohai";
%! s.b = "dere";
%! s.c = 42;
%! idx1 = substruct (".", "a");
%! idx2 = substruct (".", "b");
%! idx3 = substruct (".", "c");
%! idx4 = struct ("type", {}, "subs", {});
%! assert (subsref (s, idx1), "ohai");
%! assert (subsref (s, idx2), "dere");
%! assert (subsref (s, idx3), 42);
%! assert (subsref (s, idx4), s);
%! s = subsasgn (s, idx1, "Hello");
%! s = subsasgn (s, idx2, "There");
%! s = subsasgn (s, idx3, 163);
%!# s = subsasgn (s, idx4, 163);
%! t.a = "Hello";
%! t.b = "There";
%! t.c = 163;
%! assert (s, t);
*/

DEFUN (is_sq_string, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} is_sq_string (@var{x})
Return true if @var{x} is a single-quoted character string.
@seealso{is_dq_string, ischar}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return ovl (args(0).is_sq_string ());
}

/*
%!assert (is_sq_string ('foo'), true)
%!assert (is_sq_string ("foo"), false)
%!assert (is_sq_string (1.0), false)
%!assert (is_sq_string ({2.0}), false)

%!error is_sq_string ()
%!error is_sq_string ('foo', 2)
*/

DEFUN (is_dq_string, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} is_dq_string (@var{x})
Return true if @var{x} is a double-quoted character string.
@seealso{is_sq_string, ischar}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  return ovl (args(0).is_dq_string ());
}

/*
%!assert (is_dq_string ("foo"), true)
%!assert (is_dq_string ('foo'), false)
%!assert (is_dq_string (1.0), false)
%!assert (is_dq_string ({2.0}), false)

%!error is_dq_string ()
%!error is_dq_string ("foo", 2)
*/

DEFUN (disable_permutation_matrix, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} disable_permutation_matrix ()
@deftypefnx {} {@var{old_val} =} disable_permutation_matrix (@var{new_val})
@deftypefnx {} {} disable_permutation_matrix (@var{new_val}, "local")
Query or set the internal variable that controls whether permutation
matrices are stored in a special space-efficient format.

The default value is true.  If this option is disabled Octave will store
permutation matrices as full matrices.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{disable_range, disable_diagonal_matrix}
@end deftypefn */)
{
  return SET_INTERNAL_VARIABLE (disable_permutation_matrix);
}

/*
%!function p = __test_dpm__ (dpm)
%!  disable_permutation_matrix (dpm, "local");
%!  [~, ~, p] = lu ([1,2;3,4]);
%!endfunction

%!assert (typeinfo (__test_dpm__ (false)), "permutation matrix")
%!assert (typeinfo (__test_dpm__ (true)), "matrix")
*/

DEFUN (disable_diagonal_matrix, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} disable_diagonal_matrix ()
@deftypefnx {} {@var{old_val} =} disable_diagonal_matrix (@var{new_val})
@deftypefnx {} {} disable_diagonal_matrix (@var{new_val}, "local")
Query or set the internal variable that controls whether diagonal
matrices are stored in a special space-efficient format.

The default value is true.  If this option is disabled Octave will store
diagonal matrices as full matrices.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{disable_range, disable_permutation_matrix}
@end deftypefn */)
{
  return SET_INTERNAL_VARIABLE (disable_diagonal_matrix);
}

/*
%!function [x, xi, fx, fxi] = __test_ddm__ (ddm)
%!  disable_diagonal_matrix (ddm, "local");
%!  x = eye (2);
%!  xi = x*i;
%!  fx = single (x);
%!  fxi = single (xi);
%!endfunction

%!shared x, xi, fx, fxi
%!  [x, xi, fx, fxi] = __test_ddm__ (false);
%!assert (typeinfo (x), "diagonal matrix")
%!assert (typeinfo (xi), "complex diagonal matrix")
%!assert (typeinfo (fx), "float diagonal matrix")
%!assert (typeinfo (fxi), "float complex diagonal matrix")

%!shared x, xi, fx, fxi
%!  [x, xi, fx, fxi] = __test_ddm__ (true);
%!assert (typeinfo (x), "matrix")
%!assert (typeinfo (xi), "complex matrix")
%!assert (typeinfo (fx), "float matrix")
%!assert (typeinfo (fxi), "float complex matrix")
*/

DEFUN (disable_range, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} disable_range ()
@deftypefnx {} {@var{old_val} =} disable_range (@var{new_val})
@deftypefnx {} {} disable_range (@var{new_val}, "local")
Query or set the internal variable that controls whether ranges are stored
in a special space-efficient format.

The default value is true.  If this option is disabled Octave will store
ranges as full matrices.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{disable_diagonal_matrix, disable_permutation_matrix}
@end deftypefn */)
{
  return SET_INTERNAL_VARIABLE (disable_range);
}

/*
%!function r = __test_dr__ (dr)
%!  disable_range (dr, "local");
%!  ## Constant folding will produce range for 1:13.
%!  base = 1;
%!  limit = 13;
%!  r = base:limit;
%!endfunction

%!assert (typeinfo (__test_dr__ (false)), "range")
%!assert (typeinfo (__test_dr__ (true)), "matrix")
*/
