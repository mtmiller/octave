/*

Copyright (C) 2012-2018 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_tree_funcall_h)
#define octave_tree_funcall_h 1

#include "octave-config.h"

#include "ov.h"
#include "ovl.h"
#include "parse.h"
#include "pt-exp.h"
#include "pt-walk.h"

namespace octave
{
  // Function call.  This class only represents function calls that have
  // known functions (most useful for calls to built-in functions that
  // are generated by the parser) and fixed argument lists, known at
  // compile time.

  class tree_funcall : public tree_expression
  {
  public:

    tree_funcall (const octave_value& f, const octave_value_list& a,
                  int l = -1, int c = -1)
      : tree_expression (l, c), m_fcn (f), m_args (a)
    {
      if (! m_fcn.is_function ())
        error ("tree_funcall: invalid function");
    }

    // No copying!

    tree_funcall (const tree_funcall&) = delete;

    tree_funcall& operator = (const tree_funcall&) = delete;

    ~tree_funcall (void) = default;

    bool has_magic_end (void) const { return false; }

    void print (std::ostream& os, bool pr_as_read_syntax = false,
                bool pr_orig_txt = true);

    void print_raw (std::ostream& os, bool pr_as_read_syntax = false,
                    bool pr_orig_txt = true);

    tree_funcall * dup (symbol_scope& scope) const;

    octave_value function (void) const { return m_fcn; }

    octave_value_list arguments (void) const { return m_args; }

    void accept (tree_walker& tw)
    {
      tw.visit_funcall (*this);
    }

  private:

    // Function to call.  Error if not a valid function at time of
    // construction.
    octave_value m_fcn;

    // Argument list.
    octave_value_list m_args;
  };
}

#endif
