////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2002-2021 The Octave Project Developers
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

#if ! defined (octave_ODESFunc_h)
#define octave_ODESFunc_h 1

#include "octave-config.h"

#include "dMatrix.h"

class ColumnVector;

class
ODESFunc
{
public:

  struct DAEJac
  {
    Matrix *dfdxdot;
    Matrix *dfdx;
  };

  typedef ColumnVector (*ODES_fsub) (const ColumnVector& x, double,
                                     const ColumnVector& theta);

  typedef ColumnVector (*ODES_bsub) (const ColumnVector& x, double,
                                     const ColumnVector& theta, int column);

  typedef Matrix (*ODES_jsub) (const ColumnVector& x, double,
                               const ColumnVector& theta);

  ODESFunc (void)
    : fsub (nullptr), bsub (nullptr), jsub (nullptr) { }

  ODESFunc (ODES_fsub f)
    : fsub (f), bsub (nullptr), jsub (nullptr) { }

  ODESFunc (ODES_fsub f, ODES_bsub b)
    : fsub (f), bsub (b), jsub (nullptr) { }

  ODESFunc (ODES_fsub f, ODES_bsub b, ODES_jsub j)
    : fsub (f), bsub (b), jsub (j) { }

  ODESFunc (const ODESFunc& a)
    : fsub (a.fsub), bsub (a.bsub), jsub (a.jsub) { }

  ODESFunc& operator = (const ODESFunc& a)
  {
    if (this != &a)
      {
        fsub = a.fsub;
        bsub = a.bsub;
        jsub = a.jsub;
      }
    return *this;
  }

  virtual ~ODESFunc (void) = default;

  ODES_fsub fsub_function (void) const { return fsub; }

  ODESFunc& set_fsub_function (ODES_fsub f)
  {
    fsub = f;
    return *this;
  }

  ODES_bsub bsub_function (void) const { return bsub; }

  ODESFunc& set_bsub_function (ODES_bsub b)
  {
    bsub = b;
    return *this;
  }

  ODES_jsub jsub_function (void) const { return jsub; }

  ODESFunc& set_jsub_function (ODES_jsub j)
  {
    jsub = j;
    return *this;
  }

protected:

  ODES_fsub fsub;
  ODES_bsub bsub;
  ODES_jsub jsub;
};

#endif
