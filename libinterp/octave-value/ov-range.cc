////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2021 The Octave Project Developers
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

// This file should not include config.h.  It is only included in other
// C++ source files that should have included config.h before including
// this file.

#include <istream>
#include <ostream>
#include <sstream>

#include "dNDArray.h"
#include "fNDArray.h"
#include "int8NDArray.h"
#include "int16NDArray.h"
#include "int32NDArray.h"
#include "int64NDArray.h"
#include "uint8NDArray.h"
#include "uint16NDArray.h"
#include "uint32NDArray.h"
#include "uint64NDArray.h"

#include "lo-ieee.h"
#include "lo-utils.h"

#include "defun.h"
#include "variables.h"
#include "errwarn.h"
#include "mxarray.h"
#include "mx-type-traits.h"
#include "ops.h"
#include "ovl.h"
#include "oct-hdf5.h"
#include "ov-range-traits.h"
#include "ov-range.h"
#include "ov-re-mat.h"
#include "ov-scalar.h"
#include "pr-output.h"

#include "byte-swap.h"
#include "ls-ascii-helper.h"
#include "ls-hdf5.h"
#include "ls-utils.h"

#if defined (HAVE_HDF5)

template <>
octave_hdf5_id ov_range<float>::hdf5_save_type = H5T_NATIVE_FLOAT;

template <>
octave_hdf5_id ov_range<double>::hdf5_save_type = H5T_NATIVE_DOUBLE;

template <>
octave_hdf5_id ov_range<octave_int8>::hdf5_save_type = H5T_NATIVE_INT8;

template <>
octave_hdf5_id ov_range<octave_int16>::hdf5_save_type = H5T_NATIVE_INT16;

template <>
octave_hdf5_id ov_range<octave_int32>::hdf5_save_type = H5T_NATIVE_INT32;

template <>
octave_hdf5_id ov_range<octave_int64>::hdf5_save_type = H5T_NATIVE_INT64;

template <>
octave_hdf5_id ov_range<octave_uint8>::hdf5_save_type = H5T_NATIVE_UINT8;

template <>
octave_hdf5_id ov_range<octave_uint16>::hdf5_save_type = H5T_NATIVE_UINT16;

template <>
octave_hdf5_id ov_range<octave_uint32>::hdf5_save_type = H5T_NATIVE_UINT32;

template <>
octave_hdf5_id ov_range<octave_uint64>::hdf5_save_type = H5T_NATIVE_UINT64;

#else

template <>
octave_hdf5_id ov_range<float>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<double>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_int8>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_int16>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_int32>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_int64>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_uint8>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_uint16>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_uint32>::hdf5_save_type = 0;

template <>
octave_hdf5_id ov_range<octave_uint64>::hdf5_save_type = 0;

#endif

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<float>,
                                              "float_range", "single");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<double>,
                                              "range", "double");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_int8>,
                                              "int8_range", "int8");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_int16>,
                                              "int16_range", "int16");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_int32>,
                                              "int32_range", "int32");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_int64>,
                                              "int64_range", "int64");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_uint8>,
                                              "uint8_range", "uint8");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_uint16>,
                                              "uint16_range", "uint16");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_uint32>,
                                              "uint32_range", "uint32");

DEFINE_TEMPLATE_OV_TYPEID_FUNCTIONS_AND_DATA (ov_range<octave_uint64>,
                                              "uint64_range", "uint64");

template <typename T>
static octave_base_value *
default_numeric_conversion_function (const octave_base_value& a)
{
  typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;

  const ov_range<T>& v = dynamic_cast<const ov_range<T>&> (a);

  return new ov_mx_type (v.raw_array_value ());
}

template <typename T>
octave_base_value::type_conv_info
ov_range<T>::numeric_conversion_function (void) const
{
  typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;

  return octave_base_value::type_conv_info
    (default_numeric_conversion_function<T>, ov_mx_type::static_type_id ());
}

template <typename T>
octave_base_value *
ov_range<T>::try_narrowing_conversion (void)
{
  octave_base_value *retval = nullptr;

  switch (numel ())
    {
    case 1:
      retval = new typename octave_value_range_traits<T>::scalar_type (m_range.elem (0));
      break;

    case 0:
      {
        typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;
        typename ov_mx_type::object_type m (dim_vector (1, 0));
        retval = new ov_mx_type (m);
      }
      break;

    case -2:
      // FIXME: is this case possible now?  It would have to be due to
      // conversion from Range to range<double>, but even in that case,
      // is the invalid numel value preserved?
      retval = new typename octave_value_range_traits<T>::matrix_type (raw_array_value ());
      break;

    default:
      break;
    }

  return retval;
}

