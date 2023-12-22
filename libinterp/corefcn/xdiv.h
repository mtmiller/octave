////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2024 The Octave Project Developers
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

#if ! defined (octave_xdiv_h)
#define octave_xdiv_h 1

#include "octave-config.h"

#include "mx-defs.h"
#include "MatrixType.h"

OCTAVE_BEGIN_NAMESPACE(octave)

extern Matrix xdiv (const Matrix& a, const Matrix& b, MatrixType& typ);
extern ComplexMatrix xdiv (const Matrix& a, const ComplexMatrix& b,
                           MatrixType& typ);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const Matrix& b,
                           MatrixType& typ);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const ComplexMatrix& b,
                           MatrixType& typ);

extern Matrix elem_xdiv (double a, const Matrix& b);
extern ComplexMatrix elem_xdiv (double a, const ComplexMatrix& b);
extern ComplexMatrix elem_xdiv (const Complex a, const Matrix& b);
extern ComplexMatrix elem_xdiv (const Complex a, const ComplexMatrix& b);

extern NDArray elem_xdiv (double a, const NDArray& b);
extern ComplexNDArray elem_xdiv (double a, const ComplexNDArray& b);
extern ComplexNDArray elem_xdiv (const Complex a, const NDArray& b);
extern ComplexNDArray elem_xdiv (const Complex a, const ComplexNDArray& b);

extern Matrix xleftdiv (const Matrix& a, const Matrix& b, MatrixType& typ,
                        blas_trans_type transt = blas_no_trans);
extern ComplexMatrix xleftdiv (const Matrix& a, const ComplexMatrix& b,
                               MatrixType& typ,
                               blas_trans_type transt = blas_no_trans);
extern ComplexMatrix xleftdiv (const ComplexMatrix& a, const Matrix& b,
                               MatrixType& typ,
                               blas_trans_type transt = blas_no_trans);
extern ComplexMatrix xleftdiv (const ComplexMatrix& a, const ComplexMatrix& b,
                               MatrixType& typ,
                               blas_trans_type transt = blas_no_trans);

extern FloatMatrix xdiv (const FloatMatrix& a, const FloatMatrix& b,
                         MatrixType& typ);
extern FloatComplexMatrix xdiv (const FloatMatrix& a,
                                const FloatComplexMatrix& b,
                                MatrixType& typ);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a,
                                const FloatMatrix& b,
                                MatrixType& typ);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a,
                                const FloatComplexMatrix& b,
                                MatrixType& typ);

extern FloatMatrix elem_xdiv (float a, const FloatMatrix& b);
extern FloatComplexMatrix elem_xdiv (float a, const FloatComplexMatrix& b);
extern FloatComplexMatrix elem_xdiv (const FloatComplex a,
                                     const FloatMatrix& b);
extern FloatComplexMatrix elem_xdiv (const FloatComplex a,
                                     const FloatComplexMatrix& b);

extern FloatNDArray elem_xdiv (float a, const FloatNDArray& b);
extern FloatComplexNDArray elem_xdiv (float a, const FloatComplexNDArray& b);
extern FloatComplexNDArray elem_xdiv (const FloatComplex a,
                                      const FloatNDArray& b);
extern FloatComplexNDArray elem_xdiv (const FloatComplex a,
                                      const FloatComplexNDArray& b);

extern FloatMatrix xleftdiv (const FloatMatrix& a, const FloatMatrix& b,
                             MatrixType& typ,
                             blas_trans_type transt = blas_no_trans);
extern FloatComplexMatrix xleftdiv (const FloatMatrix& a,
                                    const FloatComplexMatrix& b,
                                    MatrixType& typ,
                                    blas_trans_type transt = blas_no_trans);
extern FloatComplexMatrix xleftdiv (const FloatComplexMatrix& a,
                                    const FloatMatrix& b,
                                    MatrixType& typ,
                                    blas_trans_type transt = blas_no_trans);
extern FloatComplexMatrix xleftdiv (const FloatComplexMatrix& a,
                                    const FloatComplexMatrix& b,
                                    MatrixType& typ,
                                    blas_trans_type transt = blas_no_trans);

extern Matrix xdiv (const Matrix& a, const DiagMatrix& b);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const DiagMatrix& b);
extern ComplexMatrix xdiv (const ComplexMatrix& a, const ComplexDiagMatrix& b);

extern DiagMatrix xdiv (const DiagMatrix& a, const DiagMatrix& b);
extern ComplexDiagMatrix xdiv (const ComplexDiagMatrix& a, const DiagMatrix& b);
extern ComplexDiagMatrix xdiv (const ComplexDiagMatrix& a,
                               const ComplexDiagMatrix& b);

extern FloatMatrix xdiv (const FloatMatrix& a, const FloatDiagMatrix& b);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a,
                                const FloatDiagMatrix& b);
extern FloatComplexMatrix xdiv (const FloatMatrix& a,
                                const FloatComplexDiagMatrix& b);
extern FloatComplexMatrix xdiv (const FloatComplexMatrix& a,
                                const FloatComplexDiagMatrix& b);

extern FloatDiagMatrix xdiv (const FloatDiagMatrix& a,
                             const FloatDiagMatrix& b);
extern FloatComplexDiagMatrix xdiv (const FloatComplexDiagMatrix& a,
                                    const FloatDiagMatrix& b);
extern FloatComplexDiagMatrix xdiv (const FloatComplexDiagMatrix& a,
                                    const FloatComplexDiagMatrix& b);

extern Matrix xleftdiv (const DiagMatrix& a, const Matrix& b);
extern ComplexMatrix xleftdiv (const DiagMatrix& a, const ComplexMatrix& b);
extern ComplexMatrix xleftdiv (const ComplexDiagMatrix& a,
                               const ComplexMatrix& b);

extern DiagMatrix xleftdiv (const DiagMatrix& a, const DiagMatrix& b);
extern ComplexDiagMatrix xleftdiv (const DiagMatrix& a,
                                   const ComplexDiagMatrix& b);
extern ComplexDiagMatrix xleftdiv (const ComplexDiagMatrix& a,
                                   const ComplexDiagMatrix& b);

extern FloatMatrix xleftdiv (const FloatDiagMatrix& a,
                             const FloatMatrix& b);
extern FloatComplexMatrix xleftdiv (const FloatDiagMatrix& a,
                                    const FloatComplexMatrix& b);
extern FloatComplexMatrix xleftdiv (const FloatComplexDiagMatrix& a,
                                    const FloatComplexMatrix& b);

extern FloatDiagMatrix xleftdiv (const FloatDiagMatrix& a,
                                 const FloatDiagMatrix& b);
extern FloatComplexDiagMatrix xleftdiv (const FloatDiagMatrix& a,
                                        const FloatComplexDiagMatrix& b);
extern FloatComplexDiagMatrix xleftdiv (const FloatComplexDiagMatrix& a,
                                        const FloatComplexDiagMatrix& b);

OCTAVE_END_NAMESPACE(octave)

#endif
