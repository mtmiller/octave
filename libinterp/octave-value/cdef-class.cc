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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <algorithm>
#include <iomanip>

#include "cdef-class.h"
#include "cdef-manager.h"
#include "cdef-utils.h"
#include "errwarn.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "load-path.h"
#include "ov-builtin.h"
#include "ov-classdef.h"
#include "ov-fcn-handle.h"
#include "ov-usr-fcn.h"
#include "parse.h"
#include "pt-assign.h"
#include "pt-classdef.h"
#include "pt-eval.h"
#include "pt-idx.h"
#include "pt-misc.h"
#include "pt-stmt.h"
#include "pt-walk.h"

// Define to 1 to enable debugging statements.
#define DEBUG_TRACE 0

static octave_value
make_fcn_handle (const octave_value& fcn, const std::string& nm)
{
  octave_value retval;

  if (fcn.is_defined ())
    retval = octave_value (new octave_fcn_handle (fcn, nm));

  return retval;
}

cdef_class::cdef_class_rep::cdef_class_rep (const std::list<cdef_class>& superclasses)
  : cdef_meta_object_rep (), member_count (0), handle_class (false),
    object_count (0), meta (false)
{
  put ("SuperClasses", to_ov (superclasses));
  implicit_ctor_list = superclasses;
}

cdef_method
cdef_class::cdef_class_rep::find_method (const std::string& nm, bool local)
{
  auto it = method_map.find (nm);

  if (it == method_map.end ())
    {
      // FIXME: look into class directory
    }
  else
    {
      cdef_method& meth = it->second;

      // FIXME: check if method reload needed

      if (meth.ok ())
        return meth;
    }

  if (! local)
    {
      // Look into superclasses

      Cell super_classes = get ("SuperClasses").cell_value ();

      for (int i = 0; i < super_classes.numel (); i++)
        {
          cdef_class cls = lookup_class (super_classes(i));

          cdef_method meth = cls.find_method (nm);

          if (meth.ok ())
            return meth;
        }
    }

  return cdef_method ();
}

class ctor_analyzer : public octave::tree_walker
{
public:

  ctor_analyzer (void) = delete;

  ctor_analyzer (const std::string& ctor, const std::string& obj)
    : octave::tree_walker (), who (ctor), obj_name (obj) { }

  ctor_analyzer (const ctor_analyzer&) = delete;

  ctor_analyzer& operator = (const ctor_analyzer&) = delete;

  ~ctor_analyzer (void) = default;

  void visit_statement_list (octave::tree_statement_list& t)
  {
    for (const auto& stmt_p : t)
      stmt_p->accept (*this);
  }

  void visit_statement (octave::tree_statement& t)
  {
    if (t.is_expression ())
      t.expression ()->accept (*this);
  }

  void visit_simple_assignment (octave::tree_simple_assignment& t)
  {
    t.right_hand_side ()->accept (*this);
  }

  void visit_multi_assignment (octave::tree_multi_assignment& t)
  {
    t.right_hand_side ()->accept (*this);
  }

  void visit_index_expression (octave::tree_index_expression& t)
  {
    t.expression ()->accept (*this);
  }

  std::list<cdef_class> get_constructor_list (void) const
  { return ctor_list; }

  // NO-OP

  void visit_anon_fcn_handle (octave::tree_anon_fcn_handle&) { }
  void visit_argument_list (octave::tree_argument_list&) { }
  void visit_binary_expression (octave::tree_binary_expression&) { }
  void visit_break_command (octave::tree_break_command&) { }
  void visit_colon_expression (octave::tree_colon_expression&) { }
  void visit_continue_command (octave::tree_continue_command&) { }
  void visit_decl_command (octave::tree_decl_command&) { }
  void visit_decl_init_list (octave::tree_decl_init_list&) { }
  void visit_decl_elt (octave::tree_decl_elt&) { }
  void visit_simple_for_command (octave::tree_simple_for_command&) { }
  void visit_complex_for_command (octave::tree_complex_for_command&) { }
  void visit_octave_user_script (octave_user_script&) { }
  void visit_octave_user_function (octave_user_function&) { }
  void visit_function_def (octave::tree_function_def&) { }
  void visit_identifier (octave::tree_identifier&) { }
  void visit_if_clause (octave::tree_if_clause&) { }
  void visit_if_command (octave::tree_if_command&) { }
  void visit_if_command_list (octave::tree_if_command_list&) { }
  void visit_switch_case (octave::tree_switch_case&) { }
  void visit_switch_case_list (octave::tree_switch_case_list&) { }
  void visit_switch_command (octave::tree_switch_command&) { }
  void visit_matrix (octave::tree_matrix&) { }
  void visit_cell (octave::tree_cell&) { }
  void visit_no_op_command (octave::tree_no_op_command&) { }
  void visit_constant (octave::tree_constant&) { }
  void visit_fcn_handle (octave::tree_fcn_handle&) { }
  void visit_parameter_list (octave::tree_parameter_list&) { }
  void visit_postfix_expression (octave::tree_postfix_expression&) { }
  void visit_prefix_expression (octave::tree_prefix_expression&) { }
  void visit_return_command (octave::tree_return_command&) { }
  void visit_return_list (octave::tree_return_list&) { }
  void visit_try_catch_command (octave::tree_try_catch_command&) { }
  void visit_unwind_protect_command (octave::tree_unwind_protect_command&) { }
  void visit_while_command (octave::tree_while_command&) { }
  void visit_do_until_command (octave::tree_do_until_command&) { }

