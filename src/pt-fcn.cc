// pt-fcn.cc                                          -*- C++ -*-
/*

Copyright (C) 1996 John W. Eaton

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

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <strstream.h>

#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "help.h"
#include "pager.h"
#include "symtab.h"
#include "toplev.h"
#include "pt-const.h"
#include "pt-exp.h"
#include "pt-fcn.h"
#include "pt-misc.h"
#include "unwind-prot.h"
#include "user-prefs.h"
#include "utils.h"
#include "variables.h"

// Nonzero means we're returning from a function.
extern int returning;

// Nonzero means we're breaking out of a loop or function body.
extern int breaking;

// User defined functions.

void
tree_function::install_nargin_and_nargout (void)
{
  nargin_sr = sym_tab->lookup ("nargin", 1, 0);
  nargout_sr = sym_tab->lookup ("nargout", 1, 0);
}

void
tree_function::bind_nargin_and_nargout (int nargin, int nargout)
{
  tree_constant *tmp;

  tmp = new tree_constant (nargin);
  nargin_sr->define (tmp);

  tmp = new tree_constant (nargout);
  nargout_sr->define (tmp);
}

tree_function::~tree_function (void)
{
  delete param_list;
  delete ret_list;
  delete sym_tab;
  delete cmd_list;
  delete vr_list;
}

#if 0
tree_function *
tree_function::define (tree statement_list *t)
{
  cmd_list = t;
  return this;
}
#endif

tree_function *
tree_function::define_param_list (tree_parameter_list *t)
{
  param_list = t;

  if (param_list)
    {
      num_named_args = param_list->length ();
      curr_va_arg_number = num_named_args;
    }

  return this;
}

tree_function *
tree_function::define_ret_list (tree_parameter_list *t)
{
  ret_list = t;

  if (ret_list && ret_list->takes_varargs ())
    vr_list = new tree_va_return_list;
 
  return this;
}

void
tree_function::stash_fcn_file_name (void)
{
  if (fcn_name.empty ())
    file_name = "";
  else
    file_name = fcn_file_in_path (fcn_name);
}

void
tree_function::mark_as_system_fcn_file (void)
{
  if (! file_name.empty ())
    {
      // We really should stash the whole path to the file we found,
      // when we looked it up, to avoid possible race conditions...
      // XXX FIXME XXX
      //
      // We probably also don't need to get the library directory
      // every time, but since this function is only called when the
      // function file is parsed, it probably doesn't matter that
      // much.

      string ff_name = fcn_file_in_path (file_name);

      string system_dir = octave_fcn_file_dir ();

      if (system_dir.compare (ff_name, 0, system_dir.length ()) == 0)
	system_fcn_file = 1;
    }
  else
    system_fcn_file = 0;
}

bool
tree_function::takes_varargs (void) const
{
  return (param_list && param_list->takes_varargs ());
}

tree_constant
tree_function::octave_va_arg (void)
{
  tree_constant retval;

  if (curr_va_arg_number < num_args_passed)
    retval = args_passed (curr_va_arg_number++);
  else
    ::error ("va_arg: error getting arg number %d -- only %d provided",
	     curr_va_arg_number + 1, num_args_passed);

  return retval;
}

Octave_object
tree_function::octave_all_va_args (void)
{
  Octave_object retval;

  retval.resize (num_args_passed - num_named_args);

  int k = 0;
  for (int i = num_named_args; i < num_args_passed; i++)
    retval(k++) = args_passed(i);

  return retval;
}

bool
tree_function::takes_var_return (void) const
{
  return (ret_list && ret_list->takes_varargs ());
}

void
tree_function::octave_vr_val (const tree_constant& val)
{
  assert (vr_list);

  vr_list->append (val);
}

void
tree_function::stash_function_name (const string& s)
{
  fcn_name = s;
}

tree_constant
tree_function::eval (bool print)
{
  tree_constant retval;

  if (error_state || ! cmd_list)
    return retval;

  Octave_object tmp_args;
  Octave_object tmp = eval (print, 0, tmp_args);

  if (! error_state && tmp.length () > 0)
    retval = tmp(0);

  return retval;
}

// For unwind protect.

static void
pop_symbol_table_context (void *table)
{
  symbol_table *tmp = (symbol_table *) table;
  tmp->pop_context ();
}

static void
delete_vr_list (void *list)
{
  tree_va_return_list *tmp = (tree_va_return_list *) list;
  tmp->clear ();
  delete tmp;
}

static void
clear_symbol_table (void *table)
{
  symbol_table *tmp = (symbol_table *) table;
  tmp->clear ();
}

Octave_object
tree_function::eval (bool /* print */, int nargout, const Octave_object& args)
{
  Octave_object retval;

  if (error_state)
    return retval;

  if (! cmd_list)
    return retval;

  int nargin = args.length ();

  begin_unwind_frame ("func_eval");

  unwind_protect_int (call_depth);
  call_depth++;

  if (call_depth > 1)
    {
      sym_tab->push_context ();
      add_unwind_protect (pop_symbol_table_context, (void *) sym_tab);

      if (vr_list)
	{
	  // Push new vr_list.

	  unwind_protect_ptr (vr_list);
	  vr_list = new tree_va_return_list;

	  // Clear and delete the new one before restoring the old
	  // one.

	  add_unwind_protect (delete_vr_list, (void *) vr_list);
	}
    }

  if (vr_list)
    vr_list->clear ();

  // Force symbols to be undefined again when this function exits.

  add_unwind_protect (clear_symbol_table, (void *) sym_tab);

  // Save old and set current symbol table context, for
  // eval_undefined_error().

  unwind_protect_ptr (curr_sym_tab);
  curr_sym_tab = sym_tab;

  unwind_protect_ptr (curr_function);
  curr_function = this;

  // XXX FIXME XXX -- ???
  // unwind_protect_ptr (args_passed);

  args_passed = args;

  unwind_protect_int (num_args_passed);
  num_args_passed = nargin;

  unwind_protect_int (num_named_args);
  unwind_protect_int (curr_va_arg_number);

  if (param_list && ! param_list->varargs_only ())
    {
      param_list->define_from_arg_vector (args);
      if (error_state)
	goto abort;
    }

  // The following code is in a separate scope to avoid warnings from
  // G++ about `goto abort' crossing the initialization of some
  // variables.

  {
    bind_nargin_and_nargout (nargin, nargout);

    bool echo_commands
      = (user_pref.echo_executing_commands & ECHO_FUNCTIONS);

    if (echo_commands)
      print_code_function_header ();

    // Evaluate the commands that make up the function.

    bool pf = ! user_pref.silent_functions;
    tree_constant last_computed_value = cmd_list->eval (pf);

    if (echo_commands)
      print_code_function_trailer ();

    if (returning)
      returning = 0;

    if (breaking)
      breaking--;

    if (error_state)
      {
	traceback_error ();
	goto abort;
      }
    
    // Copy return values out.

    if (ret_list)
      {
	if (nargout > 0 && user_pref.define_all_return_values)
	  {
	    tree_constant tmp = builtin_any_variable ("default_return_value");
	    if (tmp.is_defined ())
	      ret_list->initialize_undefined_elements (tmp);
	  }

	retval = ret_list->convert_to_const_vector (vr_list);
      }
    else if (user_pref.return_last_computed_value)
      retval(0) = last_computed_value;
  }

 abort:
  run_unwind_frame ("func_eval");

  return retval;
}

