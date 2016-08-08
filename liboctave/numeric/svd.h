/*

Copyright (C) 2016 Carnë Draug
Copyright (C) 1994-2015 John W. Eaton

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

#if ! defined (octave_svd_h)
#define octave_svd_h 1

#include "octave-config.h"

#include <iosfwd>

template <typename T>
class
svd
{
public:

  typedef typename T::real_diag_matrix_type DM_T;

  enum class Type
  {
    std,
    economy,
    sigma_only
  };

  enum class Driver
  {
    GESVD,
    GESDD
  };

  svd (void)
    : type (), driver (), left_sm (), sigma (), right_sm ()
  { }

  svd (const T& a, svd::Type type = svd::Type::std,
       svd::Driver driver = svd::Driver::GESVD);

  svd (const svd& a)
    : type (a.type), driver (a.driver), left_sm (a.left_sm), sigma (a.sigma),
      right_sm (a.right_sm)
  { }

  svd& operator = (const svd& a)
  {
    if (this != &a)
      {
        type = a.type;
        left_sm = a.left_sm;
        sigma = a.sigma;
        right_sm = a.right_sm;
        driver = a.driver;
      }

    return *this;
  }

  ~svd (void) { }

  T left_singular_matrix (void) const;

  DM_T singular_values (void) const { return sigma; }

  T right_singular_matrix (void) const;

private:

  typedef typename T::element_type P;
  typedef typename DM_T::element_type DM_P;

  svd::Type type;
  svd::Driver driver;

  T left_sm;
  DM_T sigma;
  T right_sm;

  void gesvd (char& jobu, char& jobv, octave_idx_type m, octave_idx_type n,
              P* tmp_data, octave_idx_type m1, DM_P* s_vec, P* u, P* vt,
              octave_idx_type nrow_vt1, T& work, octave_idx_type& lwork,
              octave_idx_type& info);

  void gesdd (char& jobz, octave_idx_type m, octave_idx_type n,
              P* tmp_data, octave_idx_type m1, DM_P* s_vec, P* u, P* vt,
              octave_idx_type nrow_vt1, T& work, octave_idx_type& lwork,
              octave_idx_type* iwork, octave_idx_type& info);

};

#endif