  void visit_superclass_ref (octave::tree_superclass_ref& t)
  {
    if (t.method_name () == obj_name)
      {
        std::string class_name = t.class_name ();

        cdef_class cls = lookup_class (class_name, false);

        if (cls.ok ())
          ctor_list.push_back (cls);
      }
  }

private:

  // The name of the constructor being analyzed.
  std::string who;

  // The name of the first output argument of the constructor.
  std::string obj_name;

  // The list of superclass constructors that are explicitly called.
  std::list<cdef_class> ctor_list;
};

void
cdef_class::cdef_class_rep::install_method (const cdef_method& meth)
{
  method_map[meth.get_name ()] = meth;

  member_count++;

  if (meth.is_constructor ())
    {
      // Analyze the constructor code to determine what superclass
      // constructors are called explicitly.

      octave_function *of = meth.get_function ().function_value (true);

      if (of)
        {
          octave_user_function *uf = of->user_function_value (true);

          if (uf)
            {
              octave::tree_parameter_list *ret_list = uf->return_list ();
              octave::tree_statement_list *body = uf->body ();

              if (! ret_list || ret_list->size () != 1)
                error ("%s: invalid constructor output arguments",
                       meth.get_name ().c_str ());

              std::string obj_name = ret_list->front ()->name ();
              ctor_analyzer a (meth.get_name (), obj_name);

              body->accept (a);

              std::list<cdef_class> explicit_ctor_list
                = a.get_constructor_list ();

              for (const auto& cdef_cls : explicit_ctor_list)
                {
#if DEBUG_TRACE
                  std::cerr << "explicit superclass constructor: "
                            << cdef_cls.get_name () << std::endl;
#endif

                  implicit_ctor_list.remove (cdef_cls);
                }
            }
        }
    }
}

void
cdef_class::cdef_class_rep::load_all_methods (void)
{
  // FIXME: re-scan class directory
}

Cell
cdef_class::cdef_class_rep::get_methods (void)
{
  std::map<std::string,cdef_method> meths;

  find_methods (meths, false);

  Cell c (meths.size (), 1);

  int idx = 0;

  for (const auto& nm_mthd : meths)
    c(idx++, 0) = to_ov (nm_mthd.second);

  return c;
}

std::map<std::string, cdef_method>
cdef_class::cdef_class_rep::get_method_map (bool only_inherited)
{
  std::map<std::string, cdef_method> methods;

  find_methods (methods, only_inherited);

  return methods;
}

void
cdef_class::cdef_class_rep::find_methods (std::map<std::string,
                                          cdef_method>& meths,
                                          bool only_inherited)
{
  load_all_methods ();

  method_const_iterator it;

  for (it = method_map.begin (); it != method_map.end (); ++it)
    {
      if (! it->second.is_constructor ())
        {
          std::string nm = it->second.get_name ();

          if (meths.find (nm) == meths.end ())
            {
              if (only_inherited)
                {
                  octave_value acc = it->second.get ("Access");

                  if (! acc.is_string ()
                      || acc.string_value () == "private")
                    continue;
                }

              meths[nm] = it->second;
            }
        }
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      cls.get_rep ()->find_methods (meths, true);
    }
}

cdef_property
cdef_class::cdef_class_rep::find_property (const std::string& nm)
{
  auto it = property_map.find (nm);

  if (it != property_map.end ())
    {
      cdef_property& prop = it->second;

      if (prop.ok ())
        return prop;
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      cdef_property prop = cls.find_property (nm);

      if (prop.ok ())
        return prop;
    }

  return cdef_property ();
}

