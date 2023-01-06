////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2004-2023 The Octave Project Developers
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

#include "uint32NDArray.h"
#include "mx-op-defs.h"
#include "intNDArray.cc"

#include "bsxfun-defs.cc"

INSTANTIATE_INTNDARRAY (octave_uint32);

template OCTAVE_API
std::ostream&
operator << (std::ostream& os, const intNDArray<octave_uint32>& a);

template OCTAVE_API
std::istream&
operator >> (std::istream& is, intNDArray<octave_uint32>& a);

NDS_CMP_OPS (uint32NDArray, octave_uint32)
NDS_BOOL_OPS (uint32NDArray, octave_uint32)

SND_CMP_OPS (octave_uint32, uint32NDArray)
SND_BOOL_OPS (octave_uint32, uint32NDArray)

NDND_CMP_OPS (uint32NDArray, uint32NDArray)
NDND_BOOL_OPS (uint32NDArray, uint32NDArray)

MINMAX_FCNS (uint32NDArray, octave_uint32)

BSXFUN_STDOP_DEFS_MXLOOP (uint32NDArray)
BSXFUN_STDREL_DEFS_MXLOOP (uint32NDArray)

BSXFUN_OP_DEF_MXLOOP (pow, uint32NDArray, mx_inline_pow)
BSXFUN_POW_MIXED_MXLOOP (uint32NDArray)