void
tree_function::traceback_error (void)
{
  if (error_state >= 0)
    error_state = -1;

  if (fcn_name.empty ())
    {
      if (file_name.empty ())
	::error ("called from `?unknown?'");
      else
	::error ("called from file `%s'", file_name.c_str ());
    }
  else
    {
      if (file_name.empty ())
	::error ("called from `%s'", fcn_name.c_str ());
      else 
	::error ("called from `%s' in file `%s'",
		 fcn_name.c_str (), file_name.c_str ());
    }
}

void
tree_function::print_code (ostream& os)
{
  print_code_reset ();

  print_code_function_header (os);

  if (cmd_list)
    {
      increment_indent_level ();
      cmd_list->print_code (os);
      decrement_indent_level ();
    }

  print_code_function_trailer (os);
}

void
tree_function::print_code_function_header (void)
{
  ostrstream output_buf;
  print_code_function_header (output_buf);
  output_buf << ends;
  maybe_page_output (output_buf);
}

void
tree_function::print_code_function_header (ostream& os)
{
  print_code_indent (os);

  os << "function ";

  if (ret_list)
    {
      int len = ret_list->length ();

      if (len > 1)
	os << "[";

      ret_list->print_code (os);

      if (len > 1)
	os << "]";

      os << " = ";
    }

  os << (fcn_name.empty () ? string ("(empty)") : fcn_name) << " ";

  if (param_list)
    {
      int len = param_list->length ();
      if (len > 0)
	os << "(";

      param_list->print_code (os);

      if (len > 0)
	{
	  os << ")";
	  print_code_new_line (os);
	}
    }
  else
    {
      os << "()";
      print_code_new_line (os);
    }
}

void
tree_function::print_code_function_trailer (void)
{
  ostrstream output_buf;
  print_code_function_trailer (output_buf);
  output_buf << ends;
  maybe_page_output (output_buf);
}

void
tree_function::print_code_function_trailer (ostream& os)
{
  print_code_indent (os);

  os << "endfunction";

  print_code_new_line (os);
}

DEFUN (va_arg, args, ,
  "va_arg (): return next argument in a function that takes a\n\
variable number of parameters")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 0)
    {
      if (curr_function)
	{
	  if (curr_function->takes_varargs ())
	    retval = curr_function->octave_va_arg ();
	  else
	    {
	      ::error ("va_arg only valid within function taking variable");
	      ::error ("number of arguments");
	    }
	}
      else
	::error ("va_arg only valid within function body");
    }
  else
    print_usage ("va_arg");

  return retval;
}

DEFUN (va_start, args, ,
  "va_start (): reset the pointer to the list of optional arguments\n\
to the beginning")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 0)
    {
      if (curr_function)
	{
	  if (curr_function->takes_varargs ())
	    curr_function->octave_va_start ();
	  else
	    {
	      ::error ("va_start only valid within function taking variable");
	      ::error ("number of arguments");
	    }
	}
      else
	::error ("va_start only valid within function body");
    }
  else
    print_usage ("va_start");

  return retval;
}

DEFUN (vr_val, args, ,
  "vr_val (X): append X to the list of optional return values for a
function that allows a variable number of return values")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      if (curr_function)
	{
	  if (curr_function->takes_var_return ())
	    curr_function->octave_vr_val (args(0));
	  else
	    {
	      ::error ("vr_val only valid within function declared to");
	      ::error ("produce a variable number of values");
	    }
	}
      else
	::error ("vr_val only valid within function body");
    }
  else
    print_usage ("vr_val");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
