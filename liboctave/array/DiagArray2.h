////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2020 The Octave Project Developers
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

#if ! defined (octave_DiagArray2_h)
#define octave_DiagArray2_h 1

#include "octave-config.h"

#include <cassert>
#include <cstdlib>

#include "Array.h"

// Array<T> is inherited privately so that some methods, like index, don't
// produce unexpected results.

template <typename T>
class
OCTAVE_API
DiagArray2 : protected Array<T>
{
protected:
  octave_idx_type d1, d2;

public:

  using typename Array<T>::element_type;

  DiagArray2 (void)
    : Array<T> (), d1 (0), d2 (0) { }

  DiagArray2 (octave_idx_type r, octave_idx_type c)
    : Array<T> (dim_vector (std::min (r, c), 1)), d1 (r), d2 (c) { }

  DiagArray2 (octave_idx_type r, octave_idx_type c, const T& val)
    : Array<T> (dim_vector (std::min (r, c), 1), val), d1 (r), d2 (c) { }

  explicit DiagArray2 (const Array<T>& a)
    : Array<T> (a.as_column ()), d1 (a.numel ()), d2 (a.numel ()) { }

  DiagArray2 (const Array<T>& a, octave_idx_type r, octave_idx_type c);

  DiagArray2 (const DiagArray2<T>& a)
    : Array<T> (a), d1 (a.d1), d2 (a.d2) { }

  template <typename U>
  DiagArray2 (const DiagArray2<U>& a)
    : Array<T> (a.extract_diag ()), d1 (a.dim1 ()), d2 (a.dim2 ()) { }

  ~DiagArray2 (void) = default;

  DiagArray2<T>& operator = (const DiagArray2<T>& a)
  {
    if (this != &a)
      {
        Array<T>::operator = (a);
        d1 = a.d1;
        d2 = a.d2;
      }

    return *this;
  }

  octave_idx_type dim1 (void) const { return d1; }
  octave_idx_type dim2 (void) const { return d2; }

  octave_idx_type rows (void) const { return dim1 (); }
  octave_idx_type cols (void) const { return dim2 (); }
  octave_idx_type columns (void) const { return dim2 (); }

  octave_idx_type diag_length (void) const { return Array<T>::numel (); }
  // FIXME: a dangerous ambiguity?
  octave_idx_type length (void) const { return Array<T>::numel (); }
  octave_idx_type nelem (void) const { return dim1 () * dim2 (); }
  octave_idx_type numel (void) const { return nelem (); }

  size_t byte_size (void) const { return Array<T>::byte_size (); }

  dim_vector dims (void) const { return dim_vector (d1, d2); }

  bool isempty (void) const { return numel () == 0; }

  int ndims (void) const { return 2; }

  Array<T> extract_diag (octave_idx_type k = 0) const;

  DiagArray2<T> build_diag_matrix () const
  {
    return DiagArray2<T> (array_value ());
  }

  // Warning: the non-const two-index versions will silently ignore assignments
  // to off-diagonal elements.

  T elem (octave_idx_type r, octave_idx_type c) const
  {
    return (r == c) ? Array<T>::elem (r) : T (0);
  }

  T& elem (octave_idx_type r, octave_idx_type c);

  T dgelem (octave_idx_type i) const
  { return Array<T>::elem (i); }

  T& dgelem (octave_idx_type i)
  { return Array<T>::elem (i); }

  T checkelem (octave_idx_type r, octave_idx_type c) const
  { return check_idx (r, c) ? elem (r, c) : T (0); }

  T operator () (octave_idx_type r, octave_idx_type c) const
  {
    return elem (r, c);
  }

  T& checkelem (octave_idx_type r, octave_idx_type c);

  T& operator () (octave_idx_type r, octave_idx_type c)
  {
    return elem (r, c);
  }

  // No checking.

  T xelem (octave_idx_type r, octave_idx_type c) const
  {
    return (r == c) ? Array<T>::xelem (r) : T (0);
  }

  T& dgxelem (octave_idx_type i)
  { return Array<T>::xelem (i); }

  T dgxelem (octave_idx_type i) const
  { return Array<T>::xelem (i); }

  void resize (octave_idx_type n, octave_idx_type m, const T& rfv);
  void resize (octave_idx_type n, octave_idx_type m)
  {
    resize (n, m, Array<T>::resize_fill_value ());
  }

  DiagArray2<T> transpose (void) const;
  DiagArray2<T> hermitian (T (*fcn) (const T&) = nullptr) const;

  Array<T> array_value (void) const;

  const T * data (void) const { return Array<T>::data (); }

  const T * fortran_vec (void) const { return Array<T>::fortran_vec (); }

  T * fortran_vec (void) { return Array<T>::fortran_vec (); }

  void print_info (std::ostream& os, const std::string& prefix) const
  { Array<T>::print_info (os, prefix); }

private:

  bool check_idx (octave_idx_type r, octave_idx_type c) const;
};

#endif
