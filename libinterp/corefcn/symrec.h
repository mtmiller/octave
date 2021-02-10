////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2021 The Octave Project Developers
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

    // formal parameter
    static const unsigned int formal = 2;

    // this symbol may NOT become a variable.
    // (symbol added to a static workspace)
    static const unsigned int added_static = 4;

  private:

    class symbol_record_rep
    {
    public:

      symbol_record_rep (const std::string& nm, unsigned int sc)
        : m_frame_offset (0), m_data_offset (0), m_storage_class (sc),
          m_name (nm)
      { }

      symbol_record_rep (const symbol_record_rep&) = default;

      symbol_record_rep& operator = (const symbol_record_rep&) = default;

      ~symbol_record_rep (void) = default;

      // FIXME: use special storage class instead?
      bool is_valid (void) const { return ! m_name.empty (); }

      void set_frame_offset (size_t offset) { m_frame_offset = offset; }

      size_t frame_offset (void) const { return m_frame_offset; }

      void set_data_offset (size_t offset) { m_data_offset = offset; }

      size_t data_offset (void) const { return m_data_offset; }

      bool is_local (void) const
      {
        return m_storage_class & local;
      }

      bool is_formal (void) const
      {
        return m_storage_class & formal;
      }

      bool is_added_static (void) const
      {
        return m_storage_class & added_static;
      }

      void mark_local (void)
      {
        m_storage_class |= local;
      }

      void mark_formal (void)
      {
        m_storage_class |= formal;
      }

      void mark_added_static (void)
      {
        m_storage_class |= added_static;
      }

      void unmark_local (void)
      {
        m_storage_class &= ~local;
      }

      void unmark_formal (void)
      {
        m_storage_class &= ~formal;
      }

      void unmark_added_static (void)
      {
        m_storage_class &= ~added_static;
      }

      unsigned int storage_class (void) const { return m_storage_class; }

      std::shared_ptr<symbol_record_rep> dup (void) const;

      octave_value dump (void) const;

      std::string name (void) const { return m_name; }

      void rename (const std::string& new_name) { m_name = new_name; }

    private:

      size_t m_frame_offset;
      size_t m_data_offset;

      unsigned int m_storage_class;

      std::string m_name;
    };

  public:

    symbol_record (const std::string& nm = "", unsigned int sc = local)
      : m_rep (new symbol_record_rep (nm, sc))
    { }

    symbol_record (const std::string& nm, const octave_value&,
                   unsigned int sc = local)
      : m_rep (new symbol_record_rep (nm, sc))
    { }

    symbol_record (const symbol_record&) = default;

    symbol_record& operator = (const symbol_record&) = default;

    ~symbol_record (void) = default;

    bool is_valid (void) const { return m_rep->is_valid (); }

    explicit operator bool () const { return is_valid (); }

    void set_frame_offset (size_t offset) { m_rep->set_frame_offset (offset); }

    size_t frame_offset (void) const { return m_rep->frame_offset (); }

    void set_data_offset (size_t offset) { m_rep->set_data_offset (offset); }

    size_t data_offset (void) const { return m_rep->data_offset (); }

    symbol_record dup (void) const { return symbol_record (m_rep->dup ()); }

    std::string name (void) const { return m_rep->name (); }

    void rename (const std::string& new_name) { m_rep->rename (new_name); }

    bool is_local (void) const { return m_rep->is_local (); }
    bool is_formal (void) const { return m_rep->is_formal (); }
    bool is_added_static (void) const { return m_rep->is_added_static (); }

    void mark_local (void) { m_rep->mark_local (); }
    void mark_formal (void) { m_rep->mark_formal (); }
    void mark_added_static (void) { m_rep->mark_added_static (); }

    void unmark_local (void) { m_rep->unmark_local (); }
    void unmark_formal (void) { m_rep->unmark_formal (); }
    void unmark_added_static (void) { m_rep->unmark_added_static (); }

    unsigned int storage_class (void) const { return m_rep->storage_class (); }

    octave_value dump (void) const { return m_rep->dump (); }

  private:

    std::shared_ptr<symbol_record_rep> m_rep;

    // NEW_REP must be dynamically allocated or nullptr.
    symbol_record (const std::shared_ptr<symbol_record_rep>& new_rep)
      : m_rep (new_rep)
    { }
  };
}

#endif
