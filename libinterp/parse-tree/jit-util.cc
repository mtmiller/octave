////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2012-2021 The Octave Project Developers
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

// defines required by llvm
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#if defined (HAVE_LLVM)

#if defined (HAVE_LLVM_IR_FUNCTION_H)
#  include <llvm/IR/Value.h>
#else
#  include <llvm/Value.h>
#endif

#include <llvm/Support/raw_os_ostream.h>

namespace octave
{
  std::ostream&
  operator<< (std::ostream& os, const llvm::Value& v)
  {
    llvm::raw_os_ostream llvm_out (os);
    v.print (llvm_out);
    return os;
  }
}

#endif
