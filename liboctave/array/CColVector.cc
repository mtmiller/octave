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

// Complex Column Vector class

ComplexColumnVector::ComplexColumnVector (const ColumnVector& a)
  : MArray<Complex> (a)
{ }

bool
ComplexColumnVector::operator == (const ComplexColumnVector& a) const
{
  octave_idx_type len = numel ();
  if (len != a.numel ())
    return 0;
  return mx_inline_equal (len, data (), a.data ());
}

bool
ComplexColumnVector::operator != (const ComplexColumnVector& a) const
{
  return !(*this == a);
}

// destructive insert/delete/reorder operations

ComplexColumnVector&
ComplexColumnVector::insert (const ColumnVector& a, octave_idx_type r)
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

ComplexColumnVector&
ComplexColumnVector::insert (const ComplexColumnVector& a, octave_idx_type r)
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

ComplexColumnVector&
ComplexColumnVector::fill (double val)
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

ComplexColumnVector&
ComplexColumnVector::fill (const Complex& val)
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

ComplexColumnVector&
ComplexColumnVector::fill (double val, octave_idx_type r1, octave_idx_type r2)
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

ComplexColumnVector&
ComplexColumnVector::fill (const Complex& val,
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

ComplexColumnVector
ComplexColumnVector::stack (const ColumnVector& a) const
{
  octave_idx_type len = numel ();
  octave_idx_type nr_insert = len;
  ComplexColumnVector retval (len + a.numel ());
  retval.insert (*this, 0);
  retval.insert (a, nr_insert);
  return retval;
}

ComplexColumnVector
ComplexColumnVector::stack (const ComplexColumnVector& a) const
{
  octave_idx_type len = numel ();
  octave_idx_type nr_insert = len;
  ComplexColumnVector retval (len + a.numel ());
  retval.insert (*this, 0);
  retval.insert (a, nr_insert);
  return retval;
}

ComplexRowVector
ComplexColumnVector::hermitian (void) const
{
  return MArray<Complex>::hermitian (std::conj);
}

ComplexRowVector
ComplexColumnVector::transpose (void) const
{
  return MArray<Complex>::transpose ();
}

ColumnVector
ComplexColumnVector::abs (void) const
{
  return do_mx_unary_map<double, Complex, std::abs> (*this);
}

ComplexColumnVector
conj (const ComplexColumnVector& a)
{
  return do_mx_unary_map<Complex, Complex, std::conj<double>> (a);
}

// resize is the destructive equivalent for this one

ComplexColumnVector
ComplexColumnVector::extract (octave_idx_type r1, octave_idx_type r2) const
{
  if (r1 > r2) { std::swap (r1, r2); }

  octave_idx_type new_r = r2 - r1 + 1;

  ComplexColumnVector result (new_r);

  for (octave_idx_type i = 0; i < new_r; i++)
    result.elem (i) = elem (r1+i);

  return result;
}

ComplexColumnVector
ComplexColumnVector::extract_n (octave_idx_type r1, octave_idx_type n) const
{
  ComplexColumnVector result (n);

  for (octave_idx_type i = 0; i < n; i++)
    result.elem (i) = elem (r1+i);

  return result;
}

// column vector by column vector -> column vector operations

ComplexColumnVector&
ComplexColumnVector::operator += (const ColumnVector& a)
{
  octave_idx_type len = numel ();

  octave_idx_type a_len = a.numel ();

  if (len != a_len)
    octave::err_nonconformant ("operator +=", len, a_len);

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  mx_inline_add2 (len, d, a.data ());
  return *this;
}

ComplexColumnVector&
ComplexColumnVector::operator -= (const ColumnVector& a)
{
  octave_idx_type len = numel ();

  octave_idx_type a_len = a.numel ();

  if (len != a_len)
    octave::err_nonconformant ("operator -=", len, a_len);

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  mx_inline_sub2 (len, d, a.data ());
  return *this;
}

// matrix by column vector -> column vector operations

ComplexColumnVector
operator * (const ComplexMatrix& m, const ColumnVector& a)
{
  ComplexColumnVector tmp (a);
  return m * tmp;
}

ComplexColumnVector
operator * (const ComplexMatrix& m, const ComplexColumnVector& a)
{
  ComplexColumnVector retval;

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
          Complex *y = retval.fortran_vec ();

          F77_XFCN (zgemv, ZGEMV, (F77_CONST_CHAR_ARG2 ("N", 1),
                                   nr, nc, 1.0,
                                   F77_CONST_DBLE_CMPLX_ARG (m.data ()), nr,
                                   F77_CONST_DBLE_CMPLX_ARG (a.data ()), 1, 0.0,
                                   F77_DBLE_CMPLX_ARG (y), 1
                                   F77_CHAR_ARG_LEN (1)));
        }
    }

  return retval;
}

// matrix by column vector -> column vector operations

ComplexColumnVector
operator * (const Matrix& m, const ComplexColumnVector& a)
{
  ComplexMatrix tmp (m);
  return tmp * a;
}

// diagonal matrix by column vector -> column vector operations

ComplexColumnVector
operator * (const DiagMatrix& m, const ComplexColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return ComplexColumnVector (0);

  ComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

ComplexColumnVector
operator * (const ComplexDiagMatrix& m, const ColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return ComplexColumnVector (0);

  ComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

ComplexColumnVector
operator * (const ComplexDiagMatrix& m, const ComplexColumnVector& a)
{
  F77_INT nr = octave::to_f77_int (m.rows ());
  F77_INT nc = octave::to_f77_int (m.cols ());

  F77_INT a_len = octave::to_f77_int (a.numel ());

  if (nc != a_len)
    octave::err_nonconformant ("operator *", nr, nc, a_len, 1);

  if (nc == 0 || nr == 0)
    return ComplexColumnVector (0);

  ComplexColumnVector result (nr);

  for (octave_idx_type i = 0; i < a_len; i++)
    result.elem (i) = a.elem (i) * m.elem (i, i);

  for (octave_idx_type i = a_len; i < nr; i++)
    result.elem (i) = 0.0;

  return result;
}

// other operations

Complex
ComplexColumnVector::min (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  Complex res = elem (0);
  double absres = std::abs (res);

  for (octave_idx_type i = 1; i < len; i++)
    if (std::abs (elem (i)) < absres)
      {
        res = elem (i);
        absres = std::abs (res);
      }

  return res;
}

Complex
ComplexColumnVector::max (void) const
{
  octave_idx_type len = numel ();
  if (len == 0)
    return 0.0;

  Complex res = elem (0);
  double absres = std::abs (res);

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
operator << (std::ostream& os, const ComplexColumnVector& a)
{
//  int field_width = os.precision () + 7;
  for (octave_idx_type i = 0; i < a.numel (); i++)
    os << /* setw (field_width) << */ a.elem (i) << "\n";
  return os;
}

std::istream&
operator >> (std::istream& is, ComplexColumnVector& a)
{
  octave_idx_type len = a.numel ();

  if (len > 0)
    {
      double tmp;
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
