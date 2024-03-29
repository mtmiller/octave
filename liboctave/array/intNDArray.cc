////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2004-2024 The Octave Project Developers
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

// This file should not include config.h.  It is only included in other
// C++ source files that should have included config.h before including
// this file.

#include "Array-util.h"
#include "mx-base.h"
#include "lo-ieee.h"
#include "mx-inlines.cc"

// unary operations

template <typename T>
boolNDArray
intNDArray<T>::operator ! () const
{
  boolNDArray b (this->dims ());

  for (octave_idx_type i = 0; i < this->numel (); i++)
    b.elem (i) = ! this->elem (i);

  return b;
}

template <typename T>
bool
intNDArray<T>::any_element_not_one_or_zero () const
{
  octave_idx_type nel = this->numel ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
      T val = this->elem (i);

      if (val != 0.0 && val != 1.0)
        return true;
    }

  return false;
}

template <typename T>
intNDArray<T>
intNDArray<T>::diag (octave_idx_type k) const
{
  return MArray<T>::diag (k);
}

template <typename T>
intNDArray<T>
intNDArray<T>::diag (octave_idx_type m, octave_idx_type n) const
{
  return MArray<T>::diag (m, n);
}

// FIXME: this is not quite the right thing.

template <typename T>
boolNDArray
intNDArray<T>::all (int dim) const
{
  return do_mx_red_op<bool, T > (*this, dim, mx_inline_all);
}

template <typename T>
boolNDArray
intNDArray<T>::any (int dim) const
{
  return do_mx_red_op<bool, T > (*this, dim, mx_inline_any);
}

template <typename T>
void
intNDArray<T>::increment_index (Array<octave_idx_type>& ra_idx,
                                const dim_vector& dimensions,
                                int start_dimension)
{
  ::increment_index (ra_idx, dimensions, start_dimension);
}

template <typename T>
octave_idx_type
intNDArray<T>::compute_index (Array<octave_idx_type>& ra_idx,
                              const dim_vector& dimensions)
{
  return ::compute_index (ra_idx, dimensions);
}

template <typename T>
intNDArray<T>
intNDArray<T>::concat (const intNDArray<T>& rb,
                       const Array<octave_idx_type>& ra_idx)
{
  if (rb.numel () > 0)
    insert (rb, ra_idx);
  return *this;
}

template <typename T>
intNDArray<T>&
intNDArray<T>::insert (const intNDArray<T>& a, octave_idx_type r,
                       octave_idx_type c)
{
  Array<T>::insert (a, r, c);
  return *this;
}

template <typename T>
intNDArray<T>&
intNDArray<T>::insert (const intNDArray<T>& a,
                       const Array<octave_idx_type>& ra_idx)
{
  Array<T>::insert (a, ra_idx);
  return *this;
}

// This contains no information on the array structure !!!

template <typename T>
std::ostream&
operator << (std::ostream& os, const intNDArray<T>& a)
{
  octave_idx_type nel = a.numel ();

  for (octave_idx_type i = 0; i < nel; i++)
    os << ' ' << a.elem (i) << "\n";

  return os;
}

template <typename T>
std::istream&
operator >> (std::istream& is, intNDArray<T>& a)
{
  octave_idx_type nel = a.numel ();

  if (nel > 0)
    {
      T tmp;

      for (octave_idx_type i = 0; i < nel; i++)
        {
          is >> tmp;

          if (is)
            a.elem (i) = tmp;
          else
            return is;
        }
    }

  return is;
}

// FIXME: should abs and signum just be mapper functions?

template <typename T>
intNDArray<T>
intNDArray<T>::abs () const
{
  octave_idx_type nel = this->numel ();
  intNDArray<T> ret (this->dims ());

  for (octave_idx_type i = 0; i < nel; i++)
    {
      T val = this->elem (i);
      ret.xelem (i) = val.abs ();
    }

  return ret;
}

template <typename T>
intNDArray<T>
intNDArray<T>::signum () const
{
  octave_idx_type nel = this->numel ();
  intNDArray<T> ret (this->dims ());

  for (octave_idx_type i = 0; i < nel; i++)
    {
      T val = this->elem (i);
      ret.xelem (i) = val.signum ();
    }

  return ret;
}

template <typename T>
intNDArray<T>
intNDArray<T>::prod (int dim) const
{
  return do_mx_red_op<T, T> (*this, dim, mx_inline_prod);
}

template <typename T>
intNDArray<T>
intNDArray<T>::sum (int dim) const
{
  return do_mx_red_op<T, T> (*this, dim, mx_inline_sum);
}

template <typename T>
NDArray
intNDArray<T>::dsum (int dim) const
{
  return do_mx_red_op<double, T> (*this, dim, mx_inline_dsum);
}

template <typename T>
intNDArray<T>
intNDArray<T>::cumsum (int dim) const
{
  return do_mx_cum_op<T, T> (*this, dim, mx_inline_cumsum);
}

template <typename T>
intNDArray<T>
intNDArray<T>::max (int dim) const
{
  return do_mx_minmax_op<T> (*this, dim, mx_inline_max);
}

template <typename T>
intNDArray<T>
intNDArray<T>::max (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_minmax_op<T> (*this, idx_arg, dim, mx_inline_max);
}

template <typename T>
intNDArray<T>
intNDArray<T>::min (int dim) const
{
  return do_mx_minmax_op<T> (*this, dim, mx_inline_min);
}

template <typename T>
intNDArray<T>
intNDArray<T>::min (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_minmax_op<T> (*this, idx_arg, dim, mx_inline_min);
}

template <typename T>
intNDArray<T>
intNDArray<T>::cummax (int dim) const
{
  return do_mx_cumminmax_op<T> (*this, dim, mx_inline_cummax);
}

template <typename T>
intNDArray<T>
intNDArray<T>::cummax (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_cumminmax_op<T> (*this, idx_arg, dim, mx_inline_cummax);
}

template <typename T>
intNDArray<T>
intNDArray<T>::cummin (int dim) const
{
  return do_mx_cumminmax_op<T> (*this, dim, mx_inline_cummin);
}

template <typename T>
intNDArray<T>
intNDArray<T>::cummin (Array<octave_idx_type>& idx_arg, int dim) const
{
  return do_mx_cumminmax_op<T> (*this, idx_arg, dim, mx_inline_cummin);
}

template <typename T>
intNDArray<T>
intNDArray<T>::diff (octave_idx_type order, int dim) const
{
  return do_mx_diff_op<T> (*this, dim, order, mx_inline_diff);
}

#if defined (__clang__)
#  define INSTANTIATE_INTNDARRAY(T) template class OCTAVE_API intNDArray<T>
#else
#  define INSTANTIATE_INTNDARRAY(T) template class intNDArray<T>
#endif