void
cdef_class::cdef_class_rep::install_property (const cdef_property& prop)
{
  property_map[prop.get_name ()] = prop;

  member_count++;
}

Cell
cdef_class::cdef_class_rep::get_properties (int mode)
{
  std::map<std::string,cdef_property> props;

  props = get_property_map (mode);

  Cell c (props.size (), 1);

  int idx = 0;

  for (const auto& pname_prop : props)
    c(idx++, 0) = to_ov (pname_prop.second);

  return c;
}

std::map<std::string, cdef_property>
cdef_class::cdef_class_rep::get_property_map (int mode)
{
  std::map<std::string,cdef_property> props;

  find_properties (props, mode);

  return props;
}

void
cdef_class::cdef_class_rep::find_properties (std::map<std::string,
                                             cdef_property>& props,
                                             int mode)
{
  property_const_iterator it;

  for (it = property_map.begin (); it != property_map.end (); ++it)
    {
      std::string nm = it->second.get_name ();

      if (props.find (nm) == props.end ())
        {
          if (mode == property_inherited)
            {
              octave_value acc = it->second.get ("GetAccess");

              if (! acc.is_string ()
                  || acc.string_value () == "private")
                continue;
            }

          props[nm] = it->second;
        }
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      cls.get_rep ()->find_properties (props,
                                       (mode == property_all
                                        ? property_all
                                        : property_inherited));
    }
}

void
cdef_class::cdef_class_rep::find_names (std::set<std::string>& names,
                                        bool all)
{
  load_all_methods ();

  for (const auto& cls_fnmap : method_map)
    {
      if (! cls_fnmap.second.is_constructor ())
        {
          std::string nm = cls_fnmap.second.get_name ();

          if (! all)
            {
              octave_value acc = cls_fnmap.second.get ("Access");

              if (! acc.is_string()
                  || acc.string_value () != "public")
                continue;
            }

          names.insert (nm);
        }
    }

  for (const auto& pname_prop : property_map)
    {
      std::string nm = pname_prop.second.get_name ();

      if (! all)
        {
          octave_value acc = pname_prop.second.get ("GetAccess");

          if (! acc.is_string()
              || acc.string_value () != "public")
            continue;
        }

      names.insert (nm);
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      cls.get_rep ()->find_names (names, all);
    }
}

string_vector
cdef_class::cdef_class_rep::get_names (void)
{
  std::set<std::string> names;

  find_names (names, false);

  string_vector v (names);

  return v.sort (true);
}

void
cdef_class::cdef_class_rep::delete_object (const cdef_object& obj)
{
  cdef_method dtor = find_method ("delete");

  if (dtor.ok ())
    dtor.execute (obj, octave_value_list (), 0, true, "destructor");

  // FIXME: should we destroy corresponding properties here?

  // Call "delete" in super classes

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (cls.get_name () != "handle")
        cls.delete_object (obj);
    }
}

octave_value_list
cdef_class::cdef_class_rep::meta_subsref (const std::string& type,
                                          const std::list<octave_value_list>& idx,
                                          int nargout)
{
  size_t skip = 1;

  octave_value_list retval;

  switch (type[0])
    {
    case '(':
      // Constructor call

#if DEBUG_TRACE
      std::cerr << "constructor" << std::endl;
#endif

      retval(0) = construct (idx.front ());
      break;

    case '.':
      {
        // Static method, constant (or property?)

#if DEBUG_TRACE
        std::cerr << "static method/property" << std::endl;
#endif

        if (idx.front ().length () != 1)
          error ("invalid meta.class indexing");

        std::string nm = idx.front ()(0).xstring_value ("invalid meta.class indexing, expected a method or property name");

        cdef_method meth = find_method (nm);

        if (meth.ok ())
          {
            if (! meth.is_static ())
              error ("method `%s' is not static", nm.c_str ());

            octave_value_list args;

            if (type.length () > 1 && idx.size () > 1 && type[1] == '(')
              {
                args = *(++(idx.begin ()));
                skip++;
              }

            retval = meth.execute (args, (type.length () > skip
                                          ? 1 : nargout), true,
                                   "meta.class");
          }
        else
          {
            cdef_property prop = find_property (nm);

            if (! prop.ok ())
              error ("no such method or property `%s'", nm.c_str ());

            if (! prop.is_constant ())
              error ("property `%s' is not constant", nm.c_str ());

            retval(0) = prop.get_value (true, "meta.class");
          }
      }
      break;

    default:
      error ("invalid meta.class indexing");
      break;
    }

  if (type.length () > skip && idx.size () > skip && ! retval.empty ())
    retval = retval(0).next_subsref (nargout, type, idx, skip);

  return retval;
}

