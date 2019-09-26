/*

Copyright (C) 2012-2019 Michael Goffioul

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

#if ! defined (octave_cdef_object_h)
#define octave_cdef_object_h 1

#include "octave-config.h"

#include <map>
#include <string>

#include "oct-refcount.h"

#include "error.h"
#include "oct-map.h"
#include "ov.h"
#include "ovl.h"

namespace octave
{
  class cdef_object;
  class cdef_class;

  // This is mainly a boostrap class to declare the expected interface.
  // The actual base class is cdef_class_base, which is declared after
  // cdef_object, such that it can contain cdef_object objects.

  class
  cdef_object_rep
  {
  public:

    friend class cdef_object;

    cdef_object_rep (void) : m_count (1) { }

    cdef_object_rep& operator = (const cdef_object_rep&) = delete;

    virtual ~cdef_object_rep (void) = default;

    virtual cdef_class get_class (void) const;

    virtual void set_class (const cdef_class&)
    {
      err_invalid_object ("set_class");
    }

    virtual cdef_object_rep * clone (void) const
    {
      err_invalid_object ("clone");
    }

    virtual cdef_object_rep * empty_clone (void) const
    {
      err_invalid_object ("empty_clone");
    }

    virtual cdef_object_rep * copy (void) const
    {
      err_invalid_object ("copy");
    }

    virtual cdef_object_rep * make_array (void) const
    {
      err_invalid_object ("make_array");
    }

    virtual bool is_array (void) const { return false; }

    virtual bool is_value_object (void) const { return false; }

    virtual bool is_handle_object (void) const { return false; }

    virtual bool is_meta_object (void) const { return false; }

    virtual Array<cdef_object> array_value (void) const
    {
      err_invalid_object ("array_value");
    }

    virtual void put (const std::string&, const octave_value&)
    { err_invalid_object ("put"); }

    virtual octave_value get (const std::string&) const
    {
      err_invalid_object ("get");
    }

    virtual void set_property (octave_idx_type, const std::string&,
                               const octave_value&)
    {
      err_invalid_object ("set_property");
    }

    virtual octave_value get_property (octave_idx_type, const std::string&) const
    {
      err_invalid_object ("get_property");
    }

    virtual octave_value_list
    subsref (const std::string&, const std::list<octave_value_list>&,
             int, size_t&, const cdef_class&, bool)
    {
      err_invalid_object ("subsref");
    }

    virtual octave_value
    subsasgn (const std::string&, const std::list<octave_value_list>&,
              const octave_value&)
    {
      err_invalid_object ("subsasgn");
    }

    virtual string_vector map_keys (void) const;

    virtual bool is_valid (void) const { return false; }

    std::string class_name (void) const;

    virtual void mark_for_construction (const cdef_class&)
    {
      err_invalid_object ("mark_for_construction");
    }

    virtual bool is_constructed_for (const cdef_class&) const
    {
      err_invalid_object ("is_constructed_for");
    }

    virtual bool is_partially_constructed_for (const cdef_class&) const
    {
      err_invalid_object ("is_partially_constructed_for");
    }

    virtual void mark_as_constructed (void)
    {
      err_invalid_object ("mark_as_constructed");
    }

    virtual void mark_as_constructed (const cdef_class&)
    {
      err_invalid_object ("mark_as_constructed");
    }

    virtual bool is_constructed (void) const
    {
      err_invalid_object ("is_constructed");
    }

    virtual octave_idx_type static_count (void) const { return 0; }

    virtual void destroy (void) { delete this; }

    void release (const cdef_object& obj);

    virtual dim_vector dims (void) const { return dim_vector (); }

  protected:

    // Reference count
    refcount<octave_idx_type> m_count;

    // Restricted copying.

    cdef_object_rep (const cdef_object_rep&) : m_count (1) { }

  private:

    OCTAVE_NORETURN void err_invalid_object (const char *who) const
    {
      error ("%s: invalid object", who);
    }
  };

  class
  cdef_object
  {
  public:

    // FIXME: use a null object?
    cdef_object (void) : rep (new cdef_object_rep ()) { }

    cdef_object (const cdef_object& obj) : rep (obj.rep) { rep->m_count++; }

    cdef_object (cdef_object_rep *r) : rep (r) { }

    virtual ~cdef_object (void) { rep->release (*this); }

    cdef_object& operator = (const cdef_object& obj)
    {
      if (rep != obj.rep)
        {
          rep->release (*this);

          rep = obj.rep;
          rep->m_count++;
        }

      return *this;
    }

    cdef_class get_class (void) const;

    void set_class (const cdef_class& cls) { rep->set_class (cls); }

    std::string class_name (void) const { return rep->class_name (); }

    cdef_object clone (void) const { return cdef_object (rep->clone ()); }

    cdef_object empty_clone (void) const
    {
      return cdef_object (rep->empty_clone ());
    }

    dim_vector dims (void) const { return rep->dims (); }

    cdef_object make_array (void) const
    {
      return cdef_object (rep->make_array ());
    }

    cdef_object copy (void) const { return cdef_object (rep->copy ()); }

    bool is_array (void) const { return rep->is_array (); }

    bool is_value_object (void) const { return rep->is_value_object (); }

    bool is_handle_object (void) const { return rep->is_handle_object (); }

    bool is_meta_object (void) const { return rep->is_meta_object (); }

    Array<cdef_object> array_value (void) const { return rep->array_value (); }

    void put (const std::string& pname, const octave_value& val)
    {
      rep->put (pname, val);
    }

    octave_value get (const std::string& pname) const
    {
      return rep->get (pname);
    }

    void set_property (octave_idx_type idx, const std::string& pname,
                       const octave_value& pval)
    {
      return rep->set_property (idx, pname, pval);
    }

    octave_value
    get_property (octave_idx_type idx, const std::string& pname) const
    {
      return rep->get_property (idx, pname);
    }

    octave_value_list
    subsref (const std::string& type, const std::list<octave_value_list>& idx,
             int nargout, size_t& skip, const cdef_class& context,
             bool auto_add = false)
    {
      return rep->subsref (type, idx, nargout, skip, context, auto_add);
    }

    octave_value
    subsasgn (const std::string& type, const std::list<octave_value_list>& idx,
              const octave_value& rhs, int ignore_copies = 0)
    {
      make_unique (ignore_copies);
      return rep->subsasgn (type, idx, rhs);
    }

    string_vector map_keys (void) const { return rep->map_keys (); }

    octave_map map_value (void) const;

    const cdef_object_rep * get_rep (void) const { return rep; }

    bool ok (void) const { return rep->is_valid (); }

    void mark_for_construction (const cdef_class& cls)
    {
      rep->mark_for_construction (cls);
    }

    bool is_constructed (void) const { return rep->is_constructed (); }

    bool is_constructed_for (const cdef_class& cls) const
    {
      return rep->is_constructed_for (cls);
    }

    bool is_partially_constructed_for (const cdef_class& cls) const
    {
      return rep->is_partially_constructed_for (cls);
    }

    void mark_as_constructed (void) { rep->mark_as_constructed (); }

    void mark_as_constructed (const cdef_class& cls)
    { rep->mark_as_constructed (cls); }

    bool is (const cdef_object& obj) const { return rep == obj.rep; }

  protected:

    cdef_object_rep * get_rep (void) { return rep; }

    void make_unique (int ignore_copies)
    {
      if (rep->m_count > ignore_copies + 1)
        *this = clone ();
    }

  private:

    cdef_object_rep *rep;
  };

  class
  cdef_object_base : public cdef_object_rep
  {
  public:

    cdef_object_base (void)
      : cdef_object_rep (), klass ()
    { }

    cdef_object_base& operator = (const cdef_object_base&) = delete;

    ~cdef_object_base (void) { }

    cdef_class get_class (void) const;

    void set_class (const cdef_class& cls);

    cdef_object_rep * empty_clone (void) const
    {
      return new cdef_object_base (*this);
    }

    cdef_object_rep * make_array (void) const;

  protected:

    // Restricted copying!
    cdef_object_base (const cdef_object_base& obj)
      : cdef_object_rep (obj), klass (obj.klass)
    { }

  private:

    // The class of the object
    cdef_object klass;
  };

  class
  cdef_object_array : public cdef_object_base
  {
  public:

    cdef_object_array (void) : cdef_object_base () { }

    cdef_object_array (const Array<cdef_object>& a)
      : cdef_object_base (), array (a)
    { }

    cdef_object_array& operator = (const cdef_object_array&) = delete;

    ~cdef_object_array (void) = default;

    cdef_object_rep * clone (void) const
    {
      return new cdef_object_array (*this);
    }

    dim_vector dims (void) const { return array.dims (); }

    bool is_valid (void) const { return true; }

    bool is_array (void) const { return true; }

    Array<cdef_object> array_value (void) const { return array; }

    octave_value_list
    subsref (const std::string& type, const std::list<octave_value_list>& idx,
             int nargout, size_t& skip, const cdef_class& context,
             bool auto_add);

    octave_value
    subsasgn (const std::string& type, const std::list<octave_value_list>& idx,
              const octave_value& rhs);

    void set_property (octave_idx_type idx, const std::string& pname,
                       const octave_value& pval)
    {
      cdef_object& tmp = array.elem (idx);

      return tmp.put (pname, pval);
    }

    octave_value
    get_property (octave_idx_type idx, const std::string& pname) const
    {
      cdef_object tmp = array.elem (idx);

      return tmp.get (pname);
    }

  private:

    Array<cdef_object> array;

    void fill_empty_values (void) { fill_empty_values (array); }

    void fill_empty_values (Array<cdef_object>& arr);

    // Private copying!
    cdef_object_array (const cdef_object_array& obj)
      : cdef_object_base (obj), array (obj.array)
    { }
  };

  class
  cdef_object_scalar : public cdef_object_base
  {
  public:

    cdef_object_scalar (void) : cdef_object_base () { }

    cdef_object_scalar& operator = (const cdef_object_scalar&) = delete;

    ~cdef_object_scalar (void) = default;

    dim_vector dims (void) const { return dim_vector (1, 1); }

    void put (const std::string& pname, const octave_value& val)
    {
      map.assign (pname, val);
    }

    octave_value get (const std::string& pname) const
    {
      Cell val = map.contents (pname);

      if (val.numel () < 1)
        error ("get: unknown slot: %s", pname.c_str ());

      return val(0, 0);
    }

    void set_property (octave_idx_type idx, const std::string& pname,
                       const octave_value& pval)
    {
      if (idx != 0)
        error ("invalid index");  // FIXME

      put (pname, pval);
    }

    octave_value
    get_property (octave_idx_type idx, const std::string& pname) const
    {
      if (idx != 0)
        error ("invalid index");  // FIXME

      return get (pname);
    }

    octave_value_list
    subsref (const std::string& type, const std::list<octave_value_list>& idx,
             int nargout, size_t& skip, const cdef_class& context,
             bool auto_add);

    octave_value
    subsasgn (const std::string& type, const std::list<octave_value_list>& idx,
              const octave_value& rhs);

    void mark_for_construction (const cdef_class&);

    bool is_constructed_for (const cdef_class& cls) const;

    bool is_partially_constructed_for (const cdef_class& cls) const;

    void mark_as_constructed (void) { ctor_list.clear (); }

    void mark_as_constructed (const cdef_class& cls);

    bool is_constructed (void) const { return ctor_list.empty (); }

  protected:

    // Object property values
    octave_scalar_map map;

    // Internal/temporary structure used during object construction
    std::map< cdef_class, std::list<cdef_class>> ctor_list;

  protected:

    // Restricted object copying!
    cdef_object_scalar (const cdef_object_scalar& obj)
      : cdef_object_base (obj), map (obj.map), ctor_list (obj.ctor_list)
    { }
  };

  class
  handle_cdef_object : public cdef_object_scalar
  {
  public:

    handle_cdef_object (void) : cdef_object_scalar () { }

    handle_cdef_object& operator = (const handle_cdef_object&) = delete;

    ~handle_cdef_object (void);

    cdef_object_rep * clone (void) const
    {
      handle_cdef_object *obj = const_cast<handle_cdef_object *> (this);
      obj->m_count++;
      return obj;
    }

    cdef_object_rep * copy (void) const
    {
      return new handle_cdef_object (*this);
    }

    bool is_valid (void) const { return true; }

    bool is_handle_object (void) const { return true; }

  protected:

    // Restricted copying!
    handle_cdef_object (const handle_cdef_object& obj)
      : cdef_object_scalar (obj)
    { }
  };

  class
  value_cdef_object : public cdef_object_scalar
  {
  public:

    value_cdef_object (void) : cdef_object_scalar () { }

    value_cdef_object& operator = (const value_cdef_object&) = delete;

    ~value_cdef_object (void);

    cdef_object_rep * clone (void) const
    {
      return new value_cdef_object (*this);
    }

    cdef_object_rep * copy (void) const { return clone (); }

    bool is_valid (void) const { return true; }

    bool is_value_object (void) const { return true; }

  private:

    // Private copying!
    value_cdef_object (const value_cdef_object& obj)
      : cdef_object_scalar (obj)
    { }
  };

  class
  cdef_meta_object_rep : public handle_cdef_object
  {
  public:

    cdef_meta_object_rep (void) : handle_cdef_object () { }

    cdef_meta_object_rep& operator = (const cdef_meta_object_rep&) = delete;

    ~cdef_meta_object_rep (void) = default;

    cdef_object_rep * copy (void) const
    { return new cdef_meta_object_rep (*this); }

    bool is_meta_object (void) const { return true; }

    virtual bool is_class (void) const { return false; }

    virtual bool is_property (void) const { return false; }

    virtual bool is_method (void) const { return false; }

    virtual bool is_package (void) const { return false; }

    virtual octave_value_list
    meta_subsref (const std::string& /* type */,
                  const std::list<octave_value_list>& /* idx */,
                  int /* nargout */)
    {
      error ("subsref: invalid meta object");
    }

    virtual void meta_release (void) { }

    virtual bool meta_accepts_postfix_index (char /* type */) const
    {
      return false;
    }

  protected:

    // Restricted copying!
    cdef_meta_object_rep (const cdef_meta_object_rep& obj)
      : handle_cdef_object (obj)
    { }
  };

  class
  cdef_meta_object : public cdef_object
  {
  public:

    cdef_meta_object (void) : cdef_object () { }

    // Object consistency is checked in sub-classes.
    cdef_meta_object (const cdef_meta_object& obj) : cdef_object (obj) { }

    cdef_meta_object (cdef_meta_object_rep *r) : cdef_object (r) { }

    cdef_meta_object (const cdef_object& obj) : cdef_object (obj) { }

    cdef_meta_object& operator = (const cdef_object&) = delete;

    ~cdef_meta_object (void) = default;

    bool is_class (void) const { return get_rep ()->is_class (); }

    bool is_property (void) const { return get_rep ()->is_property (); }

    bool is_method (void) const { return get_rep ()->is_method (); }

    bool is_package (void) const { return get_rep ()->is_package (); }

    octave_value_list
    meta_subsref (const std::string& type,
                  const std::list<octave_value_list>& idx, int nargout)
    {
      return get_rep ()->meta_subsref (type, idx, nargout);
    }

    void meta_release (void) { get_rep ()->meta_release (); }

    bool meta_accepts_postfix_index (char type) const
    {
      return get_rep ()->meta_accepts_postfix_index (type);
    }

  private:

    cdef_meta_object_rep * get_rep (void)
    {
      return dynamic_cast<cdef_meta_object_rep *> (cdef_object::get_rep ());
    }

    const cdef_meta_object_rep * get_rep (void) const
    {
      return dynamic_cast<const cdef_meta_object_rep *> (cdef_object::get_rep ());
    }
  };
}

#endif