template <typename T>
octave_value
ov_range<T>::subsref (const std::string& type,
                      const std::list<octave_value_list>& idx)
{
  octave_value retval;

  switch (type[0])
    {
    case '(':
      retval = do_index_op (idx.front ());
      break;

    case '{':
    case '.':
      {
        std::string nm = type_name ();
        error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
      }
      break;

    default:
      panic_impossible ();
    }

  return retval.next_subsref (type, idx);
}

template <typename T>
octave_value
ov_range<T>::do_index_op (const octave_value_list& idx,
                                   bool resize_ok)
{
  if (idx.length () == 1 && ! resize_ok)
    {
      octave_value retval;

      // The range can handle a single subscript.

      try
        {
          idx_vector i = idx(0).index_vector ();

          if (i.is_scalar () && i(0) < numel ())
            retval = m_range.elem (i(0));
          else
            retval = m_range.index (i);
        }
      catch (octave::index_exception& ie)
        {
          // More info may be added later before displaying error.

          ie.set_pos_if_unset (1, 1);
          throw;
        }

      return retval;
    }
  else
    {
      octave_value tmp (new typename octave_value_range_traits<T>::matrix_type (raw_array_value ()));

      return tmp.index_op (idx, resize_ok);
    }
}

template <typename T>
idx_vector
ov_range<T>::index_vector (bool require_integers) const
{
  octave_value tmp (raw_array_value ());
  return tmp.index_vector (require_integers);
}

template <typename T>
double
ov_range<T>::double_value (bool) const
{
  octave_idx_type nel = numel ();

  if (nel == 0)
    err_invalid_conversion ("range", "real scalar");

  warn_implicit_conversion ("Octave:array-to-scalar",
                            "range", "real scalar");

  return m_range.base ();
}

template <typename T>
float
ov_range<T>::float_value (bool) const
{
  octave_idx_type nel = numel ();

  if (nel == 0)
    err_invalid_conversion ("range", "real scalar");

  warn_implicit_conversion ("Octave:array-to-scalar",
                            "range", "real scalar");

  return m_range.base ();
}

template <typename T>
charNDArray
ov_range<T>::char_array_value (bool) const
{
  const Array<T> matrix = raw_array_value ();
  charNDArray retval (dims ());

  octave_idx_type nel = numel ();

  for (octave_idx_type i = 0; i < nel; i++)
    retval.elem (i) = static_cast<char> (matrix.elem (i));

  return retval;
}

template <typename T>
Complex
ov_range<T>::complex_value (bool) const
{
  octave_idx_type nel = numel ();

  if (nel == 0)
    err_invalid_conversion ("range", "complex scalar");

  warn_implicit_conversion ("Octave:array-to-scalar",
                            "range", "complex scalar");

  return Complex (m_range.base (), 0);
}

template <typename T>
FloatComplex
ov_range<T>::float_complex_value (bool) const
{
  float tmp = lo_ieee_float_nan_value ();

  FloatComplex retval (tmp, tmp);

  octave_idx_type nel = numel ();

  if (nel == 0)
    err_invalid_conversion ("range", "complex scalar");

  warn_implicit_conversion ("Octave:array-to-scalar",
                            "range", "complex scalar");

  retval = m_range.base ();

  return retval;
}

template <typename T>
boolNDArray
ov_range<T>::bool_array_value (bool warn) const
{
  Array<T> matrix = raw_array_value ();

  if (warn && ! matrix.test_all (xis_one_or_zero<T>))
    warn_logical_conversion ();

  return boolNDArray (matrix);
}

template <typename T>
octave_value
ov_range<T>::resize (const dim_vector& dv, bool fill) const
{
  Array<T> retval = raw_array_value ();
  if (fill)
    retval.resize (dv, 0);
  else
    retval.resize (dv);
  return retval;
}

template <typename T>
octave::range<float>
ov_range<T>::float_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::float_range_value ()", type_name ());
}

template <typename T>
octave::range<double>
ov_range<T>::range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::range_value()", type_name ());
}

template <typename T>
octave::range<octave_int8>
ov_range<T>::int8_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::int8_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_int16>
ov_range<T>::int16_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::int16_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_int32>
ov_range<T>::int32_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::int32_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_int64>
ov_range<T>::int64_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::int64_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_uint8>
ov_range<T>::uint8_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::uint8_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_uint16>
ov_range<T>::uint16_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::uint16_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_uint32>
ov_range<T>::uint32_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::uint32_range_value ()", type_name ());
}

