////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009-2020 The Octave Project Developers
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

#if ! defined (octave_pt_eval_h)
#define octave_pt_eval_h 1

#include "octave-config.h"

#include <iosfwd>
#include <list>
#include <memory>
#include <set>
#include <stack>
#include <string>

#include "bp-table.h"
#include "call-stack.h"
#include "oct-lvalue.h"
#include "ov.h"
#include "ovl.h"
#include "profiler.h"
#include "pt-walk.h"
#include "stack-frame.h"

class octave_user_code;

namespace octave
{
  class symbol_info_list;
  class symbol_scope;
  class tree_decl_elt;
  class tree_expression;

  class debugger;
  class interpreter;
  class unwind_protect;

  // How to evaluate the code that the parse trees represent.

  class OCTINTERP_API tree_evaluator : public tree_walker
  {
  public:

    enum echo_state
    {
      ECHO_OFF = 0,
      ECHO_SCRIPTS = 1,
      ECHO_FUNCTIONS = 2,
      ECHO_ALL = 4
    };

    template <typename T>
    class value_stack
    {
    public:

      value_stack (void) = default;

      value_stack (const value_stack&) = default;

      value_stack& operator = (const value_stack&) = default;

      ~value_stack (void) = default;

      void push (const T& val) { m_stack.push (val); }

      void pop (void)
      {
        m_stack.pop ();
      }

      T val_pop (void)
      {
        T retval = m_stack.top ();
        m_stack.pop ();
        return retval;
      }

      T top (void) const
      {
        return m_stack.top ();
      }

      size_t size (void) const
      {
        return m_stack.size ();
      }

      bool empty (void) const
      {
        return m_stack.empty ();
      }

      void clear (void)
      {
        while (! m_stack.empty ())
          m_stack.pop ();
      }

    private:

      std::stack<T> m_stack;
    };

    typedef void (*decl_elt_init_fcn) (tree_decl_elt&);

    tree_evaluator (interpreter& interp)
      : m_interpreter (interp), m_statement_context (SC_OTHER),
        m_lvalue_list (nullptr), m_autoload_map (), m_bp_table (*this),
        m_call_stack (*this), m_profiler (), m_debug_frame (0),
        m_debug_mode (false), m_quiet_breakpoint_flag (false),
        m_debugger_stack (), m_max_recursion_depth (256),
        m_whos_line_format ("  %a:4; %ln:6; %cs:16:6:1;  %rb:12;  %lc:-1;\n"),
        m_silent_functions (false), m_string_fill_char (' '),
        m_PS4 ("+ "), m_dbstep_flag (0), m_echo (ECHO_OFF),
        m_echo_state (false), m_echo_file_name (), m_echo_file_pos (1),
        m_echo_files (), m_in_loop_command (false),
        m_breaking (0), m_continuing (0), m_returning (0),
        m_indexed_object (), m_index_list (), m_index_type (),
        m_index_position (0), m_num_indices (0)
    { }

    // No copying!

    tree_evaluator (const tree_evaluator&) = delete;

    tree_evaluator& operator = (const tree_evaluator&) = delete;

    ~tree_evaluator (void) = default;

    bool at_top_level (void) const;

    void eval (std::shared_ptr<tree_statement_list>& stmt_list,
               bool interactive);

    std::string mfilename (const std::string& opt = "") const;

    octave_value_list eval_string (const std::string& eval_str, bool silent,
                                   int& parse_status, int nargout);

    octave_value eval_string (const std::string& eval_str, bool silent,
                              int& parse_status);

    octave_value_list eval_string (const octave_value& arg, bool silent,
                                   int& parse_status, int nargout);

    octave_value_list eval (const std::string& try_code, int nargout);

    octave_value_list eval (const std::string& try_code,
                            const std::string& catch_code, int nargout);

    octave_value_list evalin (const std::string& context,
                              const std::string& try_code, int nargout);

    octave_value_list evalin (const std::string& context,
                              const std::string& try_code,
                              const std::string& catch_code, int nargout);

    void visit_anon_fcn_handle (tree_anon_fcn_handle&);

    void visit_argument_list (tree_argument_list&);

    void visit_binary_expression (tree_binary_expression&);

    void visit_boolean_expression (tree_boolean_expression&);

    void visit_compound_binary_expression (tree_compound_binary_expression&);

    void visit_break_command (tree_break_command&);

    void visit_colon_expression (tree_colon_expression&);

    void visit_continue_command (tree_continue_command&);