void
cdef_class::cdef_class_rep::meta_release (void)
{
  cdef_manager& cdm
    = octave::__get_cdef_manager__ ("cdef_class::cdef_class_rep::meta_release");

  cdm.unregister_class (wrap ());
}

void
cdef_class::cdef_class_rep::initialize_object (cdef_object& obj)
{
  // Populate the object with default property values

  std::list<cdef_class> super_classes = lookup_classes (
                                          get ("SuperClasses").cell_value ());

  for (auto& cls : super_classes)
    cls.initialize_object (obj);

  for (const auto& pname_prop : property_map)
    {
      if (! pname_prop.second.get ("Dependent").bool_value ())
        {
          octave_value pvalue = pname_prop.second.get ("DefaultValue");

          if (pvalue.is_defined ())
            obj.put (pname_prop.first, pvalue);
          else
            obj.put (pname_prop.first, octave_value (Matrix ()));
        }
    }

  refcount++;
  obj.mark_for_construction (cdef_class (this));
}

void
cdef_class::cdef_class_rep::run_constructor (cdef_object& obj,
                                             const octave_value_list& args)
{
  octave_value_list empty_args;

  for (const auto& cls : implicit_ctor_list)
    {
      cdef_class supcls = lookup_class (cls);

      supcls.run_constructor (obj, empty_args);
    }

  std::string cls_name = get_name ();
  std::string ctor_name = get_base_name (cls_name);

  cdef_method ctor = find_method (ctor_name);

  if (ctor.ok ())
    {
      octave_value_list ctor_args (args);
      octave_value_list ctor_retval;

      ctor_args.prepend (to_ov (obj));
      ctor_retval = ctor.execute (ctor_args, 1, true, "constructor");

      if (ctor_retval.length () != 1)
        error ("%s: invalid number of output arguments for classdef constructor",
               ctor_name.c_str ());

      obj = to_cdef (ctor_retval(0));
    }

  obj.mark_as_constructed (wrap ());
}

octave_value
cdef_class::cdef_class_rep::construct (const octave_value_list& args)
{
  cdef_object obj = construct_object (args);

  if (obj.ok ())
    return to_ov (obj);

  return octave_value ();
}

cdef_object
cdef_class::cdef_class_rep::construct_object (const octave_value_list& args)
{
  if (is_abstract ())
    error ("cannot instantiate object for abstract class `%s'",
           get_name ().c_str ());

  cdef_object obj;

  if (is_meta_class ())
    {
      // This code path is only used to create empty meta objects
      // as filler for the empty values within a meta object array.

      cdef_class this_cls = wrap ();

      static cdef_object empty_class;

      cdef_manager& cdm
        = octave::__get_cdef_manager__ ("cdef_class::cdef_class_rep::construct_object");

      if (this_cls == cdm.meta_class ())
        {
          if (! empty_class.ok ())
            empty_class = cdm.make_class ("", std::list<cdef_class> ());
          obj = empty_class;
        }
      else if (this_cls == cdm.meta_property ())
        {
          static cdef_property empty_property;

          if (! empty_class.ok ())
            empty_class = cdm.make_class ("", std::list<cdef_class> ());
          if (! empty_property.ok ())
            empty_property = cdm.make_property (empty_class, "");
          obj = empty_property;
        }
      else if (this_cls == cdm.meta_method ())
        {
          static cdef_method empty_method;

          if (! empty_class.ok ())
            empty_class = cdm.make_class ("", std::list<cdef_class> ());
          if (! empty_method.ok ())
            empty_method = cdm.make_method (empty_class, "", octave_value ());
          obj = empty_method;
        }
      else if (this_cls == cdm.meta_package ())
        {
          static cdef_package empty_package;

          if (! empty_package.ok ())
            empty_package = cdm.make_package ("");
          obj = empty_package;
        }
      else
        panic_impossible ();

      return obj;
    }
  else
    {
      if (is_handle_class ())
        obj = cdef_object (new handle_cdef_object ());
      else
        obj = cdef_object (new value_cdef_object ());
      obj.set_class (wrap ());

      initialize_object (obj);

      run_constructor (obj, args);

      return obj;
    }

  return cdef_object ();
}

