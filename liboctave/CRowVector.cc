// RowVector manipulations.                              -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>

#include <Complex.h>

#include "f77-uscore.h"
#include "lo-error.h"
#include "mx-base.h"
#include "mx-inlines.cc"

// Fortran functions we call.

extern "C"
{
  int F77_FCN (zgemv, ZGEMV) (const char*, const int&, const int&,
			      const Complex&, const Complex*,
			      const int&, const Complex*, const int&,
			      const Complex&, Complex*, const int&,
			      long);
}

// Complex Row Vector class

ComplexRowVector::ComplexRowVector (const RowVector& a)
  : MArray<Complex> (a.length ())
{
  for (int i = 0; i < length (); i++)
    elem (i) = a.elem (i);
}

int
ComplexRowVector::operator == (const ComplexRowVector& a) const
{
  int len = length ();
  if (len != a.length ())
    return 0;
  return equal (data (), a.data (), len);
}

int
ComplexRowVector::operator != (const ComplexRowVector& a) const
{
  return !(*this == a);
}

// destructive insert/delete/reorder operations

ComplexRowVector&
ComplexRowVector::insert (const RowVector& a, int c)
{
  int a_len = a.length ();
  if (c < 0 || c + a_len - 1 > length ())
    {
      (*current_liboctave_error_handler) ("range error for insert");
      return *this;
    }

  for (int i = 0; i < a_len; i++)
    elem (c+i) = a.elem (i);

  return *this;
}

ComplexRowVector&
ComplexRowVector::insert (const ComplexRowVector& a, int c)
{
  int a_len = a.length ();
  if (c < 0 || c + a_len - 1 > length ())
    {
      (*current_liboctave_error_handler) ("range error for insert");
      return *this;
    }

  for (int i = 0; i < a_len; i++)
    elem (c+i) = a.elem (i);

  return *this;
}

ComplexRowVector&
ComplexRowVector::fill (double val)
{
  int len = length ();
  if (len > 0)
    for (int i = 0; i < len; i++)
      elem (i) = val;
  return *this;
}

ComplexRowVector&
ComplexRowVector::fill (const Complex& val)
{
  int len = length ();
  if (len > 0)
    for (int i = 0; i < len; i++)
      elem (i) = val;
  return *this;
}

ComplexRowVector&
ComplexRowVector::fill (double val, int c1, int c2)
{
  int len = length ();
  if (c1 < 0 || c2 < 0 || c1 >= len || c2 >= len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  if (c1 > c2) { int tmp = c1; c1 = c2; c2 = tmp; }

  for (int i = c1; i <= c2; i++)
    elem (i) = val;

  return *this;
}

ComplexRowVector&
ComplexRowVector::fill (const Complex& val, int c1, int c2)
{
  int len = length ();
  if (c1 < 0 || c2 < 0 || c1 >= len || c2 >= len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  if (c1 > c2) { int tmp = c1; c1 = c2; c2 = tmp; }

  for (int i = c1; i <= c2; i++)
    elem (i) = val;

  return *this;
}

ComplexRowVector
ComplexRowVector::append (const RowVector& a) const
{
  int len = length ();
  int nc_insert = len;
  ComplexRowVector retval (len + a.length ());
  retval.insert (*this, 0);
  retval.insert (a, nc_insert);
  return retval;
}

ComplexRowVector
ComplexRowVector::append (const ComplexRowVector& a) const
{
  int len = length ();
  int nc_insert = len;
  ComplexRowVector retval (len + a.length ());
  retval.insert (*this, 0);
  retval.insert (a, nc_insert);
  return retval;
}

ComplexColumnVector
ComplexRowVector::hermitian (void) const
{
  int len = length ();
  return ComplexColumnVector (conj_dup (data (), len), len);
}

ComplexColumnVector
ComplexRowVector::transpose (void) const
{
  int len = length ();
  return ComplexColumnVector (dup (data (), len), len);
}

ComplexRowVector
conj (const ComplexRowVector& a)
{
  int a_len = a.length ();
  ComplexRowVector retval;
  if (a_len > 0)
    retval = ComplexRowVector (conj_dup (a.data (), a_len), a_len);
  return retval;
}

// resize is the destructive equivalent for this one

ComplexRowVector
ComplexRowVector::extract (int c1, int c2) const
{
  if (c1 > c2) { int tmp = c1; c1 = c2; c2 = tmp; }

  int new_c = c2 - c1 + 1;

  ComplexRowVector result (new_c);

  for (int i = 0; i < new_c; i++)
    result.elem (i) = elem (c1+i);

  return result;
}

// row vector by row vector -> row vector operations

ComplexRowVector&
ComplexRowVector::operator += (const RowVector& a)
{
  int len = length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector += operation attempted");
      return *this;
    }

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  add2 (d, a.data (), len);
  return *this;
}

ComplexRowVector&
ComplexRowVector::operator -= (const RowVector& a)
{
  int len = length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector -= operation attempted");
      return *this;
    }

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  subtract2 (d, a.data (), len);
  return *this;
}

