/*

Copyright (C) 1996, 1997 John W. Eaton

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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_ops_h)
#define octave_ops_h 1

#include "Array-util.h"

extern void install_ops (void);

#define INSTALL_UNOP(op, t, f) \
  octave_value_typeinfo::register_unary_op \
    (octave_value::op, t::static_type_id (), oct_unop_ ## f);

#define INSTALL_NCUNOP(op, t, f) \
  octave_value_typeinfo::register_non_const_unary_op \
    (octave_value::op, t::static_type_id (), oct_unop_ ## f);

#define INSTALL_BINOP(op, t1, t2, f) \
  octave_value_typeinfo::register_binary_op \
    (octave_value::op, t1::static_type_id (), t2::static_type_id (), \
     oct_binop_ ## f);

#define INSTALL_CATOP(t1, t2, f) \
  octave_value_typeinfo::register_cat_op \
    (t1::static_type_id (), t2::static_type_id (), oct_catop_ ## f);

#define INSTALL_ASSIGNOP(op, t1, t2, f) \
  octave_value_typeinfo::register_assign_op \
    (octave_value::op, t1::static_type_id (), t2::static_type_id (), \
     oct_assignop_ ## f);

#define INSTALL_ASSIGNANYOP(op, t1, f) \
  octave_value_typeinfo::register_assignany_op \
    (octave_value::op, t1::static_type_id (), oct_assignop_ ## f);

#define INSTALL_ASSIGNCONV(t1, t2, tr) \
  octave_value_typeinfo::register_pref_assign_conv \
    (t1::static_type_id (), t2::static_type_id (), tr::static_type_id ());

#define INSTALL_CONVOP(t1, t2, f) \
  octave_value_typeinfo::register_type_conv_op \
    (t1::static_type_id (), t2::static_type_id (), oct_conv_ ## f);

#define INSTALL_WIDENOP(t1, t2, f) \
  octave_value_typeinfo::register_widening_op \
    (t1::static_type_id (), t2::static_type_id (), oct_conv_ ## f);

#define BOOL_OP1(xt, xn, get_x, yt, yn, get_y) \
  xt xn = get_x; \
  yt yn = get_y;

#define BOOL_OP2(x) \
  octave_idx_type nr = x.rows (); \
  octave_idx_type nc = x.columns ();

#define BOOL_OP3(test) \
  boolMatrix retval (nr, nc); \
  for (octave_idx_type j = 0; j < nc; j++) \
    for (octave_idx_type i = 0; i < nr; i++) \
      retval (i, j) = test; \
  return retval;

#define SC_MX_BOOL_OP(st, sn, get_s, mt, mn, get_m, test, empty_result) \
  do \
    { \
      BOOL_OP1 (st, sn, get_s, mt, mn, get_m) \
      BOOL_OP2 (mn) \
      if (nr == 0 || nc == 0) \
        return empty_result; \
      BOOL_OP3 (test) \
    } \
  while (0)

#define MX_SC_BOOL_OP(mt, mn, get_m, st, sn, get_s, test, empty_result) \
  do \
    { \
      BOOL_OP1 (mt, mn, get_m, st, sn, get_s) \
      BOOL_OP2 (mn) \
      if (nr == 0 || nc == 0) \
        return empty_result; \
      BOOL_OP3 (test) \
    } \
  while (0)

#define MX_MX_BOOL_OP(m1t, m1n, get_m1, m2t, m2n, get_m2, test, op, \
		      one_empty_result, two_empty_result) \
  do \
    { \
      BOOL_OP1 (m1t, m1n, get_m1, m2t, m2n, get_m2) \
      octave_idx_type m1_nr = m1n.rows (); \
      octave_idx_type m1_nc = m1n.cols (); \
      octave_idx_type m2_nr = m2n.rows (); \
      octave_idx_type m2_nc = m2n.cols (); \
      if (m1_nr == m2_nr && m1_nc == m2_nc) \
	{ \
	  if (m1_nr == 0 && m1_nc == 0) \
	    return two_empty_result; \
	  else \
	    { \
	      BOOL_OP2 (m1n) \
	      BOOL_OP3 (test) \
	    } \
	} \
      else \
	{ \
	  if ((m1_nr == 0 && m1_nc == 0) || (m2_nr == 0 && m2_nc == 0)) \
	    return one_empty_result; \
	  else \
	    { \
	      gripe_nonconformant ("operator " op, m1_nr, m1_nc, \
				   m2_nr, m2_nc); \
	      return boolMatrix (); \
	    } \
	} \
    } \
  while (0)

#define CAST_UNOP_ARG(t) \
  t v = dynamic_cast<t> (a)

#define CAST_BINOP_ARGS(t1, t2) \
  t1 v1 = dynamic_cast<t1> (a1);		\
  t2 v2 = dynamic_cast<t2> (a2)

#define CAST_CONV_ARG(t) \
  t v = dynamic_cast<t> (a)

#define ASSIGNOPDECL(name) \
  static octave_value \
  oct_assignop_ ## name (octave_base_value& a1, \
			 const octave_value_list& idx, \
			 const octave_base_value& a2)

#define ASSIGNANYOPDECL(name) \
  static octave_value \
  oct_assignop_ ## name (octave_base_value& a1, \
			 const octave_value_list& idx, \
			 const octave_value& a2)

#define DEFASSIGNOP(name, t1, t2) \
  ASSIGNOPDECL (name)

#define DEFASSIGNOP_FN(name, t1, t2, f) \
  ASSIGNOPDECL (name) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
 \
    v1.f (idx, v2.t1 ## _value ()); \
    return octave_value (); \
  }

#define DEFNDASSIGNOP_FN(name, t1, t2, e, f) \
  ASSIGNOPDECL (name) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
 \
    v1.f (idx, v2.e ## _value ()); \
    return octave_value (); \
  }

#define DEFASSIGNANYOP_FN(name, t1, f) \
  ASSIGNANYOPDECL (name) \
  { \
    octave_ ## t1& v1 = dynamic_cast<octave_ ## t1&> (a1); \
 \
    v1.f (idx, a2); \
    return octave_value (); \
  }

#define CONVDECL(name) \
  static octave_base_value * \
  oct_conv_ ## name (const octave_base_value& a)

#define CONVDECLX(name) \
  static octave_base_value * \
  oct_conv_ ## name (const octave_base_value&)

#define DEFCONV(name, a_dummy, b_dummy) \
  CONVDECL (name)

#define DEFCONVFNX(name, tfrom, ovtto, tto, e) \
  CONVDECL (name) \
  { \
    CAST_CONV_ARG (const octave_ ## tfrom&); \
 \
    return new octave_ ## ovtto (tto ## NDArray (v.e ## array_value ())); \
  }

#define DEFDBLCONVFN(name, ovtfrom, e) \
  CONVDECL (name) \
  { \
    CAST_CONV_ARG (const octave_ ## ovtfrom&); \
 \
    return new octave_matrix (NDArray (v.e ## _value ())); \
  }

#define DEFSTRINTCONVFN(name, tto) \
  DEFCONVFNX(name, char_matrix_str, tto ## _matrix, tto, char_)

#define DEFSTRDBLCONVFN(name) \
  DEFCONVFNX(name, char_matrix_str, matrix, , char_)

#define DEFCONVFN(name, tfrom, tto) \
  DEFCONVFNX (name, tfrom, tto ## _matrix, tto, )

#define DEFCONVFN2(name, tfrom, sm, tto) \
  DEFCONVFNX (name, tfrom ## _ ## sm, tto ## _matrix, tto, tfrom ## _)

#define UNOPDECL(name, a) \
  static octave_value \
  oct_unop_ ## name (const octave_base_value& a)

#define DEFUNOPX(name, t) \
  UNOPDECL (name, , )

#define DEFUNOP(name, t) \
  UNOPDECL (name, a)

#define DEFUNOP_OP(name, t, op) \
  UNOPDECL (name, a) \
  { \
    CAST_UNOP_ARG (const octave_ ## t&); \
    return octave_value (op v.t ## _value ()); \
  }

#define DEFNDUNOP_OP(name, t, e, op) \
  UNOPDECL (name, a) \
  { \
    CAST_UNOP_ARG (const octave_ ## t&); \
    return octave_value (op v.e ## _value ()); \
  }

// FIXME -- in some cases, the constructor isn't necessary.

#define DEFUNOP_FN(name, t, f) \
  UNOPDECL (name, a) \
  { \
    CAST_UNOP_ARG (const octave_ ## t&); \
    return octave_value (f (v.t ## _value ())); \
  }

#define DEFNDUNOP_FN(name, t, e, f) \
  UNOPDECL (name, a) \
  { \
    CAST_UNOP_ARG (const octave_ ## t&); \
    return octave_value (f (v.e ## _value ())); \
  }

#define DEFNCUNOP_METHOD(name, t, method) \
  static void \
  oct_unop_ ## name (octave_base_value& a) \
  { \
    CAST_UNOP_ARG (octave_ ## t&); \
    v.method (); \
  }

#define BINOPDECL(name, a1, a2) \
  static octave_value \
  oct_binop_ ## name (const octave_base_value& a1, const octave_base_value& a2)

#define DEFBINOPX(name, t1, t2) \
  BINOPDECL (name, , )

#define DEFBINOP(name, t1, t2) \
  BINOPDECL (name, a1, a2)

#define DEFBINOP_OP(name, t1, t2, op) \
  BINOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## t1&, const octave_ ## t2&); \
    return octave_value \
      (v1.t1 ## _value () op v2.t2 ## _value ()); \
  }

#define DEFNDBINOP_OP(name, t1, t2, e1, e2, op) \
  BINOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## t1&, const octave_ ## t2&); \
    return octave_value \
      (v1.e1 ## _value () op v2.e2 ## _value ()); \
  }

// FIXME -- in some cases, the constructor isn't necessary.

#define DEFBINOP_FN(name, t1, t2, f) \
  BINOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## t1&, const octave_ ## t2&); \
    return octave_value (f (v1.t1 ## _value (), v2.t2 ## _value ())); \
  }

#define DEFNDBINOP_FN(name, t1, t2, e1, e2, f) \
  BINOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## t1&, const octave_ ## t2&); \
    return octave_value (f (v1.e1 ## _value (), v2.e2 ## _value ())); \
  }

#define BINOP_NONCONFORMANT(msg) \
  gripe_nonconformant (msg, \
		       a1.rows (), a1.columns (), \
		       a2.rows (), a2.columns ()); \
  return octave_value ()

#define CATOPDECL(name, a1, a2)	\
  static octave_value \
  oct_catop_ ## name (octave_base_value& a1, const octave_base_value& a2, \
		      const Array<int>& ra_idx)

#define DEFCATOPX(name, t1, t2)	\
  CATOPDECL (name, , )

#define DEFCATOP(name, t1, t2)	\
  CATOPDECL (name, a1, a2)

// FIXME -- in some cases, the constructor isn't necessary.

#define DEFCATOP_FN(name, t1, t2, f) \
  CATOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
    return octave_value (v1.t1 ## _value () . f (v2.t2 ## _value (), ra_idx)); \
  }

#define DEFNDCATOP_FN(name, t1, t2, e1, e2, f) \
  CATOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
    return octave_value (v1.e1 ## _value () . f (v2.e2 ## _value (), ra_idx)); \
  }

#define DEFNDCHARCATOP_FN(name, t1, t2, f) \
  CATOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
 \
    return octave_value (v1.char_array_value () . f (v2.char_array_value (), ra_idx), \
			 true, ((a1.is_sq_string () || a2.is_sq_string ()) \
				? '\'' : '"')); \
  }

// For compatibility, the second arg is always converted to the type
// of the first.  Hmm.

#define DEFNDCATOP_FN2(name, t1, t2, tc1, tc2, e1, e2, f) \
  CATOPDECL (name, a1, a2) \
  { \
    CAST_BINOP_ARGS (octave_ ## t1&, const octave_ ## t2&); \
    return octave_value (tc1 (v1.e1 ## _value ()) . f (tc2 (v2.e2 ## _value ()), ra_idx)); \
  }

#define CATOP_NONCONFORMANT(msg) \
  gripe_nonconformant (msg, \
		       a1.rows (), a1.columns (), \
		       a2.rows (), a2.columns ()); \
  return octave_value ()

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
