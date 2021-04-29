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

#include "jit-ir.h"

#if defined (HAVE_LLVM_IR_FUNCTION_H)
#  include <llvm/IR/BasicBlock.h>
#  include <llvm/IR/Instructions.h>
#else
#  include <llvm/BasicBlock.h>
#  include <llvm/Instructions.h>
#endif

#include "error.h"

namespace octave
{

  // -------------------- jit_factory --------------------
  jit_factory::~jit_factory (void)
  {
    for (auto iter = m_all_values.begin ();
         iter != m_all_values.end (); ++iter)
      delete *iter;
  }

  void
  jit_factory::track_value (jit_value *value)
  {
    if (value->type ())
      m_constants.push_back (value);
    m_all_values.push_back (value);
  }

  // -------------------- jit_block_list --------------------
  void
  jit_block_list::insert_after (iterator iter, jit_block *ablock)
  {
    ++iter;
    insert_before (iter, ablock);
  }

  void
  jit_block_list::insert_after (jit_block *loc, jit_block *ablock)
  {
    insert_after (loc->location (), ablock);
  }

  void
  jit_block_list::insert_before (iterator iter, jit_block *ablock)
  {
    iter = m_list.insert (iter, ablock);
    ablock->stash_location (iter);
  }

  void
  jit_block_list::insert_before (jit_block *loc, jit_block *ablock)
  {
    insert_before (loc->location (), ablock);
  }

  void
  jit_block_list::label (void)
  {
    if (m_list.size ())
      {
        jit_block *block = m_list.back ();
        block->label ();
      }
  }

  std::ostream&
  jit_block_list::print (std::ostream& os, const std::string& header) const
  {
    os << "-------------------- " << header << " --------------------\n";
    return os << *this;
  }

  std::ostream&
  jit_block_list::print_dom (std::ostream& os) const
  {
    os << "-------------------- dom info --------------------\n";
    for (auto iter = begin (); iter != end (); ++iter)
      {
        assert (*iter);
        (*iter)->print_dom (os);
      }
    os << std::endl;

    return os;
  }

  void
  jit_block_list::push_back (jit_block *b)
  {
    m_list.push_back (b);
    auto iter = m_list.end ();
    b->stash_location (--iter);
  }

  std::ostream&
  operator<<(std::ostream& os, const jit_block_list& blocks)
  {
    for (auto iter = blocks.begin (); iter != blocks.end (); ++iter)
      {
        assert (*iter);
        (*iter)->print (os, 0);
      }
    return os << std::endl;
  }

  // -------------------- jit_use --------------------
  jit_block *
  jit_use::user_parent (void) const
  {
    return m_user->parent ();
  }

  // -------------------- jit_value --------------------
  jit_value::~jit_value (void)
  { }

  jit_block *
  jit_value::first_use_block (void)
  {
    jit_use *use = first_use ();
    while (use)
      {
        if (! isa<jit_error_check> (use->user ()))
          return use->user_parent ();

        use = use->next ();
      }

    return 0;
  }

  void
  jit_value::replace_with (jit_value *value)
  {
    while (first_use ())
      {
        jit_instruction *user = first_use ()->user ();
        std::size_t idx = first_use ()->index ();
        user->stash_argument (idx, value);
      }
  }

#define JIT_METH(clname)                                \
  void                                                  \
  jit_ ## clname::accept (jit_ir_walker& walker)        \
  {                                                     \
    walker.visit (*this);                               \
  }

  JIT_VISIT_IR_NOTEMPLATE
#undef JIT_METH

  std::ostream&
  operator<< (std::ostream& os, const jit_value& value)
  {
    return value.short_print (os);
  }

  std::ostream&
  jit_print (std::ostream& os, jit_value *avalue)
  {
    if (avalue)
      return avalue->print (os);
    return os << "NULL";
  }

