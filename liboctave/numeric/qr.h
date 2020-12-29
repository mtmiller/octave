////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2020 The Octave Project Developers
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

#if ! defined (octave_qr_h)
#define octave_qr_h 1

#include "octave-config.h"

template <typename T> class Array;

namespace octave
{
  namespace math
  {
    template <typename T>
    class
    qr
    {
    public:

      typedef typename T::element_type ELT_T;
      typedef typename T::row_vector_type RV_T;
      typedef typename T::column_vector_type CV_T;

      enum type
      {
        std,
        raw,
        economy
      };

      qr (void) : q (), r () { }

      qr (const T& a, type qr_type = qr::std)
        : q (), r ()
      {
        init (a, qr_type);
      }

      OCTAVE_API qr (const T& q, const T& r);

      qr (const qr& a) : q (a.q), r (a.r) { }

      qr& operator = (const qr& a)
      {
        if (this != &a)
          {
            q = a.q;
            r = a.r;
          }

        return *this;
      }

      virtual ~qr (void) = default;

      T Q (void) const { return q; }

      T R (void) const { return r; }

      OCTAVE_API type get_type (void) const;

      OCTAVE_API bool regular (void) const;

      OCTAVE_API void init (const T& a, type qr_type);

      OCTAVE_API void update (const CV_T& u, const CV_T& v);

      OCTAVE_API void update (const T& u, const T& v);

      OCTAVE_API void insert_col (const CV_T& u, octave_idx_type j);

      OCTAVE_API void insert_col (const T& u, const Array<octave_idx_type>& j);

      OCTAVE_API void delete_col (octave_idx_type j);

      OCTAVE_API void delete_col (const Array<octave_idx_type>& j);

      OCTAVE_API void insert_row (const RV_T& u, octave_idx_type j);

      OCTAVE_API void delete_row (octave_idx_type j);

      OCTAVE_API void shift_cols (octave_idx_type i, octave_idx_type j);

    protected:

      T q;
      T r;

      OCTAVE_API void
      form (octave_idx_type n, T& afact, ELT_T *tau, type qr_type);
    };

    extern OCTAVE_API void warn_qrupdate_once (void);
  }
}

#endif
