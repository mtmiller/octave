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

#if ! defined (octave_ov_range_h)
#define octave_ov_range_h 1

#include "octave-config.h"

#include <cstdlib>

#include <iosfwd>
#include <string>

#include "Range.h"

#include "lo-mappers.h"
#include "lo-utils.h"
#include "mx-base.h"
#include "str-vec.h"

#include "error.h"
#include "oct-stream.h"
#include "ov-base.h"
#include "ov-range-traits.h"
#include "ov-re-mat.h"
#include "ov-typeinfo.h"

class octave_value_list;

// Range values.

template <typename T>
class
ov_range : public octave_base_value
{
public:

  ov_range (void)
    : octave_base_value (), m_range (), m_idx_cache () { }

  ov_range (const octave::range<T>& r)
    : octave_base_value (), m_range (r), m_idx_cache ()
  {
    if (numel () < 0 && numel () != -2)
      error ("invalid range");
  }

  ov_range (const ov_range<T>& r)
    : octave_base_value (), m_range (r.m_range),
      m_idx_cache (r.m_idx_cache ? new idx_vector (*r.m_idx_cache) : nullptr)
  { }

  ov_range (const octave::range<T>& r, const idx_vector& cache)
    : octave_base_value (), m_range (r), m_idx_cache ()
  {
    set_idx_cache (cache);
  }

  // No assignment.
  ov_range& operator = (const ov_range&) = delete;

  ~ov_range (void) { clear_cached_info (); }

  octave_base_value * clone (void) const
  {
    return new ov_range (*this);
  }

  // A range is really just a special kind of real matrix object.  In
  // the places where we need to call empty_clone, it makes more sense
  // to create an empty matrix (0x0) instead of an empty range (1x0).

  octave_base_value * empty_clone (void) const
  {
    return new typename octave_value_range_traits<T>::matrix_type ();
  }

  type_conv_info numeric_conversion_function (void) const;

  octave_base_value * try_narrowing_conversion (void);

  builtin_type_t builtin_type (void) const { return class_to_btyp<T>::btyp; }

  // We don't need to override all three forms of subsref.  The using
  // declaration will avoid warnings about partially-overloaded virtual
  // functions.
  using octave_base_value::subsref;

  octave_value subsref (const std::string& type,
                        const std::list<octave_value_list>& idx);

  octave_value_list subsref (const std::string& type,
                             const std::list<octave_value_list>& idx, int)
  { return subsref (type, idx); }

  octave_value do_index_op (const octave_value_list& idx,
                            bool resize_ok = false);

  idx_vector index_vector (bool require_integers = false) const;

  dim_vector dims (void) const
  {
    octave_idx_type n = numel ();
    return dim_vector (n > 0, n);
  }

  octave_idx_type numel (void) const { return m_range.numel (); }

  octave_idx_type nnz (void) const
  {
    // FIXME: this is a potential waste of memory.

    octave_value tmp (raw_array_value ());
    return tmp.nnz ();
  }

  octave_value resize (const dim_vector& dv, bool fill = false) const;

  size_t byte_size (void) const { return 3 * sizeof (T); }

  octave_value reshape (const dim_vector& new_dims) const
  {
    return raw_array_value ().reshape (new_dims);
  }

  octave_value permute (const Array<int>& vec, bool inv = false) const
  {
    return raw_array_value ().permute (vec, inv);
  }

  octave_value squeeze (void) const { return m_range; }

  octave_value full_value (void) const { return raw_array_value (); }

  bool is_defined (void) const { return true; }

  bool is_constant (void) const { return true; }

  bool is_range (void) const { return true; }

  bool is_double_type (void) const { return builtin_type () == btyp_double; }

  bool is_single_type (void) const { return builtin_type () == btyp_float; }

  bool isfloat (void) const { return btyp_isfloat (builtin_type ()); }

  bool is_int8_type (void) const { return builtin_type () == btyp_int8; }

  bool is_int16_type (void) const { return builtin_type () == btyp_int16; }

  bool is_int32_type (void) const { return builtin_type () == btyp_int32; }

  bool is_int64_type (void) const { return builtin_type () == btyp_int64; }

  bool is_uint8_type (void) const { return builtin_type () == btyp_uint8; }

  bool is_uint16_type (void) const { return builtin_type () == btyp_uint16; }

  bool is_uint32_type (void) const { return builtin_type () == btyp_uint32; }

  bool is_uint64_type (void) const { return builtin_type () == btyp_uint64; }

  bool isinteger (void) const
  {
    return btyp_isinteger (builtin_type ());
  }

  bool isreal (void) const { return isfloat (); }

  bool isnumeric (void) const
  {
    return btyp_isnumeric (builtin_type ());
  }

  bool is_true (void) const { return nnz () == numel (); }

  octave_value all (int dim = 0) const
  {
    // FIXME: this is a potential waste of memory.

    typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;
    typename ov_mx_type::object_type m (raw_array_value ());

    return m.all (dim);
  }

  octave_value any (int dim = 0) const
  {
    // FIXME: this is a potential waste of memory.

    typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;
    typename ov_mx_type::object_type m (raw_array_value ());

    return m.any (dim);
  }

  octave_value diag (octave_idx_type k = 0) const
  {
    // FIXME: this is a potential waste of memory.

    return m_range.diag (k);
  }

  octave_value diag (octave_idx_type nr, octave_idx_type nc) const
  {
    // FIXME: this is a potential waste of memory.

    typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;
    typename ov_mx_type::object_type m (raw_array_value ());

    return m.diag (nr, nc);
  }

  octave_value sort (octave_idx_type dim = 0, sortmode mode = ASCENDING) const
  {
    Array<T> tmp = raw_array_value ();
    return tmp.sort (dim, mode);
  }

  octave_value sort (Array<octave_idx_type>& sidx, octave_idx_type dim = 0,
                     sortmode mode = ASCENDING) const
  {
    Array<T> tmp = raw_array_value ();
    return tmp.sort (sidx, dim, mode);
  }

  sortmode issorted (sortmode mode = UNSORTED) const
  {
    return m_range.issorted (mode);
  }

  Array<octave_idx_type> sort_rows_idx (sortmode) const
  {
    return Array<octave_idx_type> (dim_vector (1, 0));
  }

  sortmode is_sorted_rows (sortmode mode = UNSORTED) const
  {
    return (mode == UNSORTED) ? ASCENDING : mode;
  }

  Array<T> raw_array_value (void) const { return m_range.array_value (); }

  double double_value (bool = false) const;

  float float_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
  {
    return double_value (frc_str_conv);
  }

  float float_scalar_value (bool frc_str_conv = false) const
  {
    return float_value (frc_str_conv);
  }

  Matrix matrix_value (bool = false) const
  {
    return raw_array_value ();
  }

  FloatMatrix float_matrix_value (bool = false) const
  {
    return raw_array_value ();
  }

  NDArray array_value (bool = false) const
  {
    return raw_array_value ();
  }

  FloatNDArray float_array_value (bool = false) const
  {
    return raw_array_value ();
  }

  charNDArray char_array_value (bool = false) const;

  // FIXME: it would be better to have Range::intXNDArray_value
  // functions to avoid the intermediate conversion to a matrix
  // object.

  int8NDArray int8_array_value (void) const
  {
    return raw_array_value ();
  }

  int16NDArray int16_array_value (void) const
  {
    return raw_array_value ();
  }

  int32NDArray int32_array_value (void) const
  {
    return raw_array_value ();
  }

  int64NDArray int64_array_value (void) const
  {
    return raw_array_value ();
  }

  uint8NDArray uint8_array_value (void) const
  {
    return raw_array_value ();
  }

  uint16NDArray uint16_array_value (void) const
  {
    return raw_array_value ();
  }

  uint32NDArray uint32_array_value (void) const
  {
    return raw_array_value ();
  }

  uint64NDArray uint64_array_value (void) const
  {
    return raw_array_value ();
  }

  SparseMatrix sparse_matrix_value (bool = false) const
  {
    return SparseMatrix (matrix_value ());
  }

  SparseComplexMatrix sparse_complex_matrix_value (bool = false) const
  {
    return SparseComplexMatrix (complex_matrix_value ());
  }

  Complex complex_value (bool = false) const;

  FloatComplex float_complex_value (bool = false) const;

  boolNDArray bool_array_value (bool warn = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const
  {
    return raw_array_value ();
  }

  FloatComplexMatrix float_complex_matrix_value (bool = false) const
  {
    return raw_array_value ();
  }

  ComplexNDArray complex_array_value (bool = false) const
  {
    return raw_array_value ();
  }

  FloatComplexNDArray float_complex_array_value (bool = false) const
  {
    return raw_array_value ();
  }

  octave::range<float> float_range_value (void) const;

  octave::range<double> range_value (void) const;

  octave::range<octave_int8> int8_range_value (void) const;

  octave::range<octave_int16> int16_range_value (void) const;

  octave::range<octave_int32> int32_range_value (void) const;

  octave::range<octave_int64> int64_range_value (void) const;

  octave::range<octave_uint8> uint8_range_value (void) const;

  octave::range<octave_uint16> uint16_range_value (void) const;

  octave::range<octave_uint32> uint32_range_value (void) const;

  octave::range<octave_uint64> uint64_range_value (void) const;

  octave_value convert_to_str_internal (bool pad, bool force, char type) const;

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

  void print (std::ostream& os, bool pr_as_read_syntax = false);

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

  bool print_name_tag (std::ostream& os, const std::string& name) const;

  void short_disp (std::ostream& os) const;

  float_display_format get_edit_display_format (void) const;

  std::string edit_display (const float_display_format& fmt,
                            octave_idx_type i, octave_idx_type j) const;

  bool save_ascii (std::ostream& os);

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool save_as_floats);

  bool load_binary (std::istream& is, bool swap,
                    octave::mach_info::float_format fmt);

  bool save_hdf5 (octave_hdf5_id loc_id, const char *name, bool flag);

  bool load_hdf5 (octave_hdf5_id loc_id, const char *name);

  int write (octave::stream& os, int block_size,
             oct_data_conv::data_type output_type, int skip,
             octave::mach_info::float_format flt_fmt) const
  {
    // FIXME: could be more memory efficient by having a
    // special case of the octave::stream::write method for ranges.

    return os.write (matrix_value (), block_size, output_type, skip, flt_fmt);
  }

  mxArray * as_mxArray (bool interleaved) const;

  octave_value map (unary_mapper_t umap) const
  {
    octave_value tmp (raw_array_value ());
    return tmp.map (umap);
  }

  octave_value fast_elem_extract (octave_idx_type n) const;

protected:

  octave::range<T> m_range;

  idx_vector set_idx_cache (const idx_vector& idx) const
  {
    delete m_idx_cache;
    m_idx_cache = (idx ? new idx_vector (idx) : nullptr);
    return idx;
  }

  void clear_cached_info (void) const
  {
    delete m_idx_cache; m_idx_cache = nullptr;
  }

  mutable idx_vector *m_idx_cache;

  static octave_hdf5_id hdf5_save_type;

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, float)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, double)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_int8)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_int16)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_int32)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_int64)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_uint8)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_uint16)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_uint32)
DECLARE_TEMPLATE_OV_TYPEID_SPECIALIZATIONS (ov_range, octave_uint64)

