/*

Copyright (C) 1993-2017 John W. Eaton
Copyright (C) 2009 VZLU Prague

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

#if ! defined (octave_symrec_h)
#define octave_symrec_h 1

#include "octave-config.h"

#include <deque>
#include <list>
#include <memory>
#include <string>

class octave_user_function;

#include "ov.h"
#include "ovl.h"

namespace octave
{
  class symbol_scope_rep;

  class symbol_record
  {
  public:

    typedef size_t context_id;

    // generic variable
    static const unsigned int local = 1;

    // varargin, argn, .nargin., .nargout.
    // (FIXME: is this really used now?)
    static const unsigned int automatic = 2;

    // formal parameter
    static const unsigned int formal = 4;

    // not listed or cleared (.nargin., .nargout.)
    static const unsigned int hidden = 8;

    // inherited from parent scope; not cleared at function exit
    static const unsigned int inherited = 16;

    // global (redirects to global scope)
    static const unsigned int global = 32;

    // not cleared at function exit
    static const unsigned int persistent = 64;

    // this symbol may NOT become a variable.
    // (symbol added to a static workspace)
    static const unsigned int added_static = 128;

  private:

    class symbol_record_rep
    {
    public:

      symbol_record_rep (const std::string& nm, const octave_value& v,
                         unsigned int sc)
        : m_storage_class (sc), m_name (nm), m_fwd_scope (nullptr),
          m_fwd_rep (), m_value_stack (), m_valid (true)
      {
        m_value_stack.push_back (v);
      }

      // No copying!

      symbol_record_rep (const symbol_record_rep& ov) = delete;

      symbol_record_rep& operator = (const symbol_record_rep&) = delete;

      ~symbol_record_rep (void) = default;

      void assign (const octave_value& value, context_id context)
      {
        varref(context) = value;
      }

      void assign (octave_value::assign_op op,
                   const std::string& type,
                   const std::list<octave_value_list>& idx,
                   const octave_value& value, context_id context)
      {
        varref(context).assign (op, type, idx, value);
      }

      void assign (octave_value::assign_op op, const octave_value& value,
                   context_id context)
      {
        varref(context).assign (op, value);
      }

      void do_non_const_unary_op (octave_value::unary_op op,
                                  context_id context)
      {
        varref(context).do_non_const_unary_op (op);
      }

      void do_non_const_unary_op (octave_value::unary_op op,
                                  const std::string& type,
                                  const std::list<octave_value_list>& idx,
                                  context_id context)
      {
        varref(context).do_non_const_unary_op (op, type, idx);
      }

      context_id get_fwd_scope_context (void) const;

      octave_value& varref (context_id context)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->varref (get_fwd_scope_context ());

        if (is_persistent ())
          context = 0;

        context_id n = m_value_stack.size ();
        while (n++ <= context)
          m_value_stack.push_back (octave_value ());

        return m_value_stack[context];
      }

      octave_value varval (context_id context) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->varval (get_fwd_scope_context ());

        if (is_persistent ())
          context = 0;

        if (context < m_value_stack.size ())
          return m_value_stack[context];
        else
          return octave_value ();
      }

      void push_context (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return;

        if (! (is_persistent () || is_global ()))
          m_value_stack.push_back (octave_value ());
      }

      // If pop_context returns 0, we are out of values and this element
      // of the symbol table should be deleted.  This can happen for
      // functions like
      //
      //   function foo (n)
      //     if (n > 0)
      //       foo (n-1);
      //     else
      //       eval ("x = 1");
      //     endif
      //   endfunction
      //
      // Here, X should only exist in the final stack frame.

      size_t pop_context (void)
      {
        size_t retval = 1;

        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return retval;

        if (! (is_persistent () || is_global ()))
          {
            m_value_stack.pop_back ();
            retval = m_value_stack.size ();
          }

        return retval;
      }

      void clear (context_id context)
      {
        // There is no need to do anything with a fowarded
        // symbol_record_rep here.
        //
        // For scripts, we are never executing in the script "scope".
        //
        // For globals, we are only interested in breaking the link to
        // the global value and clearing the local value, not the
        // global one.

        // For persistent values, we clear the value then unmark so
        // that we clear the first element of the value stack.

        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return;

        if (! (is_hidden () || is_inherited ()))
          {
            if (is_global ())
              unbind_fwd_rep ();

            assign (octave_value (), context);

            if (is_persistent ())
              unmark_persistent ();
          }
      }

      bool is_defined (context_id context) const
      {
        return varval (context).is_defined ();
      }

      bool is_valid (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_valid ();

        return m_valid;
      }

      bool is_variable (context_id context) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_variable (context);

        return (! is_local () || is_defined (context));
      }

      bool is_local (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_local ();

        return m_storage_class & local;
      }

      bool is_automatic (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_automatic ();

        return m_storage_class & automatic;
      }

      bool is_formal (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_formal ();

        return m_storage_class & formal;
      }

      bool is_hidden (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_hidden ();

        return m_storage_class & hidden;
      }

      bool is_inherited (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_inherited ();

        return m_storage_class & inherited;
      }

      bool is_global (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_global ();

        return m_storage_class & global;
      }

      bool is_persistent (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_persistent ();

        return m_storage_class & persistent;
      }

      bool is_added_static (void) const
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          return t_fwd_rep->is_added_static ();

        return m_storage_class & added_static;
      }

      void mark_local (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_local ();
            return;
          }

        m_storage_class |= local;
      }

      void mark_automatic (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_automatic ();
            return;
          }

        m_storage_class |= automatic;
      }

      void mark_formal (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_formal ();
            return;
          }

        m_storage_class |= formal;
      }

      void mark_hidden (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_hidden ();
            return;
          }

        m_storage_class |= hidden;
      }

      void mark_inherited (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_inherited ();
            return;
          }

        m_storage_class |= inherited;
      }

      // This flag should only be set for a symbol record that is
      // actually in the global symbol_scope, and that should only
      // happen when it is added to the global symbol_scope.

      void mark_global (void)
      {
        m_storage_class |= global;
      }

      void mark_persistent (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_persistent ();
            return;
          }

        if (is_global ())
          error ("can't make global variable %s persistent", m_name.c_str ());

        m_storage_class |= persistent;
      }

      void mark_added_static (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->mark_added_static ();
            return;
          }

        m_storage_class |= added_static;
      }

      void unmark_local (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_local ();
            return;
          }

        m_storage_class &= ~local;
      }

      void unmark_automatic (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_automatic ();
            return;
          }

        m_storage_class &= ~automatic;
      }

      void unmark_formal (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_formal ();
            return;
          }

        m_storage_class &= ~formal;
      }

      void unmark_hidden (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_hidden ();
            return;
          }

        m_storage_class &= ~hidden;
      }

      void unmark_inherited (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_inherited ();
            return;
          }

        m_storage_class &= ~inherited;
      }

      void unmark_persistent (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_persistent ();
            return;
          }

        m_storage_class &= ~persistent;
      }

      void unmark_added_static (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->unmark_added_static ();
            return;
          }

        m_storage_class &= ~added_static;
      }

      unsigned int storage_class (void) const { return m_storage_class; }

      void init_persistent (void);

      void invalidate (void)
      {
        if (auto t_fwd_rep = m_fwd_rep.lock ())
          {
            t_fwd_rep->invalidate ();
            return;
          }

        m_valid = false;
      }

      void bind_fwd_rep (symbol_scope_rep *fwd_scope,
                         const std::shared_ptr<symbol_record_rep>& fwd_rep)
      {
        m_fwd_scope = fwd_scope;
        m_fwd_rep = fwd_rep;
      }

      void unbind_fwd_rep (void)
      {
        m_fwd_scope = nullptr;
        m_fwd_rep.reset ();
      }

      symbol_record_rep * dup (symbol_scope_rep *new_scope) const;

      octave_value dump (context_id context) const;

      std::string name (void) const { return m_name; }

      void rename (const std::string& new_name) { m_name = new_name; }

    private:

      unsigned int m_storage_class;

      std::string m_name;

      symbol_scope_rep *m_fwd_scope;

      std::weak_ptr<symbol_record_rep> m_fwd_rep;

      std::deque<octave_value> m_value_stack;

      bool m_valid;
    };

  public:

    symbol_record (const std::string& nm = "",
                   const octave_value& v = octave_value (),
                   unsigned int sc = local)
      : m_rep (new symbol_record_rep (nm, v, sc))
    { }

    symbol_record (const symbol_record& sr) = default;

    symbol_record& operator = (const symbol_record& sr) = default;

    ~symbol_record (void) = default;

    symbol_record dup (symbol_scope_rep *sid) const
    {
      return symbol_record (m_rep->dup (sid));
    }

    std::string name (void) const { return m_rep->name (); }

    void rename (const std::string& new_name) { m_rep->rename (new_name); }

    octave_value
    find (context_id context,
          const octave_value_list& args = octave_value_list ()) const;

    void assign (const octave_value& value, context_id context)
    {
      m_rep->assign (value, context);
    }

    void assign (octave_value::assign_op op,
                 const std::string& type,
                 const std::list<octave_value_list>& idx,
                 const octave_value& value, context_id context)
    {
      m_rep->assign (op, type, idx, value, context);
    }

    void assign (octave_value::assign_op op, const octave_value& value,
                 context_id context)
    {
      m_rep->assign (op, value, context);
    }

    void do_non_const_unary_op (octave_value::unary_op op, context_id context)
    {
      m_rep->do_non_const_unary_op (op, context);
    }

    void do_non_const_unary_op (octave_value::unary_op op,
                                const std::string& type,
                                const std::list<octave_value_list>& idx,
                                context_id context)
    {
      m_rep->do_non_const_unary_op (op, type, idx, context);
    }

    octave_value varval (context_id context) const
    {
      return m_rep->varval (context);
    }

    void push_context (void) { m_rep->push_context (); }

    size_t pop_context (void) { return m_rep->pop_context (); }

    void clear (context_id context) { m_rep->clear (context); }

    bool is_defined (context_id context) const
    {
      return m_rep->is_defined (context);
    }

    bool is_undefined (context_id context) const
    {
      return ! m_rep->is_defined (context);
    }

    bool is_valid (void) const
    {
      return m_rep->is_valid ();
    }

    bool is_variable (context_id context) const
    {
      return m_rep->is_variable (context);
    }

    bool is_local (void) const { return m_rep->is_local (); }
    bool is_automatic (void) const { return m_rep->is_automatic (); }
    bool is_formal (void) const { return m_rep->is_formal (); }
    bool is_global (void) const { return m_rep->is_global (); }
    bool is_hidden (void) const { return m_rep->is_hidden (); }
    bool is_inherited (void) const { return m_rep->is_inherited (); }
    bool is_persistent (void) const { return m_rep->is_persistent (); }
    bool is_added_static (void) const { return m_rep->is_added_static (); }

    void mark_local (void) { m_rep->mark_local (); }
    void mark_automatic (void) { m_rep->mark_automatic (); }
    void mark_formal (void) { m_rep->mark_formal (); }
    void mark_hidden (void) { m_rep->mark_hidden (); }
    void mark_inherited (void) { m_rep->mark_inherited (); }
    void mark_global (void) { m_rep->mark_global (); }
    void mark_persistent (void) { m_rep->mark_persistent (); }
    void mark_added_static (void) { m_rep->mark_added_static (); }

    void unmark_local (void) { m_rep->unmark_local (); }
    void unmark_automatic (void) { m_rep->unmark_automatic (); }
    void unmark_formal (void) { m_rep->unmark_formal (); }
    void unmark_hidden (void) { m_rep->unmark_hidden (); }
    void unmark_inherited (void) { m_rep->unmark_inherited (); }
    void unmark_persistent (void) { m_rep->unmark_persistent (); }
    void unmark_added_static (void) { m_rep->unmark_added_static (); }

    void init_persistent (void) { m_rep->init_persistent (); }

    void invalidate (void) { m_rep->invalidate (); }

    unsigned int storage_class (void) const { return m_rep->storage_class (); }

    void bind_fwd_rep (symbol_scope_rep *fwd_scope, const symbol_record& sr)
    {
      m_rep->bind_fwd_rep (fwd_scope, sr.m_rep);
    }

    void unbind_fwd_rep (void) { m_rep->unbind_fwd_rep (); }

    octave_value dump (context_id context) const
    {
      return m_rep->dump (context);
    }

  private:

    static octave_value dummy_octave_value;

    std::shared_ptr<symbol_record_rep> m_rep;

    // NEW_REP must be dynamically allocated or nullptr.
    symbol_record (symbol_record_rep *new_rep) : m_rep (new_rep) { }
  };
}

#endif