    void visit_decl_command (tree_decl_command&);

    void visit_decl_elt (tree_decl_elt&);

    void visit_simple_for_command (tree_simple_for_command&);

    void visit_complex_for_command (tree_complex_for_command&);

    void visit_octave_user_script (octave_user_script&);

    octave_value_list
    execute_user_script (octave_user_script& user_script, int nargout,
                         const octave_value_list& args);

    void visit_octave_user_function (octave_user_function&);

    octave_value_list
    execute_user_function (octave_user_function& user_function,
                           int nargout, const octave_value_list& args);

    void visit_octave_user_function_header (octave_user_function&);

    void visit_octave_user_function_trailer (octave_user_function&);

    void visit_function_def (tree_function_def&);

    void visit_identifier (tree_identifier&);

    void visit_if_clause (tree_if_clause&);

    void visit_if_command (tree_if_command&);

    void visit_if_command_list (tree_if_command_list&);

    void visit_index_expression (tree_index_expression&);

    void visit_matrix (tree_matrix&);

    void visit_cell (tree_cell&);

    void visit_multi_assignment (tree_multi_assignment&);

    void visit_no_op_command (tree_no_op_command&);

    void visit_constant (tree_constant&);

    void visit_fcn_handle (tree_fcn_handle&);

    void visit_parameter_list (tree_parameter_list&);

    void visit_postfix_expression (tree_postfix_expression&);

    void visit_prefix_expression (tree_prefix_expression&);

    void visit_return_command (tree_return_command&);

    void visit_simple_assignment (tree_simple_assignment&);

    void visit_statement (tree_statement&);

    void visit_statement_list (tree_statement_list&);

    void visit_switch_case (tree_switch_case&);

    void visit_switch_case_list (tree_switch_case_list&);

    void visit_switch_command (tree_switch_command&);

    void visit_try_catch_command (tree_try_catch_command&);

    void do_unwind_protect_cleanup_code (tree_statement_list *list);

    void visit_unwind_protect_command (tree_unwind_protect_command&);

    void visit_while_command (tree_while_command&);
    void visit_do_until_command (tree_do_until_command&);

    void visit_superclass_ref (tree_superclass_ref&);
    void visit_metaclass_query (tree_metaclass_query&);

    void bind_ans (const octave_value& val, bool print);

    bool statement_printing_enabled (void);

    void reset_debug_state (void);

    void reset_debug_state (bool mode);

    void enter_debugger (const std::string& prompt = "debug> ");

    void keyboard (const std::string& prompt = "keyboard> ");

    void dbupdown (int n, bool verbose = false);

    // Possible types of evaluation contexts.
    enum stmt_list_type
    {
      SC_FUNCTION,  // function body
      SC_SCRIPT,    // script file
      SC_OTHER      // command-line input or eval string
    };

    Matrix ignored_fcn_outputs (void) const;

    octave_value make_fcn_handle (const std::string& nm);

    octave_value evaluate (tree_decl_elt *);

    void install_variable (const std::string& name,
                           const octave_value& value, bool global);

    octave_value global_varval (const std::string& name) const;

    octave_value& global_varref (const std::string& name);

    void global_assign (const std::string& name,
                        const octave_value& val = octave_value ());

    octave_value top_level_varval (const std::string& name) const;

    void top_level_assign (const std::string& name,
                           const octave_value& val = octave_value ());

    bool is_variable (const std::string& name) const;

    bool is_local_variable (const std::string& name) const;

    bool is_variable (const tree_expression *expr) const;

    bool is_defined (const tree_expression *expr) const;

    bool is_variable (const symbol_record& sym) const;

    bool is_defined (const symbol_record& sym) const;

    bool is_global (const std::string& name) const;

    octave_value varval (const symbol_record& sym) const;

    octave_value varval (const std::string& name) const;

    void assign (const std::string& name,
                 const octave_value& val = octave_value ());

    void assignin (const std::string& context, const std::string& name,
                   const octave_value& val = octave_value ());

    void source_file (const std::string& file_name,
                      const std::string& context = "",
                      bool verbose = false, bool require_file = true);

    void set_auto_fcn_var (stack_frame::auto_var_type avt,
                           const octave_value& val = octave_value ());

    octave_value get_auto_fcn_var (stack_frame::auto_var_type avt) const;

    void define_parameter_list_from_arg_vector
      (tree_parameter_list *param_list, const octave_value_list& args);

    void undefine_parameter_list (tree_parameter_list *param_list);