  // -------------------- jit_instruction --------------------
  void
  jit_instruction::remove (void)
  {
    if (m_parent)
      m_parent->remove (m_location);
    resize_arguments (0);
  }

  llvm::BasicBlock *
  jit_instruction::parent_llvm (void) const
  {
    return m_parent->to_llvm ();
  }

  std::ostream&
  jit_instruction::short_print (std::ostream& os) const
  {
    if (type ())
      jit_print (os, type ()) << ": ";
    return os << '#' << m_id;
  }

  void
  jit_instruction::do_construct_ssa (std::size_t start, std::size_t end)
  {
    for (std::size_t i = start; i < end; ++i)
      {
        jit_value *arg = argument (i);
        jit_variable *var = dynamic_cast<jit_variable *> (arg);
        if (var && var->has_top ())
          stash_argument (i, var->top ());
      }
  }

  // -------------------- jit_block --------------------
  void
  jit_block::replace_with (jit_value *value)
  {
    assert (isa<jit_block> (value));
    jit_block *block = static_cast<jit_block *> (value);

    jit_value::replace_with (block);

    while (ILIST_T::first_use ())
      {
        jit_phi_incoming *incoming = ILIST_T::first_use ();
        incoming->stash_value (block);
      }
  }

  void
  jit_block::replace_in_phi (jit_block *ablock, jit_block *with)
  {
    jit_phi_incoming *node = ILIST_T::first_use ();
    while (node)
      {
        jit_phi_incoming *prev = node;
        node = node->next ();

        if (prev->user_parent () == ablock)
          prev->stash_value (with);
      }
  }

  jit_block *
  jit_block::maybe_merge (void)
  {
    if (successor_count () == 1 && successor (0) != this
        && (successor (0)->use_count () == 1 || m_instructions.size () == 1))
      {
        jit_block *to_merge = successor (0);
        merge (*to_merge);
        return to_merge;
      }

    return 0;
  }

  void
  jit_block::merge (jit_block& block)
  {
    // the merge block will contain a new terminator
    jit_terminator *old_term = terminator ();
    if (old_term)
      old_term->remove ();

    bool was_empty = end () == begin ();
    auto merge_begin = end ();
    if (! was_empty)
      --merge_begin;

    m_instructions.splice (end (), block.m_instructions);
    if (was_empty)
      merge_begin = begin ();
    else
      ++merge_begin;

    // now merge_begin points to the start of the new instructions, we must
    // update their parent information
    for (auto iter = merge_begin; iter != end (); ++iter)
      {
        jit_instruction *instr = *iter;
        instr->stash_parent (this, iter);
      }

    block.replace_with (this);
  }

  jit_instruction *
  jit_block::prepend (jit_instruction *instr)
  {
    m_instructions.push_front (instr);
    instr->stash_parent (this, m_instructions.begin ());
    return instr;
  }

  jit_instruction *
  jit_block::prepend_after_phi (jit_instruction *instr)
  {
    // FIXME: Make this O(1)
    for (auto iter = begin (); iter != end (); ++iter)
      {
        jit_instruction *temp = *iter;
        if (! isa<jit_phi> (temp))
          {
            insert_before (iter, instr);
            return instr;
          }
      }

    return append (instr);
  }

  void
  jit_block::internal_append (jit_instruction *instr)
  {
    m_instructions.push_back (instr);
    instr->stash_parent (this, --m_instructions.end ());
  }

  jit_instruction *
  jit_block::insert_before (iterator loc, jit_instruction *instr)
  {
    auto iloc = m_instructions.insert (loc, instr);
    instr->stash_parent (this, iloc);
    return instr;
  }

  jit_instruction *
  jit_block::insert_after (iterator loc, jit_instruction *instr)
  {
    ++loc;
    auto iloc = m_instructions.insert (loc, instr);
    instr->stash_parent (this, iloc);
    return instr;
  }

