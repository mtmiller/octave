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

#if ! defined (octave_ov_ch_mat_h)
#define octave_ov_ch_mat_h 1

#include "octave-config.h"

#include <cstdlib>

#include <iosfwd>
#include <string>

#include "mx-base.h"
#include "str-vec.h"

#include "error.h"
#include "ov.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-int64.h"
#include "ov-re-mat.h"
#include "ov-typeinfo.h"

class octave_value_list;

// Character matrix values.

class
octave_char_matrix : public octave_base_matrix<charNDArray>
{
protected:

  octave_char_matrix (void)
    : octave_base_matrix<charNDArray> () { }

  octave_char_matrix (const charMatrix& chm)
    : octave_base_matrix<charNDArray> (chm) { }

  octave_char_matrix (const charNDArray& chm)
    : octave_base_matrix<charNDArray> (chm) { }

  octave_char_matrix (const Array<char>& chm)
    : octave_base_matrix<charNDArray> (chm) { }

  octave_char_matrix (char c)
    : octave_base_matrix<charNDArray> (c) { }

  octave_char_matrix (const char *s)
    : octave_base_matrix<charNDArray> (s) { }

  octave_char_matrix (const std::string& s)
    : octave_base_matrix<charNDArray> (s) { }

  octave_char_matrix (const string_vector& s)
    : octave_base_matrix<charNDArray> (s) { }

  octave_char_matrix (const octave_char_matrix& chm)
    : octave_base_matrix<charNDArray> (chm) { }

public:

  ~octave_char_matrix (void) = default;

  octave_base_value * clone (void) const
  { return new octave_char_matrix (*this); }
  octave_base_value * empty_clone (void) const
  { return new octave_char_matrix (); }

  octave::idx_vector index_vector (bool require_integers = false) const;

  builtin_type_t builtin_type (void) const { return btyp_char; }

  bool is_char_matrix (void) const { return true; }
  bool is_real_matrix (void) const { return true; }

  bool isreal (void) const { return true; }

  double double_value (bool = false) const;

  float float_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
  { return double_value (frc_str_conv); }

  float float_scalar_value (bool frc_str_conv = false) const
  { return float_value (frc_str_conv); }

  octave_int64 int64_scalar_value () const;
  octave_uint64 uint64_scalar_value () const;

  Matrix matrix_value (bool = false) const
  { return Matrix (charMatrix (m_matrix)); }

  FloatMatrix float_matrix_value (bool = false) const
  { return FloatMatrix (charMatrix (m_matrix)); }

  NDArray array_value (bool = false) const
  { return NDArray (m_matrix); }

  FloatNDArray float_array_value (bool = false) const
  { return FloatNDArray (m_matrix); }

  Complex complex_value (bool = false) const;

  FloatComplex float_complex_value (bool = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const
  { return ComplexMatrix (charMatrix (m_matrix)); }

  FloatComplexMatrix float_complex_matrix_value (bool = false) const
  { return FloatComplexMatrix (charMatrix (m_matrix)); }

  ComplexNDArray complex_array_value (bool = false) const
  { return ComplexNDArray (m_matrix); }

  FloatComplexNDArray float_complex_array_value (bool = false) const
  { return FloatComplexNDArray (m_matrix); }

  charMatrix char_matrix_value (bool = false) const
  { return charMatrix (m_matrix); }

  charNDArray char_array_value (bool = false) const
  { return m_matrix; }

  octave_value convert_to_str_internal (bool, bool, char type) const
  { return octave_value (m_matrix, type); }

  octave_value as_double (void) const;
  octave_value as_single (void) const;

  octave_value as_int8 (void) const;
  octave_value as_int16 (void) const;
  octave_value as_int32 (void) const;
  octave_value as_int64 (void) const;

  octave_value as_uint8 (void) const;
  octave_value as_uint16 (void) const;
  octave_value as_uint32 (void) const;
  octave_value as_uint64 (void) const;

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

  mxArray * as_mxArray (bool interleaved) const;

  octave_value map (unary_mapper_t umap) const;
};

#endif
