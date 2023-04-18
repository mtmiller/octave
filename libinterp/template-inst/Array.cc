////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2022-2023 The Octave Project Developers
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


// Include this file when instantiating the Array class with new types in
// projects linking to "liboctinterp".

#include "Array-oct.cc"

#include "ov.h"
#include "cdef-fwd.h"

// "Protect" Array<T> instantiations that are exported by liboctinterp from
// being implicitly instantiated in compilation units including this file.

// instantiated in Array-tc.cc
extern template class OCTINTERP_EXTERN_TEMPLATE_API Array<octave_value>;
extern template class OCTINTERP_EXTERN_TEMPLATE_API Array<octave_value *>;
extern template class OCTINTERP_EXTERN_TEMPLATE_API Array<octave::cdef_object>;