    octave_value_list convert_to_const_vector (tree_argument_list *arg_list);

    octave_value_list
    convert_return_list_to_const_vector
      (tree_parameter_list *ret_list, int nargout,
       const Matrix& ignored_outputs, const Cell& varargout);

    bool eval_decl_elt (tree_decl_elt *elt);

    bool switch_case_label_matches (tree_switch_case *expr,
                                    const octave_value& val);

    interpreter& get_interpreter (void) { return m_interpreter; }

    bp_table& get_bp_table (void) { return m_bp_table; }

    profiler& get_profiler (void) { return m_profiler; }

    call_stack& get_call_stack (void) { return m_call_stack; }

    void push_stack_frame (const symbol_scope& scope);

    void push_stack_frame (octave_user_function *fcn,
                           const std::shared_ptr<stack_frame>& closure_frames = std::shared_ptr<stack_frame> ());

    void push_stack_frame (octave_user_function *fcn,
                           const stack_frame::local_vars_map& local_vars);

    void push_stack_frame (octave_user_script *script);

    void push_stack_frame (octave_function *fcn);

    void pop_stack_frame (void);

    std::shared_ptr<stack_frame> get_current_stack_frame (void) const
    {
      return m_call_stack.get_current_stack_frame ();
    }

    std::shared_ptr<stack_frame> current_user_frame (void) const
    {
      return m_call_stack.current_user_frame ();
    }

    // Current line in current function.
    int current_line (void) const;

    // Current column in current function.
    int current_column (void) const;

    // Line number in current function that we are debugging.
    int debug_user_code_line (void) const;

    // Column number in current function that we are debugging.
    int debug_user_code_column (void) const;

    void debug_where (std::ostream& os) const;

    octave_user_code * current_user_code (void) const;

    unwind_protect * curr_fcn_unwind_protect_frame (void);

    // Current function that we are debugging.
    octave_user_code * debug_user_code (void) const;

    octave_function * current_function (bool skip_first = false) const;

    octave_function * caller_function (void) const;

    bool goto_frame (size_t n = 0, bool verbose = false);

    void goto_caller_frame (void);

    void goto_base_frame (void);

    void restore_frame (size_t n);

    std::string get_dispatch_class (void) const;

    void set_dispatch_class (const std::string& class_name);

    bool is_class_method_executing (std::string& dispatch_class) const;

    bool is_class_constructor_executing (std::string& dispatch_class) const;

    std::list<std::shared_ptr<stack_frame>>
    backtrace_frames (octave_idx_type& curr_user_frame) const;

    std::list<std::shared_ptr<stack_frame>> backtrace_frames () const;

    std::list<frame_info> backtrace_info (octave_idx_type& curr_user_frame,
                                          bool print_subfn = true) const;

    std::list<frame_info> backtrace_info (void) const;

    octave_map backtrace (octave_idx_type& curr_user_frame,
                          bool print_subfn = true) const;

    octave_map backtrace (void) const;

    octave_map empty_backtrace (void) const;

    std::string backtrace_message (void) const;

    void push_dummy_scope (const std::string& name);
    void pop_scope (void);

    symbol_scope get_top_scope (void) const;
    symbol_scope get_current_scope (void) const;

    void mlock (bool skip_first = false) const;

    void munlock (bool skip_first = false) const;

    bool mislocked (bool skip_first = false) const;

    octave_value max_stack_depth (const octave_value_list& args, int nargout);

    // Useful for debugging
    void display_call_stack (void) const;

    octave_value find (const std::string& name);

    void clear_objects (void);

    void clear_variable (const std::string& name);

    void clear_variable_pattern (const std::string& pattern);

    void clear_variable_regexp (const std::string& pattern);

    void clear_variables (void);

    void clear_global_variable (const std::string& name);

    void clear_global_variable_pattern (const std::string& pattern);

    void clear_global_variable_regexp (const std::string& pattern);

    void clear_global_variables (void);

    void clear_all (bool force = false);

    void clear_symbol (const std::string& name);

    void clear_symbol_pattern (const std::string& pattern);

    void clear_symbol_regexp (const std::string& pattern);

    std::list<std::string> global_variable_names (void) const;

    std::list<std::string> top_level_variable_names (void) const;

    std::list<std::string> variable_names (void) const;

    octave_user_code * get_user_code (const std::string& fname = "",
                                      const std::string& class_name = "");

    std::string current_function_name (bool skip_first = false) const;

    bool in_user_code (void) const;

