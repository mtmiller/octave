/*

Copyright (C) 2004 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <climits>

#include <iostream>
#include <vector>

#include "lo-ieee.h"
#include "lo-utils.h"
#include "mx-base.h"
#include "quit.h"

#include "defun.h"
#include "gripes.h"
#include "oct-obj.h"
#include "oct-lvalue.h"
#include "ops.h"
#include "ov-base.h"
#include "ov-base-int.h"
#include "ov-base-int.cc"
#include "ov-int8.h"
#include "ov-type-conv.h"
#include "pr-output.h"
#include "variables.h"

#include "byte-swap.h"
#include "ls-oct-ascii.h"
#include "ls-utils.h"
#include "ls-hdf5.h"

template class octave_base_matrix<int8NDArray>;

template class octave_base_int_matrix<int8NDArray>;

DEFINE_OCTAVE_ALLOCATOR (octave_int8_matrix);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_int8_matrix,
				     "int8 matrix", "int8");

template class octave_base_scalar<octave_int8>;

template class octave_base_int_scalar<octave_int8>;

DEFINE_OCTAVE_ALLOCATOR (octave_int8_scalar);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_int8_scalar,
				     "int8 scalar", "int8");

DEFUN (int8, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} int8 (@var{x})\n\
Convert @var{x} to 8-bit integer type.\n\
@end deftypefn")
{
  OCTAVE_TYPE_CONV_BODY (int8);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
