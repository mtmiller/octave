////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2022 The Octave Project Developers
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

#if ! defined (octave_symtab_h)
#define octave_symtab_h 1

#include "octave-config.h"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include "glob-match.h"
#include "lo-regexp.h"
#include "oct-refcount.h"

class tree_argument_list;
class octave_user_function;

#include "fcn-info.h"
#include "ov.h"
#include "ovl.h"
#include "symscope.h"

OCTAVE_NAMESPACE_BEGIN

  class interpreter;

  class OCTINTERP_API symbol_table
  {
  public:

    // Make symbol_table::scope and symbol_table::fcn_info valid type names.
    typedef octave::symbol_scope scope;
    typedef octave::fcn_info fcn_info;

    symbol_table (interpreter& interp);

    // No copying!

    symbol_table (const symbol_table&) = delete;

    symbol_table& operator = (const symbol_table&) = delete;

    ~symbol_table (void) = default;

    symbol_scope current_scope (void) const;

    bool is_built_in_function_name (const std::string& name);

    octave_value find_scoped_function (const std::string& name,
                                       const symbol_scope& search_scope);

    octave_value find_private_function (const std::string& dir_name,
                                        const std::string& name);

    // FIXME: this function only finds legacy class methods, not
    // classdef methods.
    octave_value find_method (const std::string& name,
                              const std::string& dispatch_type);

    octave_value find_built_in_function (const std::string& name);

    octave_value find_autoload (const std::string& name);

    octave_value
    builtin_find (const std::string& name,
                  const symbol_scope& search_scope = symbol_scope ());

    octave_value
    fcn_table_find (const std::string& name,
                    const octave_value_list& args = ovl (),
                    const symbol_scope& search_scope = symbol_scope ());

    // If NAME is of the form @CLASS/FUNCTION, call
    //
    //   find_method (FUNCTION, CLASS)
    //
    // otherwise call
    //
    //   find_function (NAME, ovl ())

    octave_value
    find_function (const std::string& name,
                   const symbol_scope& search_scope = symbol_scope ());

    // NAME should just be function name; dispatch type determined
    // from types of ARGS.

    octave_value
    find_function (const std::string& name,
                   const octave_value_list& args,
                   const symbol_scope& search_scope = symbol_scope ());

    octave_value find_user_function (const std::string& name);

    octave_value find_cmdline_function (const std::string& name);

    void install_cmdline_function (const std::string& name,
                                   const octave_value& fcn);

    // Install local function FCN named NAME.  FILE_NAME is the name of
    // the file containing the local function.

    void install_local_function (const std::string& name,
                                 const octave_value& fcn,
                                 const std::string& file_name);

    void install_user_function (const std::string& name,
                                const octave_value& fcn);

    // FIXME: should we ensure that FCN really is a built-in function
    // object?
    void install_built_in_function (const std::string& name,
                                    const octave_value& fcn);

    // This is written as two separate functions instead of a single
    // function with default values so that it will work properly with
    // unwind_protect.

    void clear_functions (bool force = false);

    void clear_function (const std::string& name);

    void clear_function_pattern (const std::string& pat);

    void clear_function_regexp (const std::string& pat);

    void clear_user_function (const std::string& name);

    // This clears oct and mex files, including autoloads.
    void clear_dld_function (const std::string& name);

    void clear_mex_functions (void);

    bool set_class_relationship (const std::string& sup_class,
                                 const std::string& inf_class);

    bool is_superiorto (const std::string& a, const std::string& b);

    void alias_built_in_function (const std::string& alias,
                                  const std::string& name);

    void install_built_in_dispatch (const std::string& name,
                                    const std::string& klass);

    std::list<std::string> user_function_names (void);

    std::list<std::string> built_in_function_names (void);

    std::list<std::string> cmdline_function_names (void);

    octave_value dump (void) const;

    void add_to_parent_map (const std::string& classname,
                            const std::list<std::string>& parent_list);

    std::list<std::string> parent_classes (const std::string& dispatch_type);

    void cleanup (void);

    fcn_info * get_fcn_info (const std::string& name);

    // The remaining functions are all provided for backward
    // compatibility.  New code should use the functions provided by the
    // interpreter class.

  private:

    // Remove when corresponding public deprecated function is removed.
    bool at_top_level_deprecated (void);

    // Remove when corresponding public deprecated function is removed.
    octave_value varval_deprecated (const std::string& name) const;

    // Remove when corresponding public deprecated function is removed.
    octave_value global_varval_deprecated (const std::string& name) const;

    // Remove when corresponding public deprecated function is removed.
    octave_value top_level_varval_deprecated (const std::string& name) const;

    // Remove when corresponding public deprecated function is removed.
    std::list<std::string> global_variable_names_deprecated (void);

    // Remove when corresponding public deprecated function is removed.
    std::list<std::string> top_level_variable_names_deprecated (void);

    // Remove when corresponding public deprecated function is removed.
    std::list<std::string> variable_names_deprecated (void);

    // Remove when corresponding public deprecated function is removed.
    void assign_deprecated (const std::string& name,
                            const octave_value& value = octave_value ());

    // Remove when corresponding public deprecated function is removed.
    // Note, FORCE_ADD no longer has any meaning.
    void assign_deprecated (const std::string& name, const octave_value& value,
                            bool /*force_add*/);

    // Remove when corresponding public deprecated function is removed.
    void clear_all_deprecated (bool force = false);

    // Remove when corresponding public deprecated function is removed.
    void clear_global_deprecated (const std::string& name);

    // Remove when corresponding public deprecated function is removed.
    void clear_global_pattern_deprecated (const std::string& pattern);

    // Remove when corresponding public deprecated function is removed.
    void clear_symbol_deprecated (const std::string& name);

    // Remove when corresponding public deprecated function is removed.
    void clear_symbol_pattern_deprecated (const std::string& pattern);

    // Remove when corresponding public deprecated function is removed.
    void global_assign_deprecated (const std::string& name,
                                   const octave_value& value = octave_value ());

    // Remove when corresponding public deprecated function is removed.
    void top_level_assign_deprecated (const std::string& name,
                                      const octave_value& value = octave_value ());

  public:

#if defined (OCTAVE_PROVIDE_DEPRECATED_SYMBOLS)
    OCTAVE_DEPRECATED (6, "use 'interpreter::at_top_level' instead")
    bool at_top_level (void)
    {
      return at_top_level_deprecated ();
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::varval' instead")
    octave_value varval (const std::string& name) const
    {
      return varval_deprecated (name);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::global_varval' instead")
    octave_value global_varval (const std::string& name) const
    {
      return global_varval_deprecated (name);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::top_level_varval' instead")
    octave_value top_level_varval (const std::string& name) const
    {
      return top_level_varval_deprecated (name);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::global_variable_names' instead")
    std::list<std::string> global_variable_names (void)
    {
      return global_variable_names_deprecated ();
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::top_level_variable_names' instead")
    std::list<std::string> top_level_variable_names (void)
    {
      return top_level_variable_names_deprecated ();
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::variable_names' instead")
    std::list<std::string> variable_names (void)
    {
      return variable_names_deprecated ();
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::assign' instead")
    void assign (const std::string& name,
                 const octave_value& value = octave_value ())
    {
      assign_deprecated (name, value);
    }

    // Note, FORCE_ADD no longer has any meaning.
    OCTAVE_DEPRECATED (6, "use 'interpreter::assign' instead")
    void assign (const std::string& name, const octave_value& value,
                 bool /*force_add*/)
    {
      assign_deprecated (name, value);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::clear_all' instead")
    void clear_all (bool force = false)
    {
      clear_all_deprecated (force);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::clear_global' instead")
    void clear_global (const std::string& name)
    {
      clear_global_deprecated (name);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::clear_global_pattern' instead")
    void clear_global_pattern (const std::string& pattern)
    {
      clear_global_pattern_deprecated (pattern);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::clear_symbol' instead")
    void clear_symbol (const std::string& name)
    {
      clear_symbol_deprecated (name);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::clear_symbol_pattern' instead")
    void clear_symbol_pattern (const std::string& pattern)
    {
      clear_symbol_pattern_deprecated (pattern);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::global_assign' instead")
    void global_assign (const std::string& name,
                        const octave_value& value = octave_value ())
    {
      global_assign_deprecated (name, value);
    }

    OCTAVE_DEPRECATED (6, "use 'interpreter::top_level_assign' instead")
    void top_level_assign (const std::string& name,
                           const octave_value& value = octave_value ())
    {
      top_level_assign_deprecated (name, value);
    }
#endif

  private:

    interpreter& m_interpreter;

    typedef std::map<std::string, octave_value>::const_iterator
      global_symbols_const_iterator;
    typedef std::map<std::string, octave_value>::iterator
      global_symbols_iterator;

    typedef std::map<std::string, fcn_info>::const_iterator
      fcn_table_const_iterator;
    typedef std::map<std::string, fcn_info>::iterator
      fcn_table_iterator;

    // Map from function names to function info (private
    // functions, class constructors, class methods, etc.)
    // Note that subfunctions are defined in the scope that contains
    // them.
    std::map<std::string, fcn_info> m_fcn_table;

    // Map from class names to set of classes that have lower
    // precedence.
    std::map<std::string, std::set<std::string>> m_class_precedence_table;

    typedef std::map<std::string, std::set<std::string>>::const_iterator
      class_precedence_table_const_iterator;
    typedef std::map<std::string, std::set<std::string>>::iterator
      class_precedence_table_iterator;

    // Map from class names to parent class names.
    std::map<std::string, std::list<std::string>> m_parent_map;

    typedef std::map<std::string, std::list<std::string>>::const_iterator
      const_parent_map_iterator;
    typedef std::map<std::string, std::list<std::string>>::iterator
      parent_map_iterator;

    octave_value dump_fcn_table_map (void) const;

    // This function is generated automatically by mk-builtins.pl.
    void install_builtins (void);
  };

OCTAVE_NAMESPACE_END

#endif