    symbol_info_list glob_symbol_info (const std::string& pattern) const;

    symbol_info_list regexp_symbol_info (const std::string& pattern) const;

    symbol_info_list get_symbol_info (void);

    symbol_info_list top_scope_symbol_info (void) const;

    octave_map get_autoload_map (void) const;

    std::string lookup_autoload (const std::string& nm) const;

    std::list<std::string> autoloaded_functions (void) const;

    std::list<std::string> reverse_lookup_autoload (const std::string& nm) const;

    void add_autoload (const std::string& fcn, const std::string& nm);

    void remove_autoload (const std::string& fcn, const std::string& nm);

    int max_recursion_depth (void) const { return m_max_recursion_depth; }

    int max_recursion_depth (int n)
    {
      int val = m_max_recursion_depth;
      m_max_recursion_depth = n;
      return val;
    }

    octave_value
    max_recursion_depth (const octave_value_list& args, int nargout);

    bool silent_functions (void) const { return m_silent_functions; }

    bool silent_functions (bool b)
    {
      int val = m_silent_functions;
      m_silent_functions = b;
      return val;
    }

    octave_value whos_line_format (const octave_value_list& args, int nargout);

    std::string whos_line_format (void) const { return m_whos_line_format; }

    std::string whos_line_format (const std::string& s)
    {
      std::string val = m_whos_line_format;
      m_whos_line_format = s;
      return val;
    }

    octave_value
    silent_functions (const octave_value_list& args, int nargout);

    size_t debug_frame (void) const { return m_debug_frame; }

    size_t debug_frame (size_t n)
    {
      size_t val = m_debug_frame;
      m_debug_frame = n;
      return val;
    }

    size_t current_call_stack_frame_number (void) const
    {
      return m_call_stack.current_frame ();
    }

    bool quiet_breakpoint_flag (void) const { return m_quiet_breakpoint_flag; }

    bool quiet_breakpoint_flag (bool flag)
    {
      bool val = m_quiet_breakpoint_flag;
      m_quiet_breakpoint_flag = flag;
      return val;
    }

    char string_fill_char (void) const { return m_string_fill_char; }

    char string_fill_char (char c)
    {
      int val = m_string_fill_char;
      m_string_fill_char = c;
      return val;
    }

    // The following functions are provided for convenience.  They
    // call the corresponding functions in the debugger class for the
    // current debugger (if any).

    bool in_debug_repl (void) const;

    void dbcont (void);

    void dbquit (bool all = false);

    octave_value PS4 (const octave_value_list& args, int nargout);

    std::string PS4 (void) const { return m_PS4; }

    std::string PS4 (const std::string& s)
    {
      std::string val = m_PS4;
      m_PS4 = s;
      return val;
    }

    octave_value indexed_object (void) const
    {
      return m_indexed_object;
    }

    void set_indexed_object (const octave_value& obj = octave_value ())
    {
      m_indexed_object = obj;
    }

    const std::list<octave_value_list>& index_list (void) const
    {
      return m_index_list;
    }

    void set_index_list (const std::string& index_type,
                         const std::list<octave_value_list>& index_list)
    {
      m_index_type = index_type;
      m_index_list = index_list;
    }

    void clear_index_list (void)
    {
      m_index_type = "";
      m_index_list.clear ();
    }

    void append_index_list (char type, const octave_value_list& idx)
    {
      m_index_type += type;
      m_index_list.push_back (idx);
    }

    const std::string& index_type (void) const
    {
      return m_index_type;
    }

    int index_position (void) const { return m_index_position; }

    int num_indices (void) const { return m_num_indices; }

    octave_value_list evaluate_end_expression (const octave_value_list& args);

    const std::list<octave_lvalue> * lvalue_list (void) const
    {
      return m_lvalue_list;
    }

    void set_lvalue_list (const std::list<octave_lvalue> *lst)
    {
      m_lvalue_list = lst;
    }

    int breaking (void) const { return m_breaking; }

    int breaking (int n)
    {
      int val = m_breaking;
      m_breaking = n;
      return val;
    }

    int continuing (void) const { return m_continuing; }

    int continuing (int n)
    {
      int val = m_continuing;
      m_continuing = n;
      return val;
    }

    int returning (void) const { return m_returning; }

    int returning (int n)
    {
      int val = m_returning;
      m_returning = n;
      return val;
    }

    int dbstep_flag (void) const { return m_dbstep_flag; }

