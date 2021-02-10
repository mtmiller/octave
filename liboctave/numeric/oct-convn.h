////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009-2021 The Octave Project Developers
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

#if ! defined (octave_oct_convn_h)
#define octave_oct_convn_h 1

#include "octave-config.h"

#include "CColVector.h"
#include "CMatrix.h"
#include "CNDArray.h"
#include "CRowVector.h"
#include "dColVector.h"
#include "dMatrix.h"
#include "dNDArray.h"
#include "dRowVector.h"
#include "fCColVector.h"
#include "fCMatrix.h"
#include "fCNDArray.h"
#include "fCRowVector.h"
#include "fColVector.h"
#include "fMatrix.h"
#include "fNDArray.h"
#include "fRowVector.h"

enum convn_type
{
  convn_full,
  convn_same,
  convn_valid
};

#define CONV_DECLS(TPREF, RPREF)                                        \
  extern OCTAVE_API TPREF ## NDArray                                    \
  convn (const TPREF ## NDArray& a, const RPREF ## NDArray& b,          \
         convn_type ct);                                                \
  extern OCTAVE_API TPREF ## Matrix                                     \
  convn (const TPREF ## Matrix& a, const RPREF ## Matrix& b,            \
         convn_type ct);                                                \
  extern OCTAVE_API TPREF ## Matrix                                     \
  convn (const TPREF ## Matrix& a, const RPREF ## ColumnVector& c,      \
         const RPREF ## RowVector& r, convn_type ct)


CONV_DECLS ( , );
CONV_DECLS (Complex, );
CONV_DECLS (Complex, Complex);
CONV_DECLS (Float, Float);
CONV_DECLS (FloatComplex, Float);
CONV_DECLS (FloatComplex, FloatComplex);

#endif