// Specializations.

template <>
octave::range<float>
ov_range<float>::float_range_value (void) const;

template <>
octave::range<double>
ov_range<double>::range_value (void) const;

template <>
octave::range<octave_int8>
ov_range<octave_int8>::int8_range_value (void) const;

template <>
octave::range<octave_int16>
ov_range<octave_int16>::int16_range_value (void) const;

template <>
octave::range<octave_int32>
ov_range<octave_int32>::int32_range_value (void) const;

template <>
octave::range<octave_int64>
ov_range<octave_int64>::int64_range_value (void) const;

template <>
octave::range<octave_uint8>
ov_range<octave_uint8>::uint8_range_value (void) const;

template <>
octave::range<octave_uint16>
ov_range<octave_uint16>::uint16_range_value (void) const;

template <>
octave::range<octave_uint32>
ov_range<octave_uint32>::uint32_range_value (void) const;

template <>
octave::range<octave_uint64>
ov_range<octave_uint64>::uint64_range_value (void) const;

// The following specializations are here to preserve previous Range
// performance until solutions can be generalized for other types.

template <>
idx_vector
ov_range<double>::index_vector (bool require_integers) const;

template <>
octave_idx_type
ov_range<double>::nnz (void) const;

// The following specialization is also historical baggage.  For double
// ranges, we can produce special double-valued diagnoal matrix objects
// but Octave currently provides only double and Complex diagonal matrix
// objects.

template <>
octave_value
ov_range<double>::diag (octave_idx_type k) const;

template <>
octave_value
ov_range<double>::diag (octave_idx_type nr, octave_idx_type nc) const;

template <>
void
ov_range<double>::print_raw (std::ostream& os, bool pr_as_read_syntax) const;


typedef ov_range<float> octave_float_range;
typedef ov_range<double> octave_double_range;

typedef ov_range<octave_int8> octave_int8_range;
typedef ov_range<octave_int16> octave_int16_range;
typedef ov_range<octave_int32> octave_int32_range;
typedef ov_range<octave_int64> octave_int64_range;

typedef ov_range<octave_uint8> octave_uint8_range;
typedef ov_range<octave_uint16> octave_uint16_range;
typedef ov_range<octave_uint32> octave_uint32_range;
typedef ov_range<octave_uint64> octave_uint64_range;

typedef octave_double_range octave_range;

#endif
