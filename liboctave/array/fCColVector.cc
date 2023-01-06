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

// FloatComplex Column Vector class

FloatComplexColumnVector::FloatComplexColumnVector (const FloatColumnVector& a)
  : MArray<FloatComplex> (a)
{ }

bool
FloatComplexColumnVector::operator == (const FloatComplexColumnVector& a) const
{
  octave_idx_type len = numel ();
  if (len != a.numel ())
    return 0;
  return mx_inline_equal (len, data (), a.data ());
}

bool
FloatComplexColumnVector::operator != (const FloatComplexColumnVector& a) const
{
  return !(*this == a);
}

// destructive insert/delete/reorder operations

FloatComplexColumnVector&
FloatComplexColumnVector::insert (const FloatColumnVector& a, octave_idx_type r)
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

FloatComplexColumnVector&
FloatComplexColumnVector::insert (const FloatComplexColumnVector& a,
                                  octave_idx_type r)
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

FloatComplexColumnVector&
FloatComplexColumnVector::fill (float val)
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

FloatComplexColumnVector&
FloatComplexColumnVector::fill (const FloatComplex& val)
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

FloatComplexColumnVector&
FloatComplexColumnVector::fill (float val,
                                octave_idx_type r1, octave_idx_type r2)
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

FloatComplexColumnVector&
FloatComplexColumnVector::fill (const FloatComplex& val,
                                octave_idx_type r1, octave_idx_type r2)
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

FloatComplexColumnVector
FloatComplexColumnVector::stack (const FloatColumnVector& a) const
{
  octave_idx_type len = numel ();
  octave_idx_type nr_insert = len;
  FloatComplexColumnVector retval (len + a.numel ());
  retval.insert (*this, 0);
  retval.insert (a, nr_insert);
  return retval;
}

FloatComplexColumnVector
FloatComplexColumnVector::stack (const FloatComplexColumnVector& a) const
{
  octave_idx_type len = numel ();
  octave_idx_type nr_insert = len;
  FloatComplexColumnVector retval (len + a.numel ());
  retval.insert (*this, 0);
  retval.insert (a, nr_insert);
  return retval;
}

FloatComplexRowVector
FloatComplexColumnVector::hermitian (void) const
{
  return MArray<FloatComplex>::hermitian (std::conj);
}

FloatComplexRowVector
FloatComplexColumnVector::transpose (void) const
{
  return MArray<FloatComplex>::transpose ();
}

FloatColumnVector
FloatComplexColumnVector::abs (void) const
{
  return do_mx_unary_map<float, FloatComplex, std::abs> (*this);
}

FloatComplexColumnVector
conj (const FloatComplexColumnVector& a)
{
  return do_mx_unary_map<FloatComplex, FloatComplex, std::conj<float>> (a);
}

// resize is the destructive equivalent for this one

FloatComplexColumnVector
FloatComplexColumnVector::extract (octave_idx_type r1, octave_idx_type r2) const
{
  if (r1 > r2) { std::swap (r1, r2); }

  octave_idx_type new_r = r2 - r1 + 1;

  FloatComplexColumnVector result (new_r);

  for (octave_idx_type i = 0; i < new_r; i++)
    result.elem (i) = elem (r1+i);

  return result;
}

FloatComplexColumnVector
FloatComplexColumnVector::extract_n (octave_idx_type r1,
                                     octave_idx_type n) const
{
  FloatComplexColumnVector result (n);

  for (octave_idx_type i = 0; i < n; i++)
    result.elem (i) = elem (r1+i);

  return result;
}

// column vector by column vector -> column vector operations

FloatComplexColumnVector&
FloatComplexColumnVector::operator += (const FloatColumnVector& a)
{
  octave_idx_type len = numel ();

  octave_idx_type a_len = a.numel ();

  if (len != a_len)
    octave::err_nonconformant ("operator +=", len, a_len);

  if (len == 0)
    return *this;

  FloatComplex *d = fortran_vec (); // Ensures only 1 reference to my privates!

  mx_inline_add2 (len, d, a.data ());
  return *this;
}

FloatComplexColumnVector&
FloatComplexColumnVector::operator -= (const FloatColumnVector& a)
{
  octave_idx_type len = numel ();

  octave_idx_type a_len = a.numel ();

  if (len != a_len)
    octave::err_nonconformant ("operator -=", len, a_len);

  if (len == 0)
    return *this;

  FloatComplex *d = fortran_vec (); // Ensures only 1 reference to my privates!

  mx_inline_sub2 (len, d, a.data ());
  return *this;
}

// matrix by column vector -> column vector operations

FloatComplexColumnVector
operator * (const FloatComplexMatrix& m, const FloatColumnVector& a)
{
  FloatComplexColumnVector tmp (a);
  return m * tmp;
}

FloatComplexColumnVector
operator * (const FloatComplexMatrix& m, const FloatComplexColumnVector& a)
{
  FloatComplexColumnVector retval;

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
          FloatComplex *y = retval.fortran_vec ();

          F77_XFCN (cgemv, CGEMV, (F77_CONST_CHAR_ARG2 ("N", 1),
                                   nr, nc, 1.0f, F77_CONST_CMPLX_ARG (m.data ()), nr,
                                   F77_CONST_CMPLX_ARG (a.data ()), 1, 0.0f, F77_CMPLX_ARG (y), 1
                                   F77_CHAR_ARG_LEN (1)));
        }
    }

  return retval;
}

// matrix by column vector -> column vector operations

FloatComplexColumnVector
operator * (const FloatMatrix& m, const FloatComplexColumnVector& a)
{
  FloatComplexMatrix tmp (m);
  return tmp * a;
}

// diagonal matrix by column vector -> column vector operations

FloatComplexColumnVector
operator * (const FloatDiagMatrix& m, const FloatComplexColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return FloatComplexColumnVector (0);

  FloatComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

FloatComplexColumnVector
operator * (const FloatComplexDiagMatrix& m, const FloatColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return FloatComplexColumnVector (0);

  FloatComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

FloatComplexColumnVector
operator * (const FloatComplexDiagMatrix& m, const FloatComplexColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return FloatComplexColumnVector (0);

  FloatComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

// other operations

FloatComplex
FloatComplexColumnVector::min (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  FloatComplex res = elem (0);
  float absres = std::abs (res);

  for (octave_idx_type i = 1; i < len; i++)
    if (std::abs (elem (i)) < absres)
      {
        res = elem (i);
        absres = std::abs (res);
      }

  return res;
}

FloatComplex
FloatComplexColumnVector::max (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  FloatComplex res = elem (0);
  float absres = std::abs (res);

  for (octave_idx_type i = 1; i < len; i++)
    if (std::abs (elem (i)) > absres)
      {
        res = elem (i);
        absres = std::abs (res);
      }

  return res;
}

// i/o

std::ostream&
operator << (std::ostream& os, const FloatComplexColumnVector& a)
{
//  int field_width = os.precision () + 7;
  for (octave_idx_type i = 0; i < a.numel (); i++)
    os << /* setw (field_width) << */ a.elem (i) << "\n";
  return os;
}

std::istream&
operator >> (std::istream& is, FloatComplexColumnVector& a)
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