  jit_terminator *
  jit_block::terminator (void) const
  {
    if (m_instructions.empty ())
      return nullptr;

    jit_instruction *last = m_instructions.back ();
    return dynamic_cast<jit_terminator *> (last);
  }

  bool
  jit_block::branch_alive (jit_block *asucc) const
  {
    return terminator ()->alive (asucc);
  }

  jit_block *
  jit_block::successor (std::size_t i) const
  {
    jit_terminator *term = terminator ();
    return term->successor (i);
  }

  std::size_t
  jit_block::successor_count (void) const
  {
    jit_terminator *term = terminator ();
    return term ? term->successor_count () : 0;
  }

  llvm::BasicBlock *
  jit_block::to_llvm (void) const
  {
    return llvm::cast<llvm::BasicBlock> (m_llvm_value);
  }

  std::ostream&
  jit_block::print_dom (std::ostream& os) const
  {
    short_print (os);
    os << ":\n";
    os << "  m_id: " << m_id << std::endl;
    os << "  predecessors: ";
    for (jit_use *use = first_use (); use; use = use->next ())
      os << *use->user_parent () << ' ';
    os << std::endl;

    os << "  successors: ";
    for (std::size_t i = 0; i < successor_count (); ++i)
      os << *successor (i) << ' ';
    os << std::endl;

    os << "  m_idom: ";
    if (m_idom)
      os << *m_idom;
    else
      os << "NULL";
    os << std::endl;
    os << "  df: ";
    for (auto iter = df_begin (); iter != df_end (); ++iter)
      os << **iter << ' ';
    os << std::endl;

    os << "  m_dom_succ: ";
    for (std::size_t i = 0; i < m_dom_succ.size (); ++i)
      os << *m_dom_succ[i] << ' ';

    return os << std::endl;
  }

  void
  jit_block::compute_df (std::size_t avisit_count)
  {
    if (visited (avisit_count))
      return;

    if (use_count () >= 2)
      {
        for (jit_use *use = first_use (); use; use = use->next ())
          {
            jit_block *runner = use->user_parent ();
            while (runner != m_idom)
              {
                runner->m_df.insert (this);
                runner = runner->m_idom;
              }
          }
      }

    for (std::size_t i = 0; i < successor_count (); ++i)
      successor (i)->compute_df (avisit_count);
  }

  bool
  jit_block::update_idom (std::size_t avisit_count)
  {
    if (visited (avisit_count) || ! use_count ())
      return false;

    bool changed = false;
    for (jit_use *use = first_use (); use; use = use->next ())
      {
        jit_block *pred = use->user_parent ();
        changed = pred->update_idom (avisit_count) || changed;
      }

    jit_use *use = first_use ();
    jit_block *new_idom = use->user_parent ();
    use = use->next ();

    for (; use; use = use->next ())
      {
        jit_block *pred = use->user_parent ();
        jit_block *pidom = pred->m_idom;
        if (pidom)
          new_idom = idom_intersect (pidom, new_idom);
      }

    if (m_idom != new_idom)
      {
        m_idom = new_idom;
        return true;
      }

    return changed;
  }

  void
  jit_block::label (std::size_t avisit_count, std::size_t& number)
  {
    if (visited (avisit_count))
      return;

    for (jit_use *use = first_use (); use; use = use->next ())
      {
        jit_block *pred = use->user_parent ();
        pred->label (avisit_count, number);
      }

    m_id = number++;
  }

  void
  jit_block::pop_all (void)
  {
    for (auto iter = begin (); iter != end (); ++iter)
      {
        jit_instruction *instr = *iter;
        instr->pop_variable ();
      }
  }

  std::ostream&
  jit_block::print (std::ostream& os, std::size_t indent) const
  {
    print_indent (os, indent);
    short_print (os) << ":        %pred = ";
    for (jit_use *use = first_use (); use; use = use->next ())
      {
        jit_block *pred = use->user_parent ();
        os << *pred;
        if (use->next ())
          os << ", ";
      }
    os << std::endl;

    for (auto iter = begin (); iter != end (); ++iter)
      {
        jit_instruction *instr = *iter;
        instr->print (os, indent + 1) << std::endl;
      }
    return os;
  }

