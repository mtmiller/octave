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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "CMatrix.h"
#include "aepbalance.h"
#include "dColVector.h"
#include "dMatrix.h"
#include "fCMatrix.h"
#include "fColVector.h"
#include "fMatrix.h"
#include "lo-error.h"
#include "lo-lapack-proto.h"

namespace octave
{
  static inline char
  get_job (bool noperm, bool noscal)
  {
    return noperm ? (noscal ? 'N' : 'S') : (noscal ? 'P' : 'B');
  }

  namespace math
  {
    template <>
    OCTAVE_API
    aepbalance<Matrix>::aepbalance (const Matrix& a, bool noperm, bool noscal)
      : balanced_mat (a), scale (), ilo (), ihi (),
        job (get_job (noperm, noscal))
    {
      F77_INT n = to_f77_int (a.cols ());

      if (a.rows () != n)
        (*current_liboctave_error_handler)
          ("aepbalance: requires square matrix");

      scale = ColumnVector (n);

      F77_INT info, t_ilo, t_ihi;

      F77_XFCN (dgebal, DGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1), n,
                                 balanced_mat.fortran_vec (), n,
                                 t_ilo, t_ihi, scale.fortran_vec (), info
                                 F77_CHAR_ARG_LEN (1)));

      ilo = t_ilo;
      ihi = t_ihi;
    }

    template <>
    OCTAVE_API Matrix
    aepbalance<Matrix>::balancing_matrix (void) const
    {
      F77_INT n = to_f77_int (balanced_mat.rows ());

      Matrix balancing_mat (n, n, 0.0);
      for (F77_INT i = 0; i < n; i++)
        balancing_mat.elem (i ,i) = 1.0;

      F77_INT info;
      F77_INT t_ilo = to_f77_int (ilo);
      F77_INT t_ihi = to_f77_int (ihi);

      char side = 'R';

      F77_XFCN (dgebak, DGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                                 F77_CONST_CHAR_ARG2 (&side, 1),
                                 n, t_ilo, t_ihi, scale.data (), n,
                                 balancing_mat.fortran_vec (), n, info
                                 F77_CHAR_ARG_LEN (1)
                                 F77_CHAR_ARG_LEN (1)));

      return balancing_mat;
    }

    template <>
    OCTAVE_API
    aepbalance<FloatMatrix>::aepbalance (const FloatMatrix& a, bool noperm,
                                         bool noscal)
      : balanced_mat (a), scale (), ilo (), ihi (),
        job (get_job (noperm, noscal))
    {
      F77_INT n = to_f77_int (a.cols ());

      if (a.rows () != n)
        (*current_liboctave_error_handler)
          ("aepbalance: requires square matrix");

      scale = FloatColumnVector (n);

      F77_INT info, t_ilo, t_ihi;

      F77_XFCN (sgebal, SGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1), n,
                                 balanced_mat.fortran_vec (), n, t_ilo,
                                 t_ihi, scale.fortran_vec (), info
                                 F77_CHAR_ARG_LEN (1)));

      ilo = t_ilo;
      ihi = t_ihi;
    }

    template <>
    OCTAVE_API FloatMatrix
    aepbalance<FloatMatrix>::balancing_matrix (void) const
    {
      F77_INT n = to_f77_int (balanced_mat.rows ());

      FloatMatrix balancing_mat (n, n, 0.0);
      for (F77_INT i = 0; i < n; i++)
        balancing_mat.elem (i,i) = 1.0;

      F77_INT info;
      F77_INT t_ilo = to_f77_int (ilo);
      F77_INT t_ihi = to_f77_int (ihi);

      char side = 'R';

      F77_XFCN (sgebak, SGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                                 F77_CONST_CHAR_ARG2 (&side, 1),
                                 n, t_ilo, t_ihi, scale.data (), n,
                                 balancing_mat.fortran_vec (), n, info
                                 F77_CHAR_ARG_LEN (1)
                                 F77_CHAR_ARG_LEN (1)));

      return balancing_mat;
    }

    template <>
    OCTAVE_API
    aepbalance<ComplexMatrix>::aepbalance (const ComplexMatrix& a, bool noperm,
                                           bool noscal)
      : balanced_mat (a), scale (), ilo (), ihi (),
        job (get_job (noperm, noscal))
    {
      F77_INT n = to_f77_int (a.cols ());

      if (a.rows () != n)
        (*current_liboctave_error_handler)
          ("aepbalance: requires square matrix");

      scale = ColumnVector (n);

      F77_INT info, t_ilo, t_ihi;

      F77_XFCN (zgebal, ZGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1), n,
                                 F77_DBLE_CMPLX_ARG (balanced_mat.fortran_vec ()),
                                 n, t_ilo, t_ihi, scale.fortran_vec (), info
                                 F77_CHAR_ARG_LEN (1)));

      ilo = t_ilo;
      ihi = t_ihi;
    }

    template <>
    OCTAVE_API ComplexMatrix
    aepbalance<ComplexMatrix>::balancing_matrix (void) const
    {
      F77_INT n = to_f77_int (balanced_mat.rows ());

      ComplexMatrix balancing_mat (n, n, 0.0);
      for (F77_INT i = 0; i < n; i++)
        balancing_mat.elem (i, i) = 1.0;

      F77_INT info;
      F77_INT t_ilo = to_f77_int (ilo);
      F77_INT t_ihi = to_f77_int (ihi);

      char side = 'R';

      F77_XFCN (zgebak, ZGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                                 F77_CONST_CHAR_ARG2 (&side, 1),
                                 n, t_ilo, t_ihi, scale.data (), n,
                                 F77_DBLE_CMPLX_ARG (balancing_mat.fortran_vec ()),
                                 n, info
                                 F77_CHAR_ARG_LEN (1)
                                 F77_CHAR_ARG_LEN (1)));

      return balancing_mat;
    }

    template <>
    OCTAVE_API
    aepbalance<FloatComplexMatrix>::aepbalance (const FloatComplexMatrix& a,
                                                bool noperm, bool noscal)
      : balanced_mat (a), scale (), ilo (), ihi (),
        job (get_job (noperm, noscal))
    {
      F77_INT n = to_f77_int (a.cols ());

      if (a.rows () != n)
        (*current_liboctave_error_handler)
          ("aepbalance: requires square matrix");

      scale = FloatColumnVector (n);

      F77_INT info, t_ilo, t_ihi;

      F77_XFCN (cgebal, CGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1), n,
                                 F77_CMPLX_ARG (balanced_mat.fortran_vec ()),
                                 n, t_ilo, t_ihi, scale.fortran_vec (), info
                                 F77_CHAR_ARG_LEN (1)));

      ilo = t_ilo;
      ihi = t_ihi;
    }

    template <>
    OCTAVE_API FloatComplexMatrix
    aepbalance<FloatComplexMatrix>::balancing_matrix (void) const
    {
      F77_INT n = to_f77_int (balanced_mat.rows ());

      FloatComplexMatrix balancing_mat (n, n, 0.0);
      for (F77_INT i = 0; i < n; i++)
        balancing_mat.elem (i, i) = 1.0;

      F77_INT info;
      F77_INT t_ilo = to_f77_int (ilo);
      F77_INT t_ihi = to_f77_int (ihi);

      char side = 'R';

      F77_XFCN (cgebak, CGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                                 F77_CONST_CHAR_ARG2 (&side, 1),
                                 n, t_ilo, t_ihi, scale.data (), n,
                                 F77_CMPLX_ARG (balancing_mat.fortran_vec ()),
                                 n, info
                                 F77_CHAR_ARG_LEN (1)
                                 F77_CHAR_ARG_LEN (1)));

      return balancing_mat;
    }

    // Instantiations we need.

    template class aepbalance<Matrix>;

    template class aepbalance<FloatMatrix>;

    template class aepbalance<ComplexMatrix>;

    template class aepbalance<FloatComplexMatrix>;
  }
}
