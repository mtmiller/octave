////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2021 The Octave Project Developers
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

#if ! defined (octave_chol_h)
#define octave_chol_h 1

#include "octave-config.h"

namespace octave
{
  namespace math
  {
    template <typename T>
    class
    chol
    {
    public:

      typedef typename T::column_vector_type VT;
      typedef typename T::real_elt_type COND_T;

      chol (void) : chol_mat (), xrcond (0), is_upper (true) { }

      chol (const T& a, bool upper = true, bool calc_cond = false)
        : chol_mat (), xrcond (0)
      {
        init (a, upper, calc_cond);
      }

      chol (const T& a, octave_idx_type& info, bool upper = true,
            bool calc_cond = false)
        : chol_mat (), xrcond (0)
      {
        info = init (a, upper, calc_cond);
      }

      chol (const chol& a)
        : chol_mat (a.chol_mat), xrcond (a.xrcond), is_upper (a.is_upper) { }

      chol& operator = (const chol& a)
      {
        if (this != &a)
          {
            chol_mat = a.chol_mat;
            xrcond = a.xrcond;
            is_upper = a.is_upper;
          }

        return *this;
      }

      T chol_matrix (void) const { return chol_mat; }

      COND_T rcond (void) const { return xrcond; }

      // Compute the inverse of a matrix using the Cholesky factorization.
      OCTAVE_API T inverse (void) const;

      OCTAVE_API void set (const T& R);

      OCTAVE_API void update (const VT& u);

      OCTAVE_API octave_idx_type downdate (const VT& u);

      OCTAVE_API octave_idx_type insert_sym (const VT& u, octave_idx_type j);

      OCTAVE_API void delete_sym (octave_idx_type j);

      OCTAVE_API void shift_sym (octave_idx_type i, octave_idx_type j);

    private:

      T chol_mat;

      COND_T xrcond;

      bool is_upper;

      OCTAVE_API octave_idx_type init (const T& a, bool upper, bool calc_cond);
    };

    template <typename T>
    OCTAVE_API T
    chol2inv (const T& r);
  }
}

#endif
