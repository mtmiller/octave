// LP.h                                                -*- C++ -*-
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

#if !defined (octave_LP_h)
#define octave_LP_h 1

#if defined (__GNUG__)
#pragma interface
#endif

#include "dColVector.h"
#include "Bounds.h"
#include "LinConst.h"
#include "base-min.h"

class LP : public base_minimizer
{
 public:

  LP (void) : base_minimizer () { }

  LP (const ColumnVector& c_arg)
    : base_minimizer (), c (c_arg) { }

  LP (const ColumnVector& c_arg, const Bounds& b)
    : base_minimizer (), c (c_arg), bnds (b) { }

  LP (const ColumnVector& c_arg, const Bounds& b, const LinConst& l)
    : base_minimizer (), c (c_arg), bnds (b), lc (l) { }

  LP (const ColumnVector& c_arg, const LinConst& l)
    : base_minimizer (), c (c_arg), lc (l) { }

 protected:

  ColumnVector c;
  Bounds bnds;
  LinConst lc;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