ComplexRowVector&
ComplexRowVector::operator += (const ComplexRowVector& a)
{
  int len = length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector += operation attempted");
      return *this;
    }

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  add2 (d, a.data (), len);
  return *this;
}

ComplexRowVector&
ComplexRowVector::operator -= (const ComplexRowVector& a)
{
  int len = length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector -= operation attempted");
      return *this;
    }

  if (len == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  subtract2 (d, a.data (), len);
  return *this;
}

// row vector by scalar -> row vector operations

ComplexRowVector
operator + (const ComplexRowVector& v, double s)
{
  int len = v.length ();
  return ComplexRowVector (add (v.data (), len, s), len);
}

ComplexRowVector
operator - (const ComplexRowVector& v, double s)
{
  int len = v.length ();
  return ComplexRowVector (subtract (v.data (), len, s), len);
}

ComplexRowVector
operator * (const ComplexRowVector& v, double s)
{
  int len = v.length ();
  return ComplexRowVector (multiply (v.data (), len, s), len);
}

ComplexRowVector
operator / (const ComplexRowVector& v, double s)
{
  int len = v.length ();
  return ComplexRowVector (divide (v.data (), len, s), len);
}

ComplexRowVector
operator + (const RowVector& v, const Complex& s)
{
  int len = v.length ();
  return ComplexRowVector (add (v.data (), len, s), len);
}

ComplexRowVector
operator - (const RowVector& v, const Complex& s)
{
  int len = v.length ();
  return ComplexRowVector (subtract (v.data (), len, s), len);
}

ComplexRowVector
operator * (const RowVector& v, const Complex& s)
{
  int len = v.length ();
  return ComplexRowVector (multiply (v.data (), len, s), len);
}

ComplexRowVector
operator / (const RowVector& v, const Complex& s)
{
  int len = v.length ();
  return ComplexRowVector (divide (v.data (), len, s), len);
}

// scalar by row vector -> row vector operations

ComplexRowVector
operator + (double s, const ComplexRowVector& a)
{
  int a_len = a.length ();
  return ComplexRowVector (add (a.data (), a_len, s), a_len);
}

ComplexRowVector
operator - (double s, const ComplexRowVector& a)
{
  int a_len = a.length ();
  return ComplexRowVector (subtract (s, a.data (), a_len), a_len);
}

ComplexRowVector
operator * (double s, const ComplexRowVector& a)
{
  int a_len = a.length ();
  return ComplexRowVector (multiply (a.data (), a_len, s), a_len);
}

ComplexRowVector
operator / (double s, const ComplexRowVector& a)
{
  int a_len = a.length ();
  return ComplexRowVector (divide (s, a.data (), a_len), a_len);
}

ComplexRowVector
operator + (const Complex& s, const RowVector& a)
{
  return ComplexRowVector ();
}

ComplexRowVector
operator - (const Complex& s, const RowVector& a)
{
  return ComplexRowVector ();
}

ComplexRowVector
operator * (const Complex& s, const RowVector& a)
{
  return ComplexRowVector ();
}

ComplexRowVector
operator / (const Complex& s, const RowVector& a)
{
  return ComplexRowVector ();
}

// row vector by matrix -> row vector

ComplexRowVector
operator * (const ComplexRowVector& v, const ComplexMatrix& a)
{
  int len = v.length ();
  if (a.rows () != len)
    {
      (*current_liboctave_error_handler)
	("nonconformant vector multiplication attempted");
      return ComplexRowVector ();
    }

  if (len == 0 || a.cols () == 0)
    return ComplexRowVector (0);

  // Transpose A to form A'*x == (x'*A)'

  int a_nr = a.rows ();
  int a_nc = a.cols ();

  int ld = a_nr;

  Complex *y = new Complex [len];

  F77_FCN (zgemv, ZGEMV) ("T", a_nc, a_nr, 1.0, a.data (), ld,
			  v.data (), 1, 0.0, y, 1, 1L); 

  return ComplexRowVector (y, len);
}

