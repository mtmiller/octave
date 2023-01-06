////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2023 The Octave Project Developers
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

#if ! defined (octave_oct_map_h)
#define octave_oct_map_h 1

#include "octave-config.h"

#include <algorithm>
#include <map>

#include "oct-refcount.h"

#include "Cell.h"
#include "ovl.h"

class string_vector;

// A class holding a map field->index.  Supports reference-counting.
class OCTINTERP_API
  octave_fields
{
  class fields_rep : public std::map<std::string, octave_idx_type>
  {
  public:
    fields_rep (void) : std::map<std::string, octave_idx_type> (), m_count (1) { }
    fields_rep (const fields_rep& other)
      : std::map<std::string, octave_idx_type> (other), m_count (1) { }

    octave::refcount<octave_idx_type> m_count;

  private:
    fields_rep& operator = (const fields_rep&); // no assignment!
  };

  fields_rep *m_rep;

  static fields_rep * nil_rep (void);

public:

  octave_fields (void) : m_rep (nil_rep ()) { m_rep->m_count++; }
  octave_fields (const string_vector&);
  octave_fields (const char *const *);

  ~octave_fields (void)
  {
    if (--m_rep->m_count == 0)
      delete m_rep;
  }

  void make_unique (void)
  {
    if (m_rep->m_count > 1)
      {
        fields_rep *r = new fields_rep (*m_rep);

        if (--m_rep->m_count == 0)
          delete m_rep;

        m_rep = r;
      }
  }

  octave_fields (const octave_fields& o) : m_rep (o.m_rep) { m_rep->m_count++; }

  octave_fields&
  operator = (const octave_fields& o)
  {
    if (&o != this)
      {
        o.m_rep->m_count++;
        if (--m_rep->m_count == 0)
          delete m_rep;
        m_rep = o.m_rep;
      }

    return *this;
  }

  // constant iteration support. non-const iteration intentionally unsupported.

  typedef std::map<std::string, octave_idx_type>::const_iterator const_iterator;
  typedef const_iterator iterator;

  const_iterator begin (void) const { return m_rep->begin (); }
  const_iterator end (void) const { return m_rep->end (); }

  const_iterator cbegin (void) const { return m_rep->cbegin (); }
  const_iterator cend (void) const { return m_rep->cend (); }

  std::string key (const_iterator p) const { return p->first; }
  octave_idx_type index (const_iterator p) const { return p->second; }

  const_iterator seek (const std::string& k) const
  { return m_rep->find (k); }

  // high-level methods.

  // number of fields.
  octave_idx_type nfields (void) const { return m_rep->size (); }

  // check whether a field exists.
  bool isfield (const std::string& name) const;

  // get index of field.  return -1 if not exist
  octave_idx_type getfield (const std::string& name) const;
  // get index of field.  add if not exist
  octave_idx_type getfield (const std::string& name);
  // remove field and return the index. -1 if didn't exist.
  octave_idx_type rmfield (const std::string& name);

  // order the fields of this map.
  // creates a permutation used to order the fields.
  void orderfields (Array<octave_idx_type>& perm);

  // compares two instances for equality up to order of fields.
  // returns a permutation needed to bring the fields of *other*
  // into the order of *this*.
  bool equal_up_to_order (const octave_fields& other,
                          octave_idx_type *perm) const;

  bool equal_up_to_order (const octave_fields& other,
                          Array<octave_idx_type>& perm) const;

  bool is_same (const octave_fields& other) const
  { return m_rep == other.m_rep; }

  // Returns the fields as a vector of strings.
  string_vector fieldnames (void) const;

  void clear (void)
  {
    *this = octave_fields ();
  }
};

