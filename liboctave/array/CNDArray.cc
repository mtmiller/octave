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

#include <complex>
#include <istream>
#include <ostream>

#include "Array-util.h"
#include "CNDArray.h"
#include "f77-fcn.h"
#include "lo-ieee.h"
#include "lo-mappers.h"
#include "mx-base.h"
#include "mx-cnda-s.h"
#include "mx-op-defs.h"
#include "oct-fftw.h"
#include "oct-locbuf.h"

#include "bsxfun-defs.cc"

ComplexNDArray::ComplexNDArray (const charNDArray& a)
  : MArray<Complex> (a.dims ())
{
  octave_idx_type n = a.numel ();
  for (octave_idx_type i = 0; i < n; i++)
    xelem (i) = static_cast<unsigned char> (a(i));
}

#if defined (HAVE_FFTW)

ComplexNDArray
ComplexNDArray::fourier (int dim) const
{
  dim_vector dv = dims ();

  if (dim > dv.ndims () || dim < 0)
    return ComplexNDArray ();

  octave_idx_type stride = 1;
  octave_idx_type n = dv(dim);

  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  octave_idx_type howmany = numel () / dv(dim);
  howmany = (stride == 1 ? howmany : (howmany > stride ? stride : howmany));
  octave_idx_type nloop = (stride == 1 ? 1 : numel () / dv(dim) / stride);
  octave_idx_type dist = (stride == 1 ? n : 1);

  const Complex *in (fortran_vec ());
  ComplexNDArray retval (dv);
  Complex *out (retval.fortran_vec ());

  // Need to be careful here about the distance between fft's
  for (octave_idx_type k = 0; k < nloop; k++)
    octave::fftw::fft (in + k * stride * n, out + k * stride * n,
                       n, howmany, stride, dist);

  return retval;
}

ComplexNDArray
ComplexNDArray::ifourier (int dim) const
{
  dim_vector dv = dims ();

  if (dim > dv.ndims () || dim < 0)
    return ComplexNDArray ();

  octave_idx_type stride = 1;
  octave_idx_type n = dv(dim);

  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  octave_idx_type howmany = numel () / dv(dim);
  howmany = (stride == 1 ? howmany : (howmany > stride ? stride : howmany));
  octave_idx_type nloop = (stride == 1 ? 1 : numel () / dv(dim) / stride);
  octave_idx_type dist = (stride == 1 ? n : 1);

  const Complex *in (fortran_vec ());
  ComplexNDArray retval (dv);
  Complex *out (retval.fortran_vec ());

  // Need to be careful here about the distance between fft's
  for (octave_idx_type k = 0; k < nloop; k++)
    octave::fftw::ifft (in + k * stride * n, out + k * stride * n,
                        n, howmany, stride, dist);

  return retval;
}

ComplexNDArray
ComplexNDArray::fourier2d (void) const
{
  dim_vector dv = dims ();
  if (dv.ndims () < 2)
    return ComplexNDArray ();

  dim_vector dv2 (dv(0), dv(1));
  const Complex *in = fortran_vec ();
  ComplexNDArray retval (dv);
  Complex *out = retval.fortran_vec ();
  octave_idx_type howmany = numel () / dv(0) / dv(1);
  octave_idx_type dist = dv(0) * dv(1);

  for (octave_idx_type i=0; i < howmany; i++)
    octave::fftw::fftNd (in + i*dist, out + i*dist, 2, dv2);

  return retval;
}

ComplexNDArray
ComplexNDArray::ifourier2d (void) const
{
  dim_vector dv = dims ();
  if (dv.ndims () < 2)
    return ComplexNDArray ();

  dim_vector dv2 (dv(0), dv(1));
  const Complex *in = fortran_vec ();
  ComplexNDArray retval (dv);
  Complex *out = retval.fortran_vec ();
  octave_idx_type howmany = numel () / dv(0) / dv(1);
  octave_idx_type dist = dv(0) * dv(1);

  for (octave_idx_type i=0; i < howmany; i++)
    octave::fftw::ifftNd (in + i*dist, out + i*dist, 2, dv2);

  return retval;
}