  jit_block *
  jit_block::maybe_split (jit_factory& factory, jit_block_list& blocks,
                          jit_block *asuccessor)
  {
    if (successor_count () > 1)
      {
        jit_terminator *term = terminator ();
        std::size_t idx = term->successor_index (asuccessor);
        jit_block *split = factory.create<jit_block> ("phi_split", m_visit_count);

        // place after this to ensure define before use in the blocks list
        blocks.insert_after (this, split);

        term->stash_argument (idx, split);
        jit_branch *br = split->append (factory.create<jit_branch> (asuccessor));
        replace_in_phi (asuccessor, split);

        if (alive ())
          {
            split->mark_alive ();
            br->infer ();
          }

        return split;
      }

    return this;
  }

  void
  jit_block::create_dom_tree (std::size_t avisit_count)
  {
    if (visited (avisit_count))
      return;

    if (m_idom != this)
      m_idom->m_dom_succ.push_back (this);

    for (std::size_t i = 0; i < successor_count (); ++i)
      successor (i)->create_dom_tree (avisit_count);
  }

  jit_block *
  jit_block::idom_intersect (jit_block *i, jit_block *j)
  {
    while (i && j && i != j)
      {
        while (i && i->id () > j->id ())
          i = i->m_idom;

        while (i && j && j->id () > i->id ())
          j = j->m_idom;
      }

    return i ? i : j;
  }

  // -------------------- jit_phi_incoming --------------------

  jit_block *
  jit_phi_incoming::user_parent (void) const
  { return m_user->parent (); }

  // -------------------- jit_phi --------------------
  bool
  jit_phi::prune (void)
  {
    jit_block *p = parent ();
    std::size_t new_idx = 0;
    jit_value *unique = argument (1);

    for (std::size_t i = 0; i < argument_count (); ++i)
      {
        jit_block *inc = incoming (i);
        if (inc->branch_alive (p))
          {
            if (unique != argument (i))
              unique = 0;

            if (new_idx != i)
              {
                stash_argument (new_idx, argument (i));
                m_incoming[new_idx].stash_value (inc);
              }

            ++new_idx;
          }
      }

    if (new_idx != argument_count ())
      {
        resize_arguments (new_idx);
        m_incoming.resize (new_idx);
      }

    assert (argument_count () > 0);
    if (unique)
      {
        replace_with (unique);
        return true;
      }

    return false;
  }

  bool
  jit_phi::infer (void)
  {
    jit_block *p = parent ();
    if (! p->alive ())
      return false;

    jit_type *infered = nullptr;
    for (std::size_t i = 0; i < argument_count (); ++i)
      {
        jit_block *inc = incoming (i);
        if (inc->branch_alive (p))
          infered = jit_type_join (infered, argument_type (i));
      }

    if (infered != type ())
      {
        stash_type (infered);
        return true;
      }

    return false;
  }

  llvm::PHINode *
  jit_phi::to_llvm (void) const
  {
    return llvm::cast<llvm::PHINode> (jit_value::to_llvm ());
  }

  // -------------------- jit_terminator --------------------
  std::size_t
  jit_terminator::successor_index (const jit_block *asuccessor) const
  {
    std::size_t scount = successor_count ();
    for (std::size_t i = 0; i < scount; ++i)
      if (successor (i) == asuccessor)
        return i;

    panic_impossible ();
  }

  bool
  jit_terminator::infer (void)
  {
    if (! parent ()->alive ())
      return false;

    bool changed = false;
    for (std::size_t i = 0; i < m_alive.size (); ++i)
      if (! m_alive[i] && check_alive (i))
        {
          changed = true;
          m_alive[i] = true;
          successor (i)->mark_alive ();
        }

    return changed;
  }