ComplexRowVector
operator * (const RowVector& v, const ComplexMatrix& a)
{
  ComplexRowVector tmp (v);
  return tmp * a;
}

// row vector by row vector -> row vector operations

ComplexRowVector
operator + (const ComplexRowVector& v, const RowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector addition attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (add (v.data (), a.data (), len), len);
}

ComplexRowVector
operator - (const ComplexRowVector& v, const RowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector subtraction attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (subtract (v.data (), a.data (), len), len);
}

ComplexRowVector
operator + (const RowVector& v, const ComplexRowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector addition attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (add (v.data (), a.data (), len), len);
}

ComplexRowVector
operator - (const RowVector& v, const ComplexRowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector subtraction attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (subtract (v.data (), a.data (), len), len);
}

ComplexRowVector
product (const ComplexRowVector& v, const RowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector product attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (multiply (v.data (), a.data (), len), len);
}

ComplexRowVector
quotient (const ComplexRowVector& v, const RowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector quotient attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (divide (v.data (), a.data (), len), len);
}

ComplexRowVector
product (const RowVector& v, const ComplexRowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector product attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (multiply (v.data (), a.data (), len), len);
}

ComplexRowVector
quotient (const RowVector& v, const ComplexRowVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector quotient attempted");
      return ComplexRowVector ();
    }

  if (len == 0)
    return ComplexRowVector (0);

  return ComplexRowVector (divide (v.data (), a.data (), len), len);
}

// other operations

ComplexRowVector
map (c_c_Mapper f, const ComplexRowVector& a)
{
  ComplexRowVector b (a);
  b.map (f);
  return b;
}

void
ComplexRowVector::map (c_c_Mapper f)
{
  for (int i = 0; i < length (); i++)
    elem (i) = f (elem (i));
}

Complex
ComplexRowVector::min (void) const
{
  int len = length ();
  if (len == 0)
    return Complex (0.0);

  Complex res = elem (0);
  double absres = abs (res);

  for (int i = 1; i < len; i++)
    if (abs (elem (i)) < absres)
      {
	res = elem (i);
	absres = abs (res);
      }

  return res;
}

Complex
ComplexRowVector::max (void) const
{
  int len = length ();
  if (len == 0)
    return Complex (0.0);

  Complex res = elem (0);
  double absres = abs (res);

  for (int i = 1; i < len; i++)
    if (abs (elem (i)) > absres)
      {
	res = elem (i);
	absres = abs (res);
      }

  return res;
}

// i/o

ostream&
operator << (ostream& os, const ComplexRowVector& a)
{
//  int field_width = os.precision () + 7;
  for (int i = 0; i < a.length (); i++)
    os << " " /* setw (field_width) */ << a.elem (i);
  return os;
}

istream&
operator >> (istream& is, ComplexRowVector& a)
{
  int len = a.length();

  if (len < 1)
    is.clear (ios::badbit);
  else
    {
      Complex tmp;
      for (int i = 0; i < len; i++)
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

// row vector by column vector -> scalar

// row vector by column vector -> scalar

Complex
operator * (const ComplexRowVector& v, const ColumnVector& a)
{
  ComplexColumnVector tmp (a);
  return v * tmp;
}

Complex
operator * (const ComplexRowVector& v, const ComplexColumnVector& a)
{
  int len = v.length ();
  if (len != a.length ())
    {
      (*current_liboctave_error_handler)
	("nonconformant vector multiplication attempted");
      return 0.0;
    }

  Complex retval (0.0, 0.0);

  for (int i = 0; i < len; i++)
    retval += v.elem (i) * a.elem (i);

  return retval;
}

// other operations

ComplexRowVector
linspace (const Complex& x1, const Complex& x2, int n)
{
  ComplexRowVector retval;

  if (n > 0)
    {
      retval.resize (n);
      Complex delta = (x2 - x1) / (n - 1);
      retval.elem (0) = x1;
      for (int i = 1; i < n-1; i++)
	retval.elem (i) = x1 + i * delta;
      retval.elem (n-1) = x2;
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