    int dbstep_flag (int val)
    {
      int old_val = m_dbstep_flag;
      m_dbstep_flag = val;
      return old_val;
    }

    void set_dbstep_flag (int step) { m_dbstep_flag = step; }

    octave_value echo (const octave_value_list& args, int nargout);

    int echo (void) const { return m_echo; }

    int echo (int val)
    {
      int old_val = m_echo;
      m_echo = val;
      return old_val;
    }

    octave_value
    string_fill_char (const octave_value_list& args, int nargout);

    void final_index_error (index_exception& e, const tree_expression *expr);

    octave_value do_who (int argc, const string_vector& argv,
                         bool return_list, bool verbose = false);

    octave_value_list
    make_value_list (tree_argument_list *args, const string_vector& arg_nm);

    std::list<octave_lvalue> make_lvalue_list (tree_argument_list *);

    void push_echo_state (int type, const std::string& file_name,
                          size_t pos = 1);

  private:

    void set_echo_state (int type, const std::string& file_name, size_t pos);

    void maybe_set_echo_state (void);

    void push_echo_state_cleanup (unwind_protect& frame);

    bool maybe_push_echo_state_cleanup (void);

    void do_breakpoint (tree_statement& stmt);

    void do_breakpoint (bool is_breakpoint,
                        bool is_end_of_fcn_or_script = false);

    bool is_logically_true (tree_expression *expr, const char *warn_for);

    // For unwind-protect.
    void uwp_set_echo_state (bool state, const std::string& file_name,
                             size_t pos);

    bool echo_this_file (const std::string& file, int type) const;

    void echo_code (size_t line);

    bool quit_loop_now (void);

    void bind_auto_fcn_vars (const string_vector& arg_names,
                             const Matrix& ignored_outputs, int nargin,
                             int nargout, bool takes_varargs,
                             const octave_value_list& va_args);

    std::string check_autoload_file (const std::string& nm) const;

    interpreter& m_interpreter;

    // The context for the current evaluation.
    stmt_list_type m_statement_context;

    const std::list<octave_lvalue> *m_lvalue_list;

    // List of autoloads (function -> file mapping).
    std::map<std::string, std::string> m_autoload_map;

    bp_table m_bp_table;

    call_stack m_call_stack;

    profiler m_profiler;

    // The number of the stack frame we are currently debugging.
    size_t m_debug_frame;

    bool m_debug_mode;

    bool m_quiet_breakpoint_flag;

    // When entering the debugger we push it on this stack.  Managing
    // debugger invocations this way allows us to handle recursive
    // debugger calls.  When we exit a debugger the object is popped
    // from the stack and deleted and we resume working with the
    // previous debugger (if any) that is now at the top of the stack.
    std::stack<debugger *> m_debugger_stack;

    // Maximum nesting level for functions, scripts, or sourced files
    // called recursively.
    int m_max_recursion_depth;

    // Defines layout for the whos/who -long command
    std::string m_whos_line_format;

    // If TRUE, turn off printing of results in functions (as if a
    // semicolon has been appended to each statement).
    bool m_silent_functions;

    // The character to fill with when creating string arrays.
    char m_string_fill_char;

    // String printed before echoed commands (enabled by --echo-commands).
    std::string m_PS4;

    // If > 0, stop executing at the (N-1)th stopping point, counting
    //         from the the current execution point in the current frame.
    //
    // If < 0, stop executing at the next possible stopping point.
    int m_dbstep_flag;

    // Echo commands as they are executed?
    //
    //   1  ==>  echo commands read from script files
    //   2  ==>  echo commands from functions
    //
    // more than one state can be active at once.
    int m_echo;

    // Are we currently echoing commands?  This state is set by the
    // functions that execute functions and scripts.
    bool m_echo_state;

    std::string m_echo_file_name;

    // Next line to echo, counting from 1.
    size_t m_echo_file_pos;

    std::map<std::string, bool> m_echo_files;

    // TRUE means we are evaluating some kind of looping construct.
    bool m_in_loop_command;

    // Nonzero means we're breaking out of a loop or function body.
    int m_breaking;

    // Nonzero means we're jumping to the end of a loop.
    int m_continuing;

    // Nonzero means we're returning from a function.
    int m_returning;

    // The following are all used by the END function.  Maybe they
    // should be kept together in a separate object?
    octave_value m_indexed_object;
    std::list<octave_value_list> m_index_list;
    std::string m_index_type;
    int m_index_position;
    int m_num_indices;
  };
}

#endif
