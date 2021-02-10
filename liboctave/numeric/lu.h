////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2021 The Octave Project Developers
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

#if ! defined (octave_lu_h)
#define octave_lu_h 1

#include "octave-config.h"

#include "Array.h"

class ColumnVector;
class PermMatrix;

namespace octave
{
  namespace math
  {
    template <typename T>
    class
    lu
    {
    public:

      typedef typename T::column_vector_type VT;
      typedef typename T::element_type ELT_T;

      lu (void)
        : a_fact (), l_fact (), ipvt () { }

      lu (const T& a);

      lu (const lu& a)
        : a_fact (a.a_fact), l_fact (a.l_fact), ipvt (a.ipvt) { }

      lu (const T& l, const T& u, const PermMatrix& p);

      lu& operator = (const lu& a)
      {
        if (this != &a)
          {
            a_fact = a.a_fact;
            l_fact = a.l_fact;
            ipvt = a.ipvt;
          }

        return *this;
      }

      virtual ~lu (void) = default;

      bool packed (void) const;

      void unpack (void);

      T L (void) const;

      T U (void) const;

      T Y (void) const;

      PermMatrix P (void) const;

      ColumnVector P_vec (void) const;

      bool regular (void) const;

      void update (const VT& u, const VT& v);

      void update (const T& u, const T& v);

      void update_piv (const VT& u, const VT& v);

      void update_piv (const T& u, const T& v);

    protected:

      // The result of getp is passed to other Octave Matrix functions,
      // so we use octave_idx_type.
      Array<octave_idx_type> getp (void) const;

      T a_fact;
      T l_fact;

      // This is internal storage that is passed to Fortran,
      // so we need a Fortran INTEGER.
      Array<octave_f77_int_type> ipvt;
    };
  }
}

#endif