class OCTINTERP_API
  octave_scalar_map
{
public:

  octave_scalar_map (const octave_fields& k)
    : m_keys (k), m_vals (k.nfields ()) { }

  octave_scalar_map (void) : m_keys (), m_vals () { }

  octave_scalar_map (const string_vector& k)
    : m_keys (k), m_vals (k.numel ()) { }

  octave_scalar_map (const octave_scalar_map& m)
    : m_keys (m.m_keys), m_vals (m.m_vals) { }

  octave_scalar_map (const std::map<std::string, octave_value>& m);

  octave_scalar_map& operator = (const octave_scalar_map& m)
  {
    m_keys = m.m_keys;
    m_vals = m.m_vals;

    return *this;
  }

  // iteration support.
  // note that both const and non-const iterators are the same.
  // The const/non-const distinction is made by the key & contents method.
  typedef octave_fields::const_iterator const_iterator;
  typedef const_iterator iterator;

  const_iterator begin (void) const { return m_keys.begin (); }
  const_iterator end (void) const { return m_keys.end (); }

  const_iterator cbegin (void) const { return m_keys.cbegin (); }
  const_iterator cend (void) const { return m_keys.cend (); }

  const_iterator seek (const std::string& k) const { return m_keys.seek (k); }

  std::string key (const_iterator p) const
  { return m_keys.key (p); }
  octave_idx_type index (const_iterator p) const
  { return m_keys.index (p); }

  const octave_value& contents (const_iterator p) const
  { return m_vals[m_keys.index (p)]; }

  octave_value& contents (iterator p)
  { return m_vals[m_keys.index (p)]; }

  const octave_value& contents (octave_idx_type i) const
  { return m_vals[i]; }

  octave_value& contents (octave_idx_type i)
  { return m_vals[i]; }

  // number of fields.
  octave_idx_type nfields (void) const { return m_keys.nfields (); }

  // check whether a field exists.
  bool isfield (const std::string& name) const
  { return m_keys.isfield (name); }

  bool contains (const std::string& name) const
  { return isfield (name); }

  string_vector fieldnames (void) const
  { return m_keys.fieldnames (); }

  string_vector keys (void) const
  { return fieldnames (); }

  // get contents of a given field.  empty value if not exist.
  octave_value getfield (const std::string& key) const;

  // set contents of a given field.  add if not exist.
  void setfield (const std::string& key, const octave_value& val);
  void assign (const std::string& k, const octave_value& val)
  { setfield (k, val); }

  // remove a given field.  do nothing if not exist.
  void rmfield (const std::string& key);
  void del (const std::string& k) { rmfield (k); }

  // return a copy with fields ordered, optionally along with permutation.
  octave_scalar_map orderfields (void) const;
  octave_scalar_map orderfields (Array<octave_idx_type>& perm) const;
  octave_scalar_map orderfields (const octave_scalar_map& other,
                                 Array<octave_idx_type>& perm) const;

  // aka getfield/setfield, but the latter returns a reference.
  octave_value contents (const std::string& k) const;
  octave_value& contents (const std::string& k);

  void clear (void)
  {
    m_keys.clear ();
    m_vals.clear ();
  }

  friend class octave_map;

private:

  octave_fields m_keys;
  std::vector<octave_value> m_vals;

};

template <>
inline octave_scalar_map
octave_value_extract<octave_scalar_map> (const octave_value& v)
{ return v.scalar_map_value (); }

