////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2023 The Octave Project Developers
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

#include "mx-ui8nda-i8.h"
#include "mx-ui8nda-i16.h"
#include "mx-ui8nda-ui16.h"
#include "mx-ui8nda-i32.h"
#include "mx-ui8nda-ui32.h"
#include "mx-ui8nda-i64.h"
#include "mx-ui8nda-ui64.h"

#include "mx-ui8nda-i8nda.h"
#include "mx-ui8nda-i16nda.h"
#include "mx-ui8nda-ui16nda.h"
#include "mx-ui8nda-i32nda.h"
#include "mx-ui8nda-ui32nda.h"
#include "mx-ui8nda-i64nda.h"
#include "mx-ui8nda-ui64nda.h"

#include "mx-ui8-i8nda.h"
#include "mx-ui8-i16nda.h"
#include "mx-ui8-ui16nda.h"
#include "mx-ui8-i32nda.h"
#include "mx-ui8-ui32nda.h"
#include "mx-ui8-i64nda.h"
#include "mx-ui8-ui64nda.h"

#include "mx-ui8nda-s.h"
#include "mx-s-ui8nda.h"

#include "mx-ui8nda-nda.h"
#include "mx-nda-ui8nda.h"

#include "mx-ui8-nda.h"
#include "mx-nda-ui8.h"

#include "mx-ui8nda-fs.h"
#include "mx-fs-ui8nda.h"

#include "mx-ui8nda-fnda.h"
#include "mx-fnda-ui8nda.h"

#include "mx-ui8-fnda.h"
#include "mx-fnda-ui8.h"

#include "errwarn.h"
#include "ovl.h"
#include "ov.h"
#include "ov-int16.h"
#include "ov-int32.h"
#include "ov-int64.h"
#include "ov-int8.h"
#include "ov-uint16.h"
#include "ov-uint32.h"
#include "ov-uint64.h"
#include "ov-uint8.h"
#include "ov-scalar.h"
#include "ov-float.h"
#include "ov-complex.h"
#include "ov-flt-complex.h"
#include "ov-re-mat.h"
#include "ov-flt-re-mat.h"
#include "ov-cx-mat.h"
#include "ov-flt-cx-mat.h"
#include "ov-typeinfo.h"
#include "ov-null-mat.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

#include "op-int.h"

OCTAVE_BEGIN_NAMESPACE(octave)

OCTAVE_INT_OPS (uint8)

OCTAVE_MS_INT_ASSIGN_OPS (mi8, uint8_, int8_, int8_)
OCTAVE_MS_INT_ASSIGN_OPS (mi16, uint8_, int16_, int16_)
OCTAVE_MS_INT_ASSIGN_OPS (mui16, uint8_, uint16_, uint16_)
OCTAVE_MS_INT_ASSIGN_OPS (mi32, uint8_, int32_, int32_)
OCTAVE_MS_INT_ASSIGN_OPS (mui32, uint8_, uint32_, uint32_)
OCTAVE_MS_INT_ASSIGN_OPS (mi64, uint8_, int64_, int64_)
OCTAVE_MS_INT_ASSIGN_OPS (mui64, uint8_, uint64_, uint64_)

OCTAVE_MM_INT_ASSIGN_OPS (mmi8, uint8_, int8_, int8_)
OCTAVE_MM_INT_ASSIGN_OPS (mmi16, uint8_, int16_, int16_)
OCTAVE_MM_INT_ASSIGN_OPS (mmui16, uint8_, uint16_, uint16_)
OCTAVE_MM_INT_ASSIGN_OPS (mmi32, uint8_, int32_, int32_)
OCTAVE_MM_INT_ASSIGN_OPS (mmui32, uint8_, uint32_, uint32_)
OCTAVE_MM_INT_ASSIGN_OPS (mmi64, uint8_, int64_, int64_)
OCTAVE_MM_INT_ASSIGN_OPS (mmui64, uint8_, uint64_, uint64_)

OCTAVE_MIXED_INT_CMP_OPS (uint8, int8)
OCTAVE_MIXED_INT_CMP_OPS (uint8, int16)
OCTAVE_MIXED_INT_CMP_OPS (uint8, uint16)
OCTAVE_MIXED_INT_CMP_OPS (uint8, int32)
OCTAVE_MIXED_INT_CMP_OPS (uint8, uint32)
OCTAVE_MIXED_INT_CMP_OPS (uint8, int64)
OCTAVE_MIXED_INT_CMP_OPS (uint8, uint64)

void
install_ui8_ui8_ops (octave::type_info& ti)
{
  OCTAVE_INSTALL_INT_OPS (uint8)

  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mi8, uint8_, int8_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mi16, uint8_, int16_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mui16, uint8_, uint16_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mi32, uint8_, int32_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mui32, uint8_, uint32_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mi64, uint8_, int64_);
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mui64, uint8_, uint64_);

  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmi8, uint8_, int8_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmi16, uint8_, int16_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmui16, uint8_, uint16_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmi32, uint8_, int32_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmui32, uint8_, uint32_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmi64, uint8_, int64_);
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmui64, uint8_, uint64_);

  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, int8);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, int16);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, uint16);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, int32);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, uint32);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, int64);
  OCTAVE_INSTALL_SM_INT_ASSIGNCONV (uint8, uint64);

  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, int8);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, int16);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, uint16);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, int32);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, uint32);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, int64);
  OCTAVE_INSTALL_MIXED_INT_CMP_OPS (uint8, uint64);
}

OCTAVE_END_NAMESPACE(octave)