  llvm::TerminatorInst *
  jit_terminator::to_llvm (void) const
  {
    return llvm::cast<llvm::TerminatorInst> (jit_value::to_llvm ());
  }

  // -------------------- jit_call --------------------
  bool
  jit_call::needs_release (void) const
  {
    if (type () && jit_typeinfo::get_release (type ()).valid ())
      {
        for (jit_use *use = first_use (); use; use = use->next ())
          {
            jit_assign *assign = dynamic_cast<jit_assign *> (use->user ());
            if (assign && assign->artificial ())
              return false;
          }

        return true;
      }
    return false;
  }

  bool
  jit_call::infer (void)
  {
    // FIXME: explain algorithm
    for (std::size_t i = 0; i < argument_count (); ++i)
      {
        m_already_infered[i] = argument_type (i);
        if (! m_already_infered[i])
          return false;
      }

    jit_type *infered = m_operation.result (m_already_infered);
    if (! infered && use_count ())
      {
        std::stringstream ss;
        ss << "Missing overload in type inference for ";
        print (ss, 0);
        throw jit_fail_exception (ss.str ());
      }

    if (infered != type ())
      {
        stash_type (infered);
        return true;
      }

    return false;
  }

  // -------------------- jit_error_check --------------------
  std::string
  jit_error_check::variable_to_string (variable v)
  {
    switch (v)
      {
      case var_error_state:
        return "error_state";
      case var_interrupt:
        return "interrupt";
      default:
        panic_impossible ();
      }
  }

  std::ostream&
  jit_error_check::print (std::ostream& os, std::size_t indent) const
  {
    print_indent (os, indent) << "error_check " << variable_to_string (m_variable)
                              << ", ";

    if (has_check_for ())
      os << "<for> " << *check_for () << ", ";
    print_successor (os << "<normal> ", 1) << ", ";
    return print_successor (os << "<error> ", 0);
  }

  // -------------------- jit_magic_end --------------------
  jit_magic_end::context::context (jit_factory& factory, jit_value *avalue,
                                   std::size_t aindex, std::size_t acount)
    : m_value (avalue), m_index (factory.create<jit_const_index> (aindex)),
      m_count (factory.create<jit_const_index> (acount))
  { }

  jit_magic_end::jit_magic_end (const std::vector<context>& full_context)
    : m_contexts (full_context)
  {
    resize_arguments (m_contexts.size ());

    std::size_t i;
    std::vector<context>::const_iterator iter;
    for (iter = m_contexts.begin (), i = 0; iter != m_contexts.end (); ++iter, ++i)
      stash_argument (i, iter->m_value);
  }

  jit_magic_end::context
  jit_magic_end::resolve_context (void) const
  {
    std::size_t idx;
    for (idx = 0; idx < m_contexts.size (); ++idx)
      {
        jit_type *ctx_type = m_contexts[idx].m_value->type ();
        if (! ctx_type || ctx_type->skip_paren ())
          break;
      }

    if (idx >= m_contexts.size ())
      idx = 0;

    context ret = m_contexts[idx];
    ret.m_value = argument (idx);
    return ret;
  }

  bool
  jit_magic_end::infer (void)
  {
    jit_type *new_type = overload ().result ();
    if (new_type != type ())
      {
        stash_type (new_type);
        return true;
      }

    return false;
  }

  std::ostream&
  jit_magic_end::print (std::ostream& os, std::size_t indent) const
  {
    context ctx = resolve_context ();
    short_print (print_indent (os, indent)) << " (" << *ctx.m_value << ", ";
    return os << *ctx.m_index << ", " << *ctx.m_count << ')';
  }

  const jit_function&
  jit_magic_end::overload (void) const
  {
    const context& ctx = resolve_context ();
    return jit_typeinfo::end (ctx.m_value, ctx.m_index, ctx.m_count);
  }

}

#endif
