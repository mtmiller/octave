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

#include <istream>
#include <ostream>

#include "lo-ieee.h"
#include "lo-specfun.h"
#include "lo-mappers.h"

#include "mxarray.h"
#include "ovl.h"
#include "oct-hdf5.h"
#include "oct-stream.h"
#include "ops.h"
#include "ov-complex.h"
#include "ov-base.h"
#include "ov-base-scalar.h"
#include "ov-base-scalar.cc"
#include "ov-flt-cx-mat.h"
#include "ov-float.h"
#include "ov-flt-complex.h"
#include "errwarn.h"
#include "pr-output.h"
#include "ops.h"

#include "ls-oct-text.h"
#include "ls-hdf5.h"

// Prevent implicit instantiations on some systems (Windows, others?)
// that can lead to duplicate definitions of static data members.

extern template class OCTINTERP_API octave_base_scalar<float>;

template class octave_base_scalar<FloatComplex>;

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_float_complex,
                                     "float complex scalar", "single");

octave_base_value *
octave_float_complex::try_narrowing_conversion (void)
{
  octave_base_value *retval = nullptr;

  float im = scalar.imag ();

  if (im == 0.0)
    retval = new octave_float_scalar (scalar.real ());

  return retval;
}

octave_value
octave_float_complex::do_index_op (const octave_value_list& idx, bool resize_ok)
{
  // FIXME: this doesn't solve the problem of
  //
  //   a = i; a([1,1], [1,1], [1,1])
  //
  // and similar constructions.  Hmm...

  // FIXME: using this constructor avoids narrowing the
  // 1x1 matrix back to a scalar value.  Need a better solution
  // to this problem.

  octave_value tmp (new octave_float_complex_matrix (float_complex_matrix_value ()));

  return tmp.do_index_op (idx, resize_ok);
}

double
octave_float_complex::double_value (bool force_conversion) const
{
  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real scalar");

  return scalar.real ();
}

float
octave_float_complex::float_value (bool force_conversion) const
{
  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real scalar");

  return scalar.real ();
}

Matrix
octave_float_complex::matrix_value (bool force_conversion) const
{
  Matrix retval;

  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real matrix");

  retval = Matrix (1, 1, scalar.real ());

  return retval;
}

FloatMatrix
octave_float_complex::float_matrix_value (bool force_conversion) const
{
  FloatMatrix retval;

  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real matrix");

  retval = FloatMatrix (1, 1, scalar.real ());

  return retval;
}

NDArray
octave_float_complex::array_value (bool force_conversion) const
{
  NDArray retval;

  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real matrix");

  retval = NDArray (dim_vector (1, 1), scalar.real ());

  return retval;
}

FloatNDArray
octave_float_complex::float_array_value (bool force_conversion) const
{
  FloatNDArray retval;

  if (! force_conversion)
    warn_implicit_conversion ("Octave:imag-to-real",
                              "complex scalar", "real matrix");

  retval = FloatNDArray (dim_vector (1, 1), scalar.real ());

  return retval;
}

Complex
octave_float_complex::complex_value (bool) const
{
  return scalar;
}

FloatComplex
octave_float_complex::float_complex_value (bool) const
{
  return static_cast<FloatComplex> (scalar);
}

ComplexMatrix
octave_float_complex::complex_matrix_value (bool) const
{
  return ComplexMatrix (1, 1, scalar);
}

FloatComplexMatrix
octave_float_complex::float_complex_matrix_value (bool) const
{
  return FloatComplexMatrix (1, 1, scalar);
}

ComplexNDArray
octave_float_complex::complex_array_value (bool /* force_conversion */) const
{
  return ComplexNDArray (dim_vector (1, 1), scalar);
}

FloatComplexNDArray
octave_float_complex::float_complex_array_value (bool /* force_conversion */) const
{
  return FloatComplexNDArray (dim_vector (1, 1), scalar);
}

