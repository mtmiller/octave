////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1995-2023 The Octave Project Developers
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

// Instantiate Arrays of short int values.

#include "Array.h"

// Prevent implicit instantiations on some systems (Windows, others?)
// that can lead to duplicate definitions of static data members.

extern template class OCTAVE_EXTERN_TEMPLATE_API Array<octave::idx_vector>;
extern template class Array<octave_idx_type>;

#include "Array-base.cc"

#define INLINE_ASCENDING_SORT 1
#define INLINE_DESCENDING_SORT 1
#include "oct-sort.cc"

template class octave_sort<short>;

INSTANTIATE_ARRAY (short, OCTAVE_CLASS_TEMPLATE_INSTANTIATION_API);

#include "DiagArray2.h"
#include "DiagArray2.cc"

template class DiagArray2<short>;
