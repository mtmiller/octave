//                                  -*- C++ -*-
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

#if !defined (octave_ComplexSCHUR_h)
#define octave_ComplexSCHUR_h 1

#if defined (__GNUG__)
#pragma interface
#endif

class ostream;

#include "CMatrix.h"

class ComplexSCHUR
{
friend class ComplexMatrix;

public:

  ComplexSCHUR (void) { }

  ComplexSCHUR (const ComplexMatrix& a, const char *ord)
    {
      init (a,ord);
    }

  ComplexSCHUR (const ComplexMatrix& a, const char *ord, int& info)
    {
      info = init (a,ord);
    }

  ComplexSCHUR (const ComplexSCHUR& a)
    {
      schur_mat = a.schur_mat;
      unitary_mat = a.unitary_mat;
    }

  ComplexSCHUR& operator = (const ComplexSCHUR& a)
    {
      schur_mat = a.schur_mat;
      unitary_mat = a.unitary_mat;

      return *this;
    }

  ComplexMatrix schur_matrix (void) const
    {
      return schur_mat;
    }

  ComplexMatrix unitary_matrix (void) const
    {
      return unitary_mat;
    }

  friend ostream& operator << (ostream& os, const ComplexSCHUR& a);

private:

  int init (const ComplexMatrix& a, const char *ord);

  ComplexMatrix schur_mat;
  ComplexMatrix unitary_mat;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