class OCTINTERP_API
  octave_map
{
public:

  octave_map (const octave_fields& k)
    : m_keys (k), m_vals (k.nfields ()), m_dimensions () { }

  octave_map (const dim_vector& dv, const octave_fields& k)
    : m_keys (k), m_vals (k.nfields (), Cell (dv)), m_dimensions (dv) { }

  typedef octave_scalar_map element_type;

  octave_map (void) : m_keys (), m_vals (), m_dimensions () { }

  octave_map (const dim_vector& dv) : m_keys (), m_vals (), m_dimensions (dv) { }

  octave_map (const string_vector& k)
    : m_keys (k), m_vals (k.numel (), Cell (1, 1)), m_dimensions (1, 1) { }

  octave_map (const dim_vector& dv, const string_vector& k)
    : m_keys (k), m_vals (k.numel (), Cell (dv)), m_dimensions (dv) { }

  octave_map (const octave_map& m)
    : m_keys (m.m_keys), m_vals (m.m_vals), m_dimensions (m.m_dimensions) { }

  octave_map (const octave_scalar_map& m);

  octave_map& operator = (const octave_map& m)
  {
    m_keys = m.m_keys;
    m_vals = m.m_vals;
    m_dimensions = m.m_dimensions;

    return *this;
  }

  // iteration support.
  // note that both const and non-const iterators are the same.
  // The const/non-const distinction is made by the key & contents method.
  typedef octave_fields::const_iterator const_iterator;
  typedef const_iterator iterator;

  const_iterator begin (void) const { return m_keys.begin (); }
  const_iterator end (void) const { return m_keys.end (); }

  const_iterator cbegin (void) const { return m_keys.cbegin (); }
  const_iterator cend (void) const { return m_keys.cend (); }

  const_iterator seek (const std::string& k) const { return m_keys.seek (k); }

  std::string key (const_iterator p) const
  { return m_keys.key (p); }
  octave_idx_type index (const_iterator p) const
  { return m_keys.index (p); }

  const Cell& contents (const_iterator p) const
  { return m_vals[m_keys.index (p)]; }

  Cell& contents (iterator p)
  { return m_vals[m_keys.index (p)]; }

  const Cell& contents (octave_idx_type i) const
  { return m_vals[i]; }

  Cell& contents (octave_idx_type i)
  { return m_vals[i]; }

  // number of fields.
  octave_idx_type nfields (void) const { return m_keys.nfields (); }

  // check whether a field exists.
  bool isfield (const std::string& name) const
  { return m_keys.isfield (name); }

  bool contains (const std::string& name) const
  { return isfield (name); }

  string_vector fieldnames (void) const
  { return m_keys.fieldnames (); }

  string_vector keys (void) const
  { return fieldnames (); }

  // get contents of a given field.  empty value if not exist.
  Cell getfield (const std::string& key) const;

  // set contents of a given field.  add if not exist.  checks for
  // correct m_dimensions.
  void setfield (const std::string& key, const Cell& val);
  void assign (const std::string& k, const Cell& val)
  { setfield (k, val); }

  // remove a given field. do nothing if not exist.
  void rmfield (const std::string& key);
  void del (const std::string& k) { rmfield (k); }

  // return a copy with fields ordered, optionally along with permutation.
  octave_map orderfields (void) const;
  octave_map orderfields (Array<octave_idx_type>& perm) const;
  octave_map orderfields (const octave_map& other,
                          Array<octave_idx_type>& perm) const;

  // aka getfield/setfield, but the latter returns a reference.
  Cell contents (const std::string& k) const;
  Cell& contents (const std::string& k);

  void clear (void)
  {
    m_keys.clear ();
    m_vals.clear ();
  }

  // The Array-like methods.
  octave_idx_type numel (void) const { return m_dimensions.numel (); }
  octave_idx_type length (void) const { return numel (); }
  bool isempty (void) const { return m_dimensions.any_zero (); }

  octave_idx_type rows (void) const { return m_dimensions(0); }
  octave_idx_type cols (void) const { return m_dimensions(1); }
  octave_idx_type columns (void) const { return m_dimensions(1); }

  // Extract a scalar substructure.
  // FIXME: actually check something.
  octave_scalar_map checkelem (octave_idx_type n) const
  { return elem (n); }

  // FIXME: actually check something.
  octave_scalar_map checkelem (octave_idx_type i, octave_idx_type j) const
  { return elem (i, j); }

  // FIXME: actually check something.
  octave_scalar_map checkelem (const Array<octave_idx_type>& ra_idx) const
  { return elem (ra_idx); }

  octave_scalar_map elem (octave_idx_type n) const;

  octave_scalar_map elem (octave_idx_type i, octave_idx_type j) const;

  octave_scalar_map elem (const Array<octave_idx_type>& ra_idx) const;

  octave_scalar_map operator () (octave_idx_type n) const
  { return elem (n); }

  octave_scalar_map operator () (octave_idx_type i, octave_idx_type j) const
  { return elem (i, j); }

  octave_scalar_map
  operator () (const Array<octave_idx_type>& ra_idx) const
  { return elem (ra_idx); }

  octave_map squeeze (void) const;

  octave_map permute (const Array<int>& vec, bool inv = false) const;

  dim_vector dims (void) const { return m_dimensions; }

  int ndims (void) const { return m_dimensions.ndims (); }

  octave_map transpose (void) const;

  octave_map reshape (const dim_vector& dv) const;

  void resize (const dim_vector& dv, bool fill = false);

  static octave_map
  cat (int dim, octave_idx_type n, const octave_scalar_map *map_list);

  static octave_map
  cat (int dim, octave_idx_type n, const octave_map *map_list);

  octave_map index (const octave::idx_vector& i, bool resize_ok = false) const;

  octave_map index (const octave::idx_vector& i, const octave::idx_vector& j,
                    bool resize_ok = false) const;

  octave_map index (const Array<octave::idx_vector>& ia,
                    bool resize_ok = false) const;

  octave_map index (const octave_value_list&, bool resize_ok = false) const;

  octave_map column (octave_idx_type k) const;
  octave_map page (octave_idx_type k) const;

  void assign (const octave::idx_vector& i, const octave_map& rhs);

  void assign (const octave::idx_vector& i, const octave::idx_vector& j,
               const octave_map& rhs);

  void assign (const Array<octave::idx_vector>& ia, const octave_map& rhs);

  void assign (const octave_value_list&, const octave_map& rhs);

  void assign (const octave_value_list& idx, const std::string& k,
               const Cell& rhs);

  void delete_elements (const octave::idx_vector& i);

  void delete_elements (int dim, const octave::idx_vector& i);

  void delete_elements (const Array<octave::idx_vector>& ia);

  void delete_elements (const octave_value_list&);

  octave_map concat (const octave_map& rb,
                     const Array<octave_idx_type>& ra_idx);

  // like checkelem, but no check.
  octave_scalar_map fast_elem_extract (octave_idx_type n) const;

  // element assignment, no bounds check
  bool fast_elem_insert (octave_idx_type n, const octave_scalar_map& rhs);

private:

  octave_fields m_keys;
  std::vector<Cell> m_vals;
  dim_vector m_dimensions;

  void optimize_dimensions (void);
  void extract_scalar (octave_scalar_map& dest,
                       octave_idx_type index) const;
  static void do_cat (int dim, octave_idx_type n,
                      const octave_scalar_map *map_list, octave_map& retval);
  static void do_cat (int dim, octave_idx_type n,
                      const octave_map *map_list, octave_map& retval);
};

template <>
inline octave_map octave_value_extract<octave_map> (const octave_value& v)
{ return v.map_value (); }

#endif