static octave_value
compute_attribute_value (octave::tree_evaluator& tw,
                         octave::tree_classdef_attribute *t)
{
  octave::tree_expression *expr = t->expression ();

  if (expr)
    {
      if (expr->is_identifier ())
        {
          std::string s = expr->name ();

          if (s == "public")
            return std::string ("public");
          else if (s == "protected")
            return std::string ("protected");
          else if (s == "private")
            return std::string ("private");
        }

      return tw.evaluate (expr);
    }
  else
    return octave_value (true);
}

template <typename T>
static std::string
attribute_value_to_string (T *t, octave_value v)
{
  if (v.is_string ())
    return v.string_value ();
  else if (t->expression ())
    return t->expression ()->original_text ();
  else
    return "true";
}

cdef_class
cdef_class::make_meta_class (octave::interpreter& interp,
                             octave::tree_classdef *t, bool is_at_folder)
{
  cdef_class retval;
  std::string class_name, full_class_name;

  // Class creation

  class_name = full_class_name = t->ident ()->name ();
  if (! t->package_name ().empty ())
    full_class_name = t->package_name () + '.' + full_class_name;

#if DEBUG_TRACE
  std::cerr << "class: " << full_class_name << std::endl;
#endif

  std::list<cdef_class> slist;

  if (t->superclass_list ())
    {
      for (auto& scls : (*t->superclass_list ()))
        {
          std::string sclass_name = (scls)->class_name ();

#if DEBUG_TRACE
          std::cerr << "superclass: " << sclass_name << std::endl;
#endif

          cdef_class sclass = lookup_class (sclass_name);

          if (sclass.get ("Sealed").bool_value ())
            error ("`%s' cannot inherit from `%s', because it is sealed",
                   full_class_name.c_str (), sclass_name.c_str ());

          slist.push_back (sclass);
        }
    }

  cdef_manager& cdm
    = octave::__get_cdef_manager__ ("cdef_class::make_meta_class");

  retval = cdm.make_class (full_class_name, slist);

  // Package owning this class

  if (! t->package_name ().empty ())
    {
      cdef_package pack = cdm.find_package (t->package_name ());

      if (pack.ok ())
        retval.put ("ContainingPackage", to_ov (pack));
    }

  // Class attributes

  octave::tree_evaluator& tw = interp.get_evaluator ();

  if (t->attribute_list ())
    {
      for (const auto& attr : (*t->attribute_list ()))
        {
          std::string aname = attr->ident ()->name ();
          octave_value avalue = compute_attribute_value (tw, attr);

#if DEBUG_TRACE
          std::cerr << "class attribute: " << aname << " = "
                    << attribute_value_to_string (attr, avalue) << std::endl;
#endif

          retval.put (aname, avalue);
        }
    }

  octave::tree_classdef_body *b = t->body ();

  if (b)
    {
      // Keep track of the get/set accessor methods.  They will be used
      // later on when creating properties.

      std::map<std::string, octave_value> get_methods;
      std::map<std::string, octave_value> set_methods;

      // Method blocks

      std::list<octave::tree_classdef_methods_block *> mb_list = b->methods_list ();

      octave::load_path& lp = interp.get_load_path ();

      for (auto& mb_p : mb_list)
        {
          std::map<std::string, octave_value> amap;

#if DEBUG_TRACE
          std::cerr << "method block" << std::endl;
#endif

          // Method attributes

          if (mb_p->attribute_list ())
            {
              for (auto& attr_p : *mb_p->attribute_list ())
                {
                  std::string aname = attr_p->ident ()->name ();
                  octave_value avalue = compute_attribute_value (tw, attr_p);

#if DEBUG_TRACE
                  std::cerr << "method attribute: " << aname << " = "
                            << attribute_value_to_string (attr_p, avalue)
                            << std::endl;
#endif

                  amap[aname] = avalue;
                }
            }

          // Methods

          if (mb_p->element_list ())
            {
              for (auto& mtd : *mb_p->element_list ())
                {
                  std::string mname = mtd.function_value ()->name ();
                  std::string mprefix = mname.substr (0, 4);

                  if (mprefix == "get.")
                    get_methods[mname.substr (4)] =
                      make_fcn_handle (mtd, full_class_name + '>' + mname);
                  else if (mprefix == "set.")
                    set_methods[mname.substr (4)] =
                      make_fcn_handle (mtd, full_class_name + '>' + mname);
                  else
                    {
                      cdef_method meth = cdm.make_method (retval, mname, mtd);

#if DEBUG_TRACE
                      std::cerr << (mname == class_name ? "constructor"
                                                        : "method")
                                << ": " << mname << std::endl;
#endif

                      for (auto& attrnm_val : amap)
                        meth.put (attrnm_val.first, attrnm_val.second);

                      retval.install_method (meth);
                    }
                }
            }
        }

      if (is_at_folder)
        {
          // Look for all external methods visible on octave path at the
          // time of loading of the class.
          //
          // FIXME: This is an "extension" to Matlab behavior, which only looks
          // in the @-folder containing the original classdef file.  However,
          // this is easier to implement it that way at the moment.

          std::list<std::string> external_methods
            = lp.methods (full_class_name);

          for (const auto& mtdnm : external_methods)
            {
              // FIXME: should we issue a warning if the method is already
              // defined in the classdef file?

              if (mtdnm != class_name
                  && ! retval.find_method (mtdnm, true).ok ())
                {
                  // Create a dummy method that is used until the actual
                  // method is loaded.
                  octave_user_function *fcn = new octave_user_function ();

                  fcn->stash_function_name (mtdnm);

                  cdef_method meth
                    = cdm.make_method (retval, mtdnm, octave_value (fcn));

                  retval.install_method (meth);
                }
            }
        }

      // Property blocks

      // FIXME: default property expression should be able to call static
      //        methods of the class being constructed.  A restricted CLASSNAME
      //        symbol should be added to the scope before evaluating default
      //        value expressions.

      std::list<octave::tree_classdef_properties_block *> pb_list
        = b->properties_list ();

      for (auto& pb_p : pb_list)
        {
          std::map<std::string, octave_value> amap;

#if DEBUG_TRACE
          std::cerr << "property block" << std::endl;
#endif

          // Property attributes

          if (pb_p->attribute_list ())
            {
              for (auto& attr_p : *pb_p->attribute_list ())
                {
                  std::string aname = attr_p->ident ()->name ();
                  octave_value avalue = compute_attribute_value (tw, attr_p);

#if DEBUG_TRACE
                  std::cerr << "property attribute: " << aname << " = "
                            << attribute_value_to_string (attr_p, avalue)
                            << std::endl;
#endif

                  if (aname == "Access")
                    {
                      amap["GetAccess"] = avalue;
                      amap["SetAccess"] = avalue;
                    }
                  else
                    amap[aname] = avalue;
                }
            }

          // Properties

          if (pb_p->element_list ())
            {
              for (auto& prop_p : *pb_p->element_list ())
                {
                  std::string prop_name = prop_p->ident ()->name ();

                  cdef_property prop = cdm.make_property (retval, prop_name);

#if DEBUG_TRACE
                  std::cerr << "property: " << prop_p->ident ()->name ()
                            << std::endl;
#endif

                  octave::tree_expression *expr = prop_p->expression ();
                  if (expr)
                    {
                      octave_value pvalue = tw.evaluate (expr);

#if DEBUG_TRACE
                      std::cerr << "property default: "
                                << attribute_value_to_string (*pit, pvalue)
                                << std::endl;
#endif

                      prop.put ("DefaultValue", pvalue);
                    }

                  // Install property attributes.  This is done before assigning
                  // the property accessors so we can do validation by using
                  // cdef_property methods.

                  for (auto& attrnm_val : amap)
                    prop.put (attrnm_val.first, attrnm_val.second);

                  // Install property access methods, if any.  Remove the
                  // accessor methods from the temporary storage map, so we can
                  // detect which ones are invalid and do not correspond to a
                  // defined property.

                  auto git = get_methods.find (prop_name);

                  if (git != get_methods.end ())
                    {
                      make_function_of_class (retval, git->second);
                      prop.put ("GetMethod", git->second);
                      get_methods.erase (git);
                    }

                  auto sit = set_methods.find (prop_name);

                  if (sit != set_methods.end ())
                    {
                      make_function_of_class (retval, sit->second);
                      prop.put ("SetMethod", sit->second);
                      set_methods.erase (sit);
                    }

                  retval.install_property (prop);
                }
            }
        }
    }

  return retval;
}

octave_function*
cdef_class::get_method_function (const std::string& /* nm */)
{
  octave_classdef_meta *p = new octave_classdef_meta (*this);

  return p;
}