octave_value
octave_float_complex::resize (const dim_vector& dv, bool fill) const
{
  if (fill)
    {
      FloatComplexNDArray retval (dv, FloatComplex (0));

      if (dv.numel ())
        retval(0) = scalar;

      return retval;
    }
  else
    {
      FloatComplexNDArray retval (dv);

      if (dv.numel ())
        retval(0) = scalar;

      return retval;
    }
}

octave_value
octave_float_complex::as_double (void) const
{
  return Complex (scalar);
}

octave_value
octave_float_complex::as_single (void) const
{
  return scalar;
}

octave_value
octave_float_complex::diag (octave_idx_type m, octave_idx_type n) const
{
  return
    FloatComplexDiagMatrix (Array<FloatComplex> (dim_vector (1, 1), scalar),
                            m, n);
}

bool
octave_float_complex::save_ascii (std::ostream& os)
{
  FloatComplex c = float_complex_value ();

  octave::write_value<FloatComplex> (os, c);

  os << "\n";

  return true;
}

bool
octave_float_complex::load_ascii (std::istream& is)
{
  scalar = octave::read_value<FloatComplex> (is);

  if (! is)
    error ("load: failed to load complex scalar constant");

  return true;
}

bool
octave_float_complex::save_binary (std::ostream& os, bool /* save_as_floats */)
{
  char tmp = static_cast<char> (LS_FLOAT);
  os.write (reinterpret_cast<char *> (&tmp), 1);
  FloatComplex ctmp = float_complex_value ();
  os.write (reinterpret_cast<char *> (&ctmp), 8);

  return true;
}

bool
octave_float_complex::load_binary (std::istream& is, bool swap,
                                   octave::mach_info::float_format fmt)
{
  char tmp;
  if (! is.read (reinterpret_cast<char *> (&tmp), 1))
    return false;

  FloatComplex ctmp;
  read_floats (is, reinterpret_cast<float *> (&ctmp),
               static_cast<save_type> (tmp), 2, swap, fmt);

  if (! is)
    return false;

  scalar = ctmp;
  return true;
}