ComplexNDArray
ComplexNDArray::fourierNd (void) const
{
  dim_vector dv = dims ();
  int rank = dv.ndims ();

  const Complex *in (fortran_vec ());
  ComplexNDArray retval (dv);
  Complex *out (retval.fortran_vec ());

  octave::fftw::fftNd (in, out, rank, dv);

  return retval;
}

ComplexNDArray
ComplexNDArray::ifourierNd (void) const
{
  dim_vector dv = dims ();
  int rank = dv.ndims ();

  const Complex *in (fortran_vec ());
  ComplexNDArray retval (dv);
  Complex *out (retval.fortran_vec ());

  octave::fftw::ifftNd (in, out, rank, dv);

  return retval;
}

#else

ComplexNDArray
ComplexNDArray::fourier (int dim) const
{
  octave_unused_parameter (dim);

  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

ComplexNDArray
ComplexNDArray::ifourier (int dim) const
{
  octave_unused_parameter (dim);

  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

ComplexNDArray
ComplexNDArray::fourier2d (void) const
{
  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

ComplexNDArray
ComplexNDArray::ifourier2d (void) const
{
  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

ComplexNDArray
ComplexNDArray::fourierNd (void) const
{
  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

ComplexNDArray
ComplexNDArray::ifourierNd (void) const
{
  (*current_liboctave_error_handler)
    ("support for FFTW was unavailable or disabled when liboctave was built");

  return ComplexNDArray ();
}

#endif

// unary operations

boolNDArray
ComplexNDArray::operator ! (void) const
{
  if (any_element_is_nan ())
    octave::err_nan_to_logical_conversion ();

  return do_mx_unary_op<bool, Complex> (*this, mx_inline_not);
}

// FIXME: this is not quite the right thing.

bool
ComplexNDArray::any_element_is_nan (void) const
{
  return do_mx_check<Complex> (*this, mx_inline_any_nan);
}

bool
ComplexNDArray::any_element_is_inf_or_nan (void) const
{
  return ! do_mx_check<Complex> (*this, mx_inline_all_finite);
}

// Return true if no elements have imaginary components.

bool
ComplexNDArray::all_elements_are_real (void) const
{
  return do_mx_check<Complex> (*this, mx_inline_all_real);
}

// Return nonzero if any element of CM has a non-integer real or
// imaginary part.  Also extract the largest and smallest (real or
// imaginary) values and return them in MAX_VAL and MIN_VAL.

bool
ComplexNDArray::all_integers (double& max_val, double& min_val) const
{
  octave_idx_type nel = numel ();

  if (nel > 0)
    {
      Complex val = elem (0);

      double r_val = val.real ();
      double i_val = val.imag ();

      max_val = r_val;
      min_val = r_val;

      if (i_val > max_val)
        max_val = i_val;

      if (i_val < max_val)
        min_val = i_val;
    }
  else
    return false;

  for (octave_idx_type i = 0; i < nel; i++)
    {
      Complex val = elem (i);

      double r_val = val.real ();
      double i_val = val.imag ();

      if (r_val > max_val)
        max_val = r_val;

      if (i_val > max_val)
        max_val = i_val;

      if (r_val < min_val)
        min_val = r_val;

      if (i_val < min_val)
        min_val = i_val;

      if (octave::math::x_nint (r_val) != r_val
          || octave::math::x_nint (i_val) != i_val)
        return false;
    }

  return true;
}

bool
ComplexNDArray::too_large_for_float (void) const
{
  return test_any (xtoo_large_for_float);
}

boolNDArray
ComplexNDArray::all (int dim) const
{
  return do_mx_red_op<bool, Complex> (*this, dim, mx_inline_all);
}

boolNDArray
ComplexNDArray::any (int dim) const
{
  return do_mx_red_op<bool, Complex> (*this, dim, mx_inline_any);
}

ComplexNDArray
ComplexNDArray::cumprod (int dim) const
{
  return do_mx_cum_op<Complex, Complex> (*this, dim, mx_inline_cumprod);
}

ComplexNDArray
ComplexNDArray::cumsum (int dim) const
{
  return do_mx_cum_op<Complex, Complex> (*this, dim, mx_inline_cumsum);
}

ComplexNDArray
ComplexNDArray::prod (int dim) const
{
  return do_mx_red_op<Complex, Complex> (*this, dim, mx_inline_prod);
}

ComplexNDArray
ComplexNDArray::sum (int dim) const
{
  return do_mx_red_op<Complex, Complex> (*this, dim, mx_inline_sum);
}

ComplexNDArray
ComplexNDArray::xsum (int dim) const
{
  return do_mx_red_op<Complex, Complex> (*this, dim, mx_inline_xsum);
}

ComplexNDArray
ComplexNDArray::sumsq (int dim) const
{
  return do_mx_red_op<double, Complex> (*this, dim, mx_inline_sumsq);
}

ComplexNDArray
ComplexNDArray::diff (octave_idx_type order, int dim) const
{
  return do_mx_diff_op<Complex> (*this, dim, order, mx_inline_diff);
}

ComplexNDArray
ComplexNDArray::concat (const ComplexNDArray& rb,
                        const Array<octave_idx_type>& ra_idx)
{
  if (rb.numel () > 0)
    insert (rb, ra_idx);
  return *this;
}

ComplexNDArray
ComplexNDArray::concat (const NDArray& rb, const Array<octave_idx_type>& ra_idx)
{
  ComplexNDArray tmp (rb);
  if (rb.numel () > 0)
    insert (tmp, ra_idx);
  return *this;
}

ComplexNDArray
concat (NDArray& ra, ComplexNDArray& rb, const Array<octave_idx_type>& ra_idx)
{
  ComplexNDArray retval (ra);
  if (rb.numel () > 0)
    retval.insert (rb, ra_idx);
  return retval;
}

static const Complex Complex_NaN_result (octave::numeric_limits<double>::NaN (),
                                         octave::numeric_limits<double>::NaN ());

ComplexNDArray
ComplexNDArray::max (int dim) const
{
  return do_mx_minmax_op<Complex> (*this, dim, mx_inline_max);
}

ComplexNDArray
ComplexNDArray::max (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_minmax_op<Complex> (*this, idx_arg, dim, mx_inline_max);
}

ComplexNDArray
ComplexNDArray::min (int dim) const
{
  return do_mx_minmax_op<Complex> (*this, dim, mx_inline_min);
}

ComplexNDArray
ComplexNDArray::min (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_minmax_op<Complex> (*this, idx_arg, dim, mx_inline_min);
}

ComplexNDArray
ComplexNDArray::cummax (int dim) const
{
  return do_mx_cumminmax_op<Complex> (*this, dim, mx_inline_cummax);
}

ComplexNDArray
ComplexNDArray::cummax (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_cumminmax_op<Complex> (*this, idx_arg, dim, mx_inline_cummax);
}

ComplexNDArray
ComplexNDArray::cummin (int dim) const
{
  return do_mx_cumminmax_op<Complex> (*this, dim, mx_inline_cummin);
}

ComplexNDArray
ComplexNDArray::cummin (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_cumminmax_op<Complex> (*this, idx_arg, dim, mx_inline_cummin);
}

NDArray
ComplexNDArray::abs (void) const
{
  return do_mx_unary_map<double, Complex, std::abs> (*this);
}

boolNDArray
ComplexNDArray::isnan (void) const
{
  return do_mx_unary_map<bool, Complex, octave::math::isnan> (*this);
}

boolNDArray
ComplexNDArray::isinf (void) const
{
  return do_mx_unary_map<bool, Complex, octave::math::isinf> (*this);
}

boolNDArray
ComplexNDArray::isfinite (void) const
{
  return do_mx_unary_map<bool, Complex, octave::math::isfinite> (*this);
}

ComplexNDArray
conj (const ComplexNDArray& a)
{
  return do_mx_unary_map<Complex, Complex, std::conj<double>> (a);
}

ComplexNDArray&
ComplexNDArray::insert (const NDArray& a, octave_idx_type r, octave_idx_type c)
{
  dim_vector a_dv = a.dims ();

  int n = a_dv.ndims ();

  if (n != dimensions.ndims ())
    (*current_liboctave_error_handler)
      ("Array<T>::insert: invalid indexing operation");

  Array<octave_idx_type> a_ra_idx (dim_vector (a_dv.ndims (), 1), 0);

  a_ra_idx.elem (0) = r;
  a_ra_idx.elem (1) = c;

  for (int i = 0; i < n; i++)
    {
      if (a_ra_idx(i) < 0 || (a_ra_idx(i) + a_dv(i)) > dimensions(i))
        (*current_liboctave_error_handler)
          ("Array<T>::insert: range error for insert");
    }

  a_ra_idx.elem (0) = 0;
  a_ra_idx.elem (1) = 0;

  octave_idx_type n_elt = a.numel ();

  // IS make_unique () NECESSARY HERE?

  for (octave_idx_type i = 0; i < n_elt; i++)
    {
      Array<octave_idx_type> ra_idx = a_ra_idx;

      ra_idx.elem (0) = a_ra_idx(0) + r;
      ra_idx.elem (1) = a_ra_idx(1) + c;

      elem (ra_idx) = a.elem (a_ra_idx);

      increment_index (a_ra_idx, a_dv);
    }

  return *this;
}

ComplexNDArray&
ComplexNDArray::insert (const ComplexNDArray& a,
                        octave_idx_type r, octave_idx_type c)
{
  Array<Complex>::insert (a, r, c);
  return *this;
}

ComplexNDArray&
ComplexNDArray::insert (const ComplexNDArray& a,
                        const Array<octave_idx_type>& ra_idx)
{
  Array<Complex>::insert (a, ra_idx);
  return *this;
}

void
ComplexNDArray::increment_index (Array<octave_idx_type>& ra_idx,
                                 const dim_vector& dimensions,
                                 int start_dimension)
{
  ::increment_index (ra_idx, dimensions, start_dimension);
}

octave_idx_type
ComplexNDArray::compute_index (Array<octave_idx_type>& ra_idx,
                               const dim_vector& dimensions)
{
  return ::compute_index (ra_idx, dimensions);
}

ComplexNDArray
ComplexNDArray::diag (octave_idx_type k) const
{
  return MArray<Complex>::diag (k);
}

ComplexNDArray
ComplexNDArray::diag (octave_idx_type m, octave_idx_type n) const
{
  return MArray<Complex>::diag (m, n);
}

// This contains no information on the array structure !!!
std::ostream&
operator << (std::ostream& os, const ComplexNDArray& a)
{
  octave_idx_type nel = a.numel ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
      os << ' ';
      octave::write_value<Complex> (os, a.elem (i));
      os << "\n";
    }
  return os;
}

std::istream&
operator >> (std::istream& is, ComplexNDArray& a)
{
  octave_idx_type nel = a.numel ();

  if (nel > 0)
    {
      Complex tmp;
      for (octave_idx_type i = 0; i < nel; i++)
        {
          tmp = octave::read_value<Complex> (is);
          if (is)
            a.elem (i) = tmp;
          else
            return is;
        }
    }

  return is;
}

MINMAX_FCNS (ComplexNDArray, Complex)

NDS_CMP_OPS (ComplexNDArray, Complex)
NDS_BOOL_OPS (ComplexNDArray, Complex)

SND_CMP_OPS (Complex, ComplexNDArray)
SND_BOOL_OPS (Complex, ComplexNDArray)

NDND_CMP_OPS (ComplexNDArray, ComplexNDArray)
NDND_BOOL_OPS (ComplexNDArray, ComplexNDArray)

ComplexNDArray& operator *= (ComplexNDArray& a, double s)
{
  if (a.is_shared ())
    a = a * s;
  else
    do_ms_inplace_op<Complex, double> (a, s, mx_inline_mul2);
  return a;
}

ComplexNDArray& operator /= (ComplexNDArray& a, double s)
{
  if (a.is_shared ())
    return a = a / s;
  else
    do_ms_inplace_op<Complex, double> (a, s, mx_inline_div2);
  return a;
}

BSXFUN_STDOP_DEFS_MXLOOP (ComplexNDArray)
BSXFUN_STDREL_DEFS_MXLOOP (ComplexNDArray)

BSXFUN_OP_DEF_MXLOOP (pow, ComplexNDArray, mx_inline_pow)
