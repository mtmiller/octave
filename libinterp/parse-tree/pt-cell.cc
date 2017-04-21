/*

Copyright (C) 1999-2017 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <iostream>

#include "Cell.h"
#include "ovl.h"
#include "pt-arg-list.h"
#include "pt-exp.h"
#include "pt-cell.h"
#include "pt-walk.h"
#include "ov.h"

namespace octave
{
  tree_expression *
  tree_cell::dup (symbol_table::scope_id scope,
                  symbol_table::context_id context) const
  {
    tree_cell *new_cell = new tree_cell (0, line (), column ());

    new_cell->copy_base (*this, scope, context);

    return new_cell;
  }
}