bool
octave_float_complex::save_hdf5 (octave_hdf5_id loc_id, const char *name,
                                 bool /* save_as_floats */)
{
  bool retval = false;

#if defined (HAVE_HDF5)

  hsize_t dimens[3];
  hid_t space_hid, type_hid, data_hid;
  space_hid = type_hid = data_hid = -1;

  space_hid = H5Screate_simple (0, dimens, nullptr);
  if (space_hid < 0)
    return false;

  type_hid = hdf5_make_complex_type (H5T_NATIVE_FLOAT);
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

  FloatComplex tmp = float_complex_value ();
  retval = H5Dwrite (data_hid, type_hid, octave_H5S_ALL, octave_H5S_ALL,
                     octave_H5P_DEFAULT, &tmp) >= 0;

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

bool
octave_float_complex::load_hdf5 (octave_hdf5_id loc_id, const char *name)
{
  bool retval = false;

#if defined (HAVE_HDF5)

#if defined (HAVE_HDF5_18)
  hid_t data_hid = H5Dopen (loc_id, name, octave_H5P_DEFAULT);
#else
  hid_t data_hid = H5Dopen (loc_id, name);
#endif
  hid_t type_hid = H5Dget_type (data_hid);

  hid_t complex_type = hdf5_make_complex_type (H5T_NATIVE_FLOAT);

  if (! hdf5_types_compatible (type_hid, complex_type))
    {
      H5Tclose (complex_type);
      H5Dclose (data_hid);
      return false;
    }

  hid_t space_id = H5Dget_space (data_hid);
  hsize_t rank = H5Sget_simple_extent_ndims (space_id);

  if (rank != 0)
    {
      H5Tclose (complex_type);
      H5Sclose (space_id);
      H5Dclose (data_hid);
      return false;
    }

  // complex scalar:
  FloatComplex ctmp;
  if (H5Dread (data_hid, complex_type, octave_H5S_ALL, octave_H5S_ALL,
               octave_H5P_DEFAULT, &ctmp)
      >= 0)
    {
      retval = true;
      scalar = ctmp;
    }

  H5Tclose (complex_type);
  H5Sclose (space_id);
  H5Dclose (data_hid);

#else
  octave_unused_parameter (loc_id);
  octave_unused_parameter (name);

  warn_load ("hdf5");
#endif

  return retval;
}

mxArray *
octave_float_complex::as_mxArray (bool interleaved) const
{
  mxArray *retval = new mxArray (interleaved, mxSINGLE_CLASS, 1, 1, mxCOMPLEX);

  if (interleaved)
    {
      mxComplexSingle *pd
        = static_cast<mxComplexSingle *> (retval->get_data ());

      pd[0].real = scalar.real ();
      pd[0].imag = scalar.imag ();
    }
  else
    {
      mxSingle *pr = static_cast<mxSingle *> (retval->get_data ());
      mxSingle *pi = static_cast<mxSingle *> (retval->get_imag_data ());

      pr[0] = scalar.real ();
      pi[0] = scalar.imag ();
    }

  return retval;
}

octave_value
octave_float_complex::map (unary_mapper_t umap) const
{
  switch (umap)
    {
#define SCALAR_MAPPER(UMAP, FCN)              \
    case umap_ ## UMAP:                       \
      return octave_value (FCN (scalar))

    SCALAR_MAPPER (abs, std::abs);
    SCALAR_MAPPER (acos, octave::math::acos);
    SCALAR_MAPPER (acosh, octave::math::acosh);
    SCALAR_MAPPER (angle, std::arg);
    SCALAR_MAPPER (arg, std::arg);
    SCALAR_MAPPER (asin, octave::math::asin);
    SCALAR_MAPPER (asinh, octave::math::asinh);
    SCALAR_MAPPER (atan, octave::math::atan);
    SCALAR_MAPPER (atanh, octave::math::atanh);
    SCALAR_MAPPER (erf, octave::math::erf);
    SCALAR_MAPPER (erfc, octave::math::erfc);
    SCALAR_MAPPER (erfcx, octave::math::erfcx);
    SCALAR_MAPPER (erfi, octave::math::erfi);
    SCALAR_MAPPER (dawson, octave::math::dawson);
    SCALAR_MAPPER (ceil, octave::math::ceil);
    SCALAR_MAPPER (conj, std::conj);
    SCALAR_MAPPER (cos, std::cos);
    SCALAR_MAPPER (cosh, std::cosh);
    SCALAR_MAPPER (exp, std::exp);
    SCALAR_MAPPER (expm1, octave::math::expm1);
    SCALAR_MAPPER (fix, octave::math::fix);
    SCALAR_MAPPER (floor, octave::math::floor);
    SCALAR_MAPPER (imag, std::imag);
    SCALAR_MAPPER (log, std::log);
    SCALAR_MAPPER (log2, octave::math::log2);
    SCALAR_MAPPER (log10, std::log10);
    SCALAR_MAPPER (log1p, octave::math::log1p);
    SCALAR_MAPPER (real, std::real);
    SCALAR_MAPPER (round, octave::math::round);
    SCALAR_MAPPER (roundb, octave::math::roundb);
    SCALAR_MAPPER (signum, octave::math::signum);
    SCALAR_MAPPER (sin, std::sin);
    SCALAR_MAPPER (sinh, std::sinh);
    SCALAR_MAPPER (sqrt, std::sqrt);
    SCALAR_MAPPER (tan, std::tan);
    SCALAR_MAPPER (tanh, std::tanh);
    SCALAR_MAPPER (isfinite, octave::math::isfinite);
    SCALAR_MAPPER (isinf, octave::math::isinf);
    SCALAR_MAPPER (isna, octave::math::isna);
    SCALAR_MAPPER (isnan, octave::math::isnan);

    // Special cases for Matlab compatibility
    case umap_xtolower:
    case umap_xtoupper:
      return scalar;

    default:
      return octave_base_value::map (umap);
    }
}
