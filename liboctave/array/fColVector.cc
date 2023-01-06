////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2023 The Octave Project Developers
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

#include "Array-util.h"
#include "lo-blas-proto.h"
#include "lo-error.h"
#include "mx-base.h"
#include "mx-inlines.cc"
#include "oct-cmplx.h"

// Column Vector class.

bool
FloatColumnVector::operator == (const FloatColumnVector& a) const
{
  octave_idx_type len = numel ();
  if (len != a.numel ())
    return 0;
  return mx_inline_equal (len, data (), a.data ());
}

bool
FloatColumnVector::operator != (const FloatColumnVector& a) const
{
  return !(*this == a);
}

FloatColumnVector&
FloatColumnVector::insert (const FloatColumnVector& a, octave_idx_type r)
{
  octave_idx_type a_len = a.numel ();

  if (r < 0 || r + a_len > numel ())
    (*current_liboctave_error_handler) ("range error for insert");

  if (a_len > 0)
    {
      make_unique ();

      for (octave_idx_type i = 0; i < a_len; i++)
        xelem (r+i) = a.elem (i);
    }

  return *this;
}

FloatColumnVector&
FloatColumnVector::fill (float val)
{
  octave_idx_type len = numel ();

  if (len > 0)
    {
      make_unique ();

      for (octave_idx_type i = 0; i < len; i++)
        xelem (i) = val;
    }

  return *this;
}

FloatColumnVector&
FloatColumnVector::fill (float val, octave_idx_type r1, octave_idx_type r2)
{
  octave_idx_type len = numel ();

  if (r1 < 0 || r2 < 0 || r1 >= len || r2 >= len)
    (*current_liboctave_error_handler) ("range error for fill");

  if (r1 > r2) { std::swap (r1, r2); }

  if (r2 >= r1)
    {
      make_unique ();

      for (octave_idx_type i = r1; i <= r2; i++)
        xelem (i) = val;
    }

  return *this;
}

FloatColumnVector
FloatColumnVector::stack (const FloatColumnVector& a) const
{
  octave_idx_type len = numel ();
  octave_idx_type nr_insert = len;
  FloatColumnVector retval (len + a.numel ());
  retval.insert (*this, 0);
  retval.insert (a, nr_insert);
  return retval;
}

FloatRowVector
FloatColumnVector::transpose (void) const
{
  return MArray<float>::transpose ();
}

FloatColumnVector
FloatColumnVector::abs (void) const
{
  return do_mx_unary_map<float, float, std::abs> (*this);
}

FloatColumnVector
real (const FloatComplexColumnVector& a)
{
  return do_mx_unary_op<float, FloatComplex> (a, mx_inline_real);
}

FloatColumnVector
imag (const FloatComplexColumnVector& a)
{
  return do_mx_unary_op<float, FloatComplex> (a, mx_inline_imag);
}

// resize is the destructive equivalent for this one

FloatColumnVector
FloatColumnVector::extract (octave_idx_type r1, octave_idx_type r2) const
{
  if (r1 > r2) { std::swap (r1, r2); }

  octave_idx_type new_r = r2 - r1 + 1;

  FloatColumnVector result (new_r);

  for (octave_idx_type i = 0; i < new_r; i++)
    result.xelem (i) = elem (r1+i);

  return result;
}

FloatColumnVector
FloatColumnVector::extract_n (octave_idx_type r1, octave_idx_type n) const
{
  FloatColumnVector result (n);

  for (octave_idx_type i = 0; i < n; i++)
    result.xelem (i) = elem (r1+i);

  return result;
}

// matrix by column vector -> column vector operations

FloatColumnVector
operator * (const FloatMatrix& m, const FloatColumnVector& a)
{
  FloatColumnVector retval;

  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  retval.clear (nr);

  if (nr != 0)
    {
      if (nc == 0)
        retval.fill (0.0);
      else
        {
          float *y = retval.fortran_vec ();

          F77_XFCN (sgemv, SGEMV, (F77_CONST_CHAR_ARG2 ("N", 1),
                                   nr, nc, 1.0f, m.data (), nr,
                                   a.data (), 1, 0.0f, y, 1
                                   F77_CHAR_ARG_LEN (1)));
        }
    }

  return retval;
}

// diagonal matrix by column vector -> column vector operations

FloatColumnVector
operator * (const FloatDiagMatrix& m, const FloatColumnVector& a)
{
  FloatColumnVector retval;

  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nr == 0 || nc == 0)
    retval.resize (nr, 0.0);
  else
    {
      retval.resize (nr);

      for (octave_idx_type i = 0; i < a_len; i++)
        retval.elem (i) = a.elem (i) * m.elem (i, i);

      for (octave_idx_type i = a_len; i < nr; i++)
        retval.elem (i) = 0.0;
    }

  return retval;
}

// other operations

float
FloatColumnVector::min (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  float res = elem (0);

  for (octave_idx_type i = 1; i < len; i++)
    if (elem (i) < res)
      res = elem (i);

  return res;
}

float
FloatColumnVector::max (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  float res = elem (0);

  for (octave_idx_type i = 1; i < len; i++)
    if (elem (i) > res)
      res = elem (i);

  return res;
}

std::ostream&
operator << (std::ostream& os, const FloatColumnVector& a)
{
//  int field_width = os.precision () + 7;
  for (octave_idx_type i = 0; i < a.numel (); i++)
    os << /* setw (field_width) << */ a.elem (i) << "\n";
  return os;
}

std::istream&
operator >> (std::istream& is, FloatColumnVector& a)
{
  octave_idx_type len = a.numel ();

  if (len > 0)
    {
      float tmp;
      for (octave_idx_type i = 0; i < len; i++)
        {
          is >> tmp;
          if (is)
            a.elem (i) = tmp;
          else
            break;
        }
    }
  return is;
}