template <typename T>
octave::range<octave_uint64>
ov_range<T>::uint64_range_value (void) const
{
  err_wrong_type_arg ("ov_range<T>::uint64_range_value ()", type_name ());
}

template <typename T>
octave_value
ov_range<T>::convert_to_str_internal (bool pad, bool force, char type) const
{
  octave_value tmp (raw_array_value ());
  return tmp.convert_to_str (pad, force, type);
}

// FIXME: could most of these fucntions preserve range type now?

template <typename T>
octave_value
ov_range<T>::as_double (void) const
{
  return NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_single (void) const
{
  return FloatMatrix (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_int8 (void) const
{
  return int8NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_int16 (void) const
{
  return int16NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_int32 (void) const
{
  return int32NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_int64 (void) const
{
  return int64NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_uint8 (void) const
{
  return uint8NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_uint16 (void) const
{
  return uint16NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_uint32 (void) const
{
  return uint32NDArray (raw_array_value ());
}

template <typename T>
octave_value
ov_range<T>::as_uint64 (void) const
{
  return uint64NDArray (raw_array_value ());
}

template <typename T>
void
ov_range<T>::print (std::ostream& os, bool pr_as_read_syntax)
{
  print_raw (os, pr_as_read_syntax);
  newline (os);
}

template <typename T>
void
ov_range<T>::print_raw (std::ostream& os, bool pr_as_read_syntax) const
{
  // FIXME: this is a potential waste of memory.

  typedef typename octave_value_range_traits<T>::matrix_type ov_mx_type;
  typename ov_mx_type::object_type tmp (raw_array_value ());

  octave_print_internal (os, tmp, pr_as_read_syntax,
                         current_print_indent_level ());
}

template <typename T>
bool
ov_range<T>::print_name_tag (std::ostream& os, const std::string& name) const
{
  bool retval = false;

  octave_idx_type n = numel ();

  indent (os);

  if (n == 0 || n == 1)
    os << name << " = ";
  else
    {
      os << name << " =";
      newline (os);
      if (! Vcompact_format)
        newline (os);

      retval = true;
    }

  return retval;
}

template <typename T>
void
ov_range<T>::short_disp (std::ostream& os) const
{
  octave_idx_type len = numel ();

  if (len == 0)
    os << "[]";
  else
    {
      os << m_range.base () << ':';

      if (len > 1)
        {
          if (m_range.increment () != T (1))
            os << m_range.increment () << ':';

          os << m_range.limit ();
        }
    }
}

// Skip white space and comments on stream IS.

static void
skip_comments (std::istream& is)
{
  char c = '\0';
  while (is.get (c))
    {
      if (c == ' ' || c == '\t' || c == '\n')
        ; // Skip whitespace on way to beginning of next line.
      else
        break;
    }

  skip_until_newline (is, false);
}

template <typename T>
float_display_format
ov_range<T>::get_edit_display_format (void) const
{
  return make_format (m_range);
}

template <typename T>
std::string
ov_range<T>::edit_display (const float_display_format& fmt,
                           octave_idx_type, octave_idx_type j) const
{
  std::ostringstream buf;
  octave_print_internal (buf, fmt, m_range.elem (j));
  return buf.str ();
}

template <typename T>
bool
ov_range<T>::save_ascii (std::ostream& os)
{
  octave::range<T> r = m_range;
  T base = r.base ();
  T limit = r.limit ();
  T inc = r.increment ();
  octave_idx_type len = r.numel ();

  if (inc != T (0))
    os << "# base, limit, increment\n";
  else
    os << "# base, length, increment\n";

  octave::write_value<T> (os, base);
  os << ' ';
  if (inc != T (0))
    octave::write_value<T> (os, limit);
  else
    os << len;
  os << ' ';
  octave::write_value<T> (os, inc);
  os << "\n";

  return true;
}

template <typename T>
bool
ov_range<T>::load_ascii (std::istream& is)
{
  // # base, limit, range comment added by save ().
  skip_comments (is);

  T base, limit, inc;
  is >> base >> limit >> inc;

  if (! is)
    error ("load: failed to load range constant");

  if (inc != T (0))
    m_range = octave::range<T> (base, limit, inc);
  else
    {
      octave_idx_type numel = static_cast<octave_idx_type> (limit);
      m_range = octave::range<T>::make_constant (base, numel);
    }

  return true;
}

template <typename T>
bool
ov_range<T>::save_binary (std::ostream& os, bool /* save_as_floats */)
{
  // FIXME: Not always double!

  char tmp = LS_DOUBLE;
  os.write (reinterpret_cast<char *> (&tmp), 1);
  octave::range<T> r = m_range;
  T bas = r.base ();
  T lim = r.limit ();
  T inc = r.increment ();
  if (inc == T (0))
    lim = r.numel ();

  os.write (reinterpret_cast<char *> (&bas), sizeof (T));
  os.write (reinterpret_cast<char *> (&lim), sizeof (T));
  os.write (reinterpret_cast<char *> (&inc), sizeof (T));

  return true;
}

template <typename T>
bool
ov_range<T>::load_binary (std::istream& is, bool swap,
                                   octave::mach_info::float_format /* fmt */)
{
  // FIXME: Not always double!

  char tmp;
  if (! is.read (reinterpret_cast<char *> (&tmp), 1))
    return false;
  T bas, lim, inc;
  if (! is.read (reinterpret_cast<char *> (&bas), sizeof (T)))
    return false;
  if (swap)
    swap_bytes<sizeof (T)> (&bas);
  if (! is.read (reinterpret_cast<char *> (&lim), sizeof (T)))
    return false;
  if (swap)
    swap_bytes<sizeof (T)> (&lim);
  if (! is.read (reinterpret_cast<char *> (&inc), sizeof (T)))
    return false;
  if (swap)
    swap_bytes<sizeof (T)> (&inc);
  if (inc != T (0))
    m_range = octave::range<T> (bas, inc, lim);
  else
    {
      octave_idx_type numel = static_cast<octave_idx_type> (lim);
      m_range = octave::range<T>::make_constant (bas, numel);
    }

  return true;
}

#if defined (HAVE_HDF5)

// The following subroutines creates an HDF5 representation of the way
// we will store Octave range types (triplets of floating-point numbers).
// NUM_TYPE is the HDF5 numeric type to use for storage (e.g.
// H5T_NATIVE_DOUBLE to save as 'double').  Note that any necessary
// conversions are handled automatically by HDF5.

template <typename T>
static hid_t
hdf5_make_range_type (hid_t num_type)
{
  hid_t type_id = H5Tcreate (H5T_COMPOUND, sizeof (T) * 3);

  H5Tinsert (type_id, "base", 0 * sizeof (T), num_type);
  H5Tinsert (type_id, "limit", 1 * sizeof (T), num_type);
  H5Tinsert (type_id, "increment", 2 * sizeof (T), num_type);

  return type_id;
}

#endif

template <typename T>
bool
ov_range<T>::save_hdf5 (octave_hdf5_id loc_id, const char *name,
                        bool /* save_as_floats */)
{
  bool retval = false;

#if defined (HAVE_HDF5)

  hsize_t dimens[3];
  hid_t space_hid, type_hid, data_hid;
  space_hid = type_hid = data_hid = -1;

  space_hid = H5Screate_simple (0, dimens, nullptr);
  if (space_hid < 0) return false;

  type_hid = hdf5_make_range_type<T> (hdf5_save_type);
  if (type_hid < 0)
    {
      H5Sclose (space_hid);
      return false;
    }
#if defined (HAVE_HDF5_18)
  data_hid = H5Dcreate (loc_id, name, type_hid, space_hid,
                        octave_H5P_DEFAULT, octave_H5P_DEFAULT, octave_H5P_DEFAULT);
#else
  data_hid = H5Dcreate (loc_id, name, type_hid, space_hid, octave_H5P_DEFAULT);
#endif
  if (data_hid < 0)
    {
      H5Sclose (space_hid);
      H5Tclose (type_hid);
      return false;
    }

  octave::range<T> r = m_range;
  T range_vals[3];
  range_vals[0] = r.base ();
  range_vals[1] = (r.increment () != T (0) ? r.limit () : r.numel ());
  range_vals[2] = r.increment ();

  if (H5Dwrite (data_hid, type_hid, octave_H5S_ALL, octave_H5S_ALL,
                octave_H5P_DEFAULT, range_vals)
      >= 0)
    {
      octave_idx_type nel = r.numel ();
      retval = hdf5_add_scalar_attr (data_hid, H5T_NATIVE_IDX,
                                     "OCTAVE_RANGE_NELEM", &nel) >= 0;
    }
  else
    retval = false;

  H5Dclose (data_hid);
  H5Tclose (type_hid);
  H5Sclose (space_hid);

#else
  octave_unused_parameter (loc_id);
  octave_unused_parameter (name);

  warn_save ("hdf5");
#endif

  return retval;
}

template <typename T>
bool
ov_range<T>::load_hdf5 (octave_hdf5_id loc_id, const char *name)
{
  bool retval = false;

#if defined (HAVE_HDF5)

#if defined (HAVE_HDF5_18)
  hid_t data_hid = H5Dopen (loc_id, name, octave_H5P_DEFAULT);
#else
  hid_t data_hid = H5Dopen (loc_id, name);
#endif
  hid_t type_hid = H5Dget_type (data_hid);

  hid_t range_type = hdf5_make_range_type<T> (hdf5_save_type);

  if (! hdf5_types_compatible (type_hid, range_type))
    {
      H5Tclose (range_type);
      H5Dclose (data_hid);
      return false;
    }

  hid_t space_hid = H5Dget_space (data_hid);
  hsize_t rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 0)
    {
      H5Tclose (range_type);
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      return false;
    }

  T rangevals[3];
  if (H5Dread (data_hid, range_type, octave_H5S_ALL, octave_H5S_ALL,
               octave_H5P_DEFAULT, rangevals)
      >= 0)
    {
      retval = true;

      // Don't use OCTAVE_RANGE_NELEM attribute, just reconstruct the range.

      if (rangevals[2] != T (0))
        m_range = octave::range<T> (rangevals[0], rangevals[1], rangevals[2]);
      else
        {
          octave_idx_type numel = static_cast<octave_idx_type> (rangevals[1]);
          m_range = octave::range<T>::make_constant (rangevals[0], numel);
        }
    }

  H5Tclose (range_type);
  H5Sclose (space_hid);
  H5Dclose (data_hid);

#else
  octave_unused_parameter (loc_id);
  octave_unused_parameter (name);

  warn_load ("hdf5");
#endif

  return retval;
}

template <typename T>
mxArray *
ov_range<T>::as_mxArray (bool interleaved) const
{
  mxClassID mx_class = mx_type_traits<T>::mx_class;

  mxArray *retval = new mxArray (interleaved, mx_class, dims (), mxREAL);

  typedef typename mx_type_traits<T>::mx_type mx_type;
  mx_type *pd = static_cast<mx_type *> (retval->get_data ());

  mwSize nel = numel ();

  Array<T> matrix = raw_array_value ();

  const T *pdata = matrix.data ();

  for (mwSize i = 0; i < nel; i++)
    pd[i] = pdata[i];

  return retval;
}

template <typename T>
octave_value
ov_range<T>::fast_elem_extract (octave_idx_type n) const
{
  return (n < numel () ? octave_value (m_range.elem (n)) : octave_value ());
}

// Specializations.

template <>
octave::range<float>
ov_range<float>::float_range_value (void) const
{
  return m_range;
}

template <>
octave::range<double>
ov_range<double>::range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_int8>
ov_range<octave_int8>::int8_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_int16>
ov_range<octave_int16>::int16_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_int32>
ov_range<octave_int32>::int32_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_int64>
ov_range<octave_int64>::int64_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_uint8>
ov_range<octave_uint8>::uint8_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_uint16>
ov_range<octave_uint16>::uint16_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_uint32>
ov_range<octave_uint32>::uint32_range_value (void) const
{
  return m_range;
}

template <>
octave::range<octave_uint64>
ov_range<octave_uint64>::uint64_range_value (void) const
{
  return m_range;
}

template <>
idx_vector
ov_range<double>::index_vector (bool require_integers) const
{
  if (m_idx_cache)
    return *m_idx_cache;

  if (require_integers || m_range.all_elements_are_ints ())
    return set_idx_cache (idx_vector (m_range));

  warning_with_id ("Octave:noninteger-range-as-index",
                   "non-integer range used as index");

  return octave_value (matrix_value ()).round ().index_vector ();
}

template <>
octave_idx_type
ov_range<double>::nnz (void) const
{
  return m_range.nnz ();
}

// The following specialization is also historical baggage.  For double
// ranges, we can produce special double-valued diagnoal matrix objects
// but Octave currently provides only double and Complex diagonal matrix
// objects.

template <>
octave_value
ov_range<double>::diag (octave_idx_type k) const
{
  // FIXME: this is a potential waste of memory.

  return
    (k == 0
     ? octave_value (DiagMatrix (DiagArray2<double> (matrix_value ())))
     : octave_value (m_range.diag (k)));
}

template <>
octave_value
ov_range<double>::diag (octave_idx_type nr, octave_idx_type nc) const
{
  Matrix mat = matrix_value ();

  return mat.diag (nr, nc);
}

template <>
void
ov_range<double>::print_raw (std::ostream& os, bool pr_as_read_syntax) const
{
  octave_print_internal (os, m_range, pr_as_read_syntax,
                         current_print_indent_level ());
}
