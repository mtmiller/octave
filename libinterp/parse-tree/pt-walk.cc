////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2021 The Octave Project Developers
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

#include "pt-all.h"

namespace octave
{
  void tree_walker::visit_anon_fcn_handle (tree_anon_fcn_handle&)
  {
    // FIXME?
  }

  void tree_walker::visit_argument_list (tree_argument_list& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_expression *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_binary_expression (tree_binary_expression& expr)
  {
    tree_expression *op1 = expr.lhs ();

    if (op1)
      op1->accept (*this);

    tree_expression *op2 = expr.rhs ();

    if (op2)
      op2->accept (*this);
  }

  void tree_walker::visit_boolean_expression (tree_boolean_expression& expr)
  {
    visit_binary_expression (expr);
  }

  void tree_walker::visit_compound_binary_expression (tree_compound_binary_expression& expr)
  {
    visit_binary_expression (expr);
  }

  void tree_walker::visit_break_command (tree_break_command&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_colon_expression (tree_colon_expression& expr)
  {
    tree_expression *op1 = expr.base ();

    if (op1)
      op1->accept (*this);

    tree_expression *op3 = expr.increment ();

    if (op3)
      op3->accept (*this);

    tree_expression *op2 = expr.limit ();

    if (op2)
      op2->accept (*this);
  }

  void tree_walker::visit_continue_command (tree_continue_command&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_decl_command (tree_decl_command& cmd)
  {
    tree_decl_init_list *init_list = cmd.initializer_list ();

    if (init_list)
      init_list->accept (*this);
  }

  void tree_walker::visit_decl_elt (tree_decl_elt& cmd)
  {
    tree_identifier *id = cmd.ident ();

    if (id)
      id->accept (*this);

    tree_expression *expr = cmd.expression ();

    if (expr)
      expr->accept (*this);
  }

  void tree_walker::visit_decl_init_list (tree_decl_init_list& lst)
  {
    // FIXME: tree_decl_elt is not derived from tree, so should it
    // really have an accept method?

    for (tree_decl_elt *elt : lst)
      {
        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_simple_for_command (tree_simple_for_command& cmd)
  {
    tree_expression *lhs = cmd.left_hand_side ();

    if (lhs)
      lhs->accept (*this);

    tree_expression *expr = cmd.control_expr ();

    if (expr)
      expr->accept (*this);

    tree_expression *maxproc = cmd.maxproc_expr ();

    if (maxproc)
      maxproc->accept (*this);

    tree_statement_list *list = cmd.body ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_complex_for_command (tree_complex_for_command& cmd)
  {
    tree_argument_list *lhs = cmd.left_hand_side ();

    if (lhs)
      lhs->accept (*this);

    tree_expression *expr = cmd.control_expr ();

    if (expr)
      expr->accept (*this);

    tree_statement_list *list = cmd.body ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_octave_user_script (octave_user_script& fcn)
  {
    tree_statement_list *cmd_list = fcn.body ();

    if (cmd_list)
      cmd_list->accept (*this);
  }

  void tree_walker::visit_octave_user_function (octave_user_function& fcn)
  {
    tree_statement_list *cmd_list = fcn.body ();

    if (cmd_list)
      cmd_list->accept (*this);
  }

  void tree_walker::visit_function_def (tree_function_def& fdef)
  {
    octave_value fcn = fdef.function ();

    octave_function *f = fcn.function_value ();

    if (f)
      f->accept (*this);
  }

  void tree_walker::visit_identifier (tree_identifier&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_if_clause (tree_if_clause& cmd)
  {
    tree_expression *expr = cmd.condition ();

    if (expr)
      expr->accept (*this);

    tree_statement_list *list = cmd.commands ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_if_command (tree_if_command& cmd)
  {
    tree_if_command_list *list = cmd.cmd_list ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_if_command_list (tree_if_command_list& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_if_clause *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_switch_case (tree_switch_case& cs)
  {
    tree_expression *label = cs.case_label ();

    if (label)
      label->accept (*this);

    tree_statement_list *list = cs.commands ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_switch_case_list (tree_switch_case_list& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_switch_case *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_switch_command (tree_switch_command& cmd)
  {
    tree_expression *expr = cmd.switch_value ();

    if (expr)
      expr->accept (*this);

    tree_switch_case_list *list = cmd.case_list ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_index_expression (tree_index_expression& expr)
  {
    tree_expression *e = expr.expression ();

    if (e)
      e->accept (*this);

    std::list<tree_argument_list *> arg_lists = expr.arg_lists ();
    std::list<string_vector> arg_names = expr.arg_names ();
    std::list<tree_expression *> dyn_fields = expr.dyn_fields ();

    auto p_arg_lists = arg_lists.begin ();
    auto p_arg_names = arg_names.begin ();
    auto p_dyn_fields = dyn_fields.begin ();

    std::string type_tags = expr.type_tags ();
    int n = type_tags.length ();

    for (int i = 0; i < n; i++)
      {
        switch (type_tags[i])
          {
          case '(':
          case '{':
            {
              tree_argument_list *l = *p_arg_lists;
              if (l)
                l->accept (*this);
            }
            break;

          case '.':
            {
              std::string fn = (*p_arg_names)(0);
              if (fn.empty ())
                {
                  tree_expression *df = *p_dyn_fields;
                  if (df)
                    df->accept (*this);
                }
            }
            break;

          default:
            panic_impossible ();
          }

        p_arg_lists++;
        p_arg_names++;
        p_dyn_fields++;
      }
  }

  void tree_walker::visit_matrix (tree_matrix& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_argument_list *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_cell (tree_cell& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_argument_list *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_multi_assignment (tree_multi_assignment& expr)
  {
    tree_argument_list *lhs = expr.left_hand_side ();

    if (lhs)
      lhs->accept (*this);

    tree_expression *rhs = expr.right_hand_side ();

    if (rhs)
      rhs->accept (*this);
  }

  void tree_walker::visit_no_op_command (tree_no_op_command&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_constant (tree_constant&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_fcn_handle (tree_fcn_handle&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_parameter_list (tree_parameter_list& lst)
  {
    auto p = lst.begin ();

    while (p != lst.end ())
      {
        tree_decl_elt *elt = *p++;

        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_postfix_expression (tree_postfix_expression& expr)
  {
    tree_expression *e = expr.operand ();

    if (e)
      e->accept (*this);
  }

  void tree_walker::visit_prefix_expression (tree_prefix_expression& expr)
  {
    tree_expression *e = expr.operand ();

    if (e)
      e->accept (*this);
  }

  void tree_walker::visit_return_command (tree_return_command&)
  {
    // Nothing to do.
  }

  void tree_walker::visit_simple_assignment (tree_simple_assignment& expr)
  {
    tree_expression *lhs = expr.left_hand_side ();

    if (lhs)
      lhs->accept (*this);

    tree_expression *rhs = expr.right_hand_side ();

    if (rhs)
      rhs->accept (*this);
  }

  void tree_walker::visit_statement (tree_statement& stmt)
  {
    tree_command *cmd = stmt.command ();

    if (cmd)
      cmd->accept (*this);
    else
      {
        tree_expression *expr = stmt.expression ();

        if (expr)
          expr->accept (*this);
      }
  }

  void tree_walker::visit_statement_list (tree_statement_list& lst)
  {
    for (tree_statement *elt : lst)
      {
        if (elt)
          elt->accept (*this);
      }
  }

  void tree_walker::visit_try_catch_command (tree_try_catch_command& cmd)
  {
    tree_statement_list *try_code = cmd.body ();

    if (try_code)
      try_code->accept (*this);

    tree_identifier *expr_id = cmd.identifier ();

    if (expr_id)
      expr_id->accept (*this);

    tree_statement_list *catch_code = cmd.cleanup ();

    if (catch_code)
      catch_code->accept (*this);
  }

  void tree_walker::visit_unwind_protect_command (tree_unwind_protect_command& cmd)
  {
    tree_statement_list *unwind_protect_code = cmd.body ();

    if (unwind_protect_code)
      unwind_protect_code->accept (*this);

    tree_statement_list *cleanup_code = cmd.cleanup ();

    if (cleanup_code)
      cleanup_code->accept (*this);
  }

  void tree_walker::visit_while_command (tree_while_command& cmd)
  {
    tree_expression *expr = cmd.condition ();

    if (expr)
      expr->accept (*this);

    tree_statement_list *list = cmd.body ();

    if (list)
      list->accept (*this);
  }

  void tree_walker::visit_do_until_command (tree_do_until_command& cmd)
  {
    tree_statement_list *list = cmd.body ();

    if (list)
      list->accept (*this);

    tree_expression *expr = cmd.condition ();

    if (expr)
      expr->accept (*this);
  }

  void tree_walker::visit_superclass_ref (tree_superclass_ref&)
  {
    // FIXME?
  }

  void tree_walker::visit_metaclass_query (tree_metaclass_query&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_attribute (tree_classdef_attribute&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_attribute_list (tree_classdef_attribute_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_superclass (tree_classdef_superclass&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_superclass_list (tree_classdef_superclass_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_property (tree_classdef_property&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_property_list (tree_classdef_property_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_properties_block (tree_classdef_properties_block&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_methods_list (tree_classdef_methods_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_methods_block (tree_classdef_methods_block&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_event (tree_classdef_event&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_events_list (tree_classdef_events_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_events_block (tree_classdef_events_block&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_enum (tree_classdef_enum&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_enum_list (tree_classdef_enum_list&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_enum_block (tree_classdef_enum_block&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef_body (tree_classdef_body&)
  {
    // FIXME?
  }

  void tree_walker::visit_classdef (tree_classdef&)
  {
    // FIXME?
  }
}
