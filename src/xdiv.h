/*

Copyright (C) 1993, 1994, 1995, 1996, 1997, 2003, 2005, 2006, 2007
              John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_xdiv_h)
#define octave_xdiv_h 1

#include "oct-cmplx.h"
#include "MatrixType.h"

class Matrix;
class ComplexMatrix;

class NDArray;
class ComplexNDArray;

extern Matrix xdiv (const Matrix& a, const Matrix& b, MatrixType &typ);
extern ComplexMatrix xdiv (const Matrix& a, const ComplexMatrix& b,
			   MatrixType &typ);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const Matrix& b,
			   MatrixType &typ);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const ComplexMatrix& b,
			   MatrixType &typ);

extern Matrix x_el_div (double a, const Matrix& b);
extern ComplexMatrix x_el_div (double a, const ComplexMatrix& b);
extern ComplexMatrix x_el_div (const Complex a, const Matrix& b);
extern ComplexMatrix x_el_div (const Complex a, const ComplexMatrix& b);

extern NDArray x_el_div (double a, const NDArray& b);
extern ComplexNDArray x_el_div (double a, const ComplexNDArray& b);
extern ComplexNDArray x_el_div (const Complex a, const NDArray& b);
extern ComplexNDArray x_el_div (const Complex a, const ComplexNDArray& b);

extern Matrix xleftdiv (const Matrix& a, const Matrix& b, MatrixType &typ);
extern ComplexMatrix xleftdiv (const Matrix& a, const ComplexMatrix& b,
			       MatrixType &typ);
extern ComplexMatrix xleftdiv (const ComplexMatrix& a, const Matrix& b,
			       MatrixType &typ);
extern ComplexMatrix xleftdiv (const ComplexMatrix& a, const ComplexMatrix& b,
			       MatrixType &typ);

class FloatMatrix;
class FloatComplexMatrix;

class FloatNDArray;
class FloatComplexNDArray;

extern FloatMatrix xdiv (const FloatMatrix& a, const FloatMatrix& b, MatrixType &typ);
extern FloatComplexMatrix xdiv (const FloatMatrix& a, const FloatComplexMatrix& b,
			   MatrixType &typ);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a, const FloatMatrix& b,
			   MatrixType &typ);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a, const FloatComplexMatrix& b,
			   MatrixType &typ);

extern FloatMatrix x_el_div (float a, const FloatMatrix& b);
extern FloatComplexMatrix x_el_div (float a, const FloatComplexMatrix& b);
extern FloatComplexMatrix x_el_div (const FloatComplex a, const FloatMatrix& b);
extern FloatComplexMatrix x_el_div (const FloatComplex a, const FloatComplexMatrix& b);

extern FloatNDArray x_el_div (float a, const FloatNDArray& b);
extern FloatComplexNDArray x_el_div (float a, const FloatComplexNDArray& b);
extern FloatComplexNDArray x_el_div (const FloatComplex a, const FloatNDArray& b);
extern FloatComplexNDArray x_el_div (const FloatComplex a, const FloatComplexNDArray& b);

extern FloatMatrix xleftdiv (const FloatMatrix& a, const FloatMatrix& b, MatrixType &typ);
extern FloatComplexMatrix xleftdiv (const FloatMatrix& a, const FloatComplexMatrix& b,
			       MatrixType &typ);
extern FloatComplexMatrix xleftdiv (const FloatComplexMatrix& a, const FloatMatrix& b,
			       MatrixType &typ);
extern FloatComplexMatrix xleftdiv (const FloatComplexMatrix& a, const FloatComplexMatrix& b,
			       MatrixType &typ);


#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
