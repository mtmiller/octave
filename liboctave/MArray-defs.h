/*

Copyright (C) 1996, 1999, 2000, 2003, 2005, 2006, 2007 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_MArray_defs_h)
#define octave_MArray_defs_h 1

// Nothing like a little CPP abuse to brighten everyone's day.

#define DO_VS_OP(r, l, v, OP, s) \
  if (l > 0) \
    { \
      for (octave_idx_type i = 0; i < l; i++) \
	r[i] = v[i] OP s; \
    }

#define DO_SV_OP(r, l, s, OP, v) \
  if (l > 0) \
    { \
      for (octave_idx_type i = 0; i < l; i++) \
	r[i] = s OP v[i]; \
    }

#define DO_VV_OP(r, l, x, OP, y) \
  if (l > 0) \
    { \
      for (octave_idx_type i = 0; i < l; i++) \
	r[i] = x[i] OP y[i]; \
    }

#define NEG_V(r, l, x) \
  if (l > 0) \
    { \
      for (octave_idx_type i = 0; i < l; i++) \
	r[i] = -x[i]; \
    }

#define DO_VS_OP2(T, a, OP, s) \
  octave_idx_type l = a.length (); \
  if (l > 0) \
    { \
      T *tmp = a.fortran_vec (); \
      for (octave_idx_type i = 0; i < l; i++) \
	tmp[i] OP s; \
    }

#define DO_VV_OP2(T, a, OP, b) \
  do \
    { \
      T *a_tmp = a.fortran_vec (); \
      const T *b_tmp = b.data (); \
      for (octave_idx_type i = 0; i < l; i++) \
	a_tmp[i] OP b_tmp[i]; \
    } \
  while (0)

// A macro that can be used to declare and instantiate OP= operators.
#define MARRAY_OP_ASSIGN_DECL(A_T, E_T, OP, PFX, API, LTGT, RHS_T) \
  PFX API A_T<E_T>& \
  operator OP LTGT (A_T<E_T>&, const RHS_T&)

// All the OP= operators that we care about.
#define MARRAY_OP_ASSIGN_DECLS(A_T, E_T, PFX, API, LTGT, RHS_T) \
  MARRAY_OP_ASSIGN_DECL (A_T, E_T, +=, PFX, API, LTGT, RHS_T); \
  MARRAY_OP_ASSIGN_DECL (A_T, E_T, -=, PFX, API, LTGT, RHS_T);

// Generate forward declarations for OP= operators.
#define MARRAY_OP_ASSIGN_FWD_DECLS(A_T, RHS_T, API) \
  MARRAY_OP_ASSIGN_DECLS (A_T, T, template <typename T>, API, , RHS_T)

// Generate friend declarations for the OP= operators.
#define MARRAY_OP_ASSIGN_FRIENDS(A_T, RHS_T, API) \
  MARRAY_OP_ASSIGN_DECLS (A_T, T, friend, API, <>, RHS_T)

// Instantiate the OP= operators.
#define MARRAY_OP_ASSIGN_DEFS(A_T, E_T, RHS_T, API) \
  MARRAY_OP_ASSIGN_DECLS (A_T, E_T, template, API, , RHS_T)

// A function that can be used to forward OP= operations from derived
// classes back to us.
#define MARRAY_OP_ASSIGN_FWD_FCN(R, F, T, C_X, X_T, C_Y, Y_T) \
  inline R \
  F (X_T& x, const Y_T& y) \
  { \
    return R (F (C_X (x), C_Y (y))); \
  }

// All the OP= operators that we care about forwarding.
#define MARRAY_OP_ASSIGN_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_OP_ASSIGN_FWD_FCN (R, operator +=, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_OP_ASSIGN_FWD_FCN (R, operator -=, T, C_X, X_T, C_Y, Y_T)

// A macro that can be used to declare and instantiate unary operators.
#define MARRAY_UNOP(A_T, E_T, F, PFX, API, LTGT) \
  PFX API A_T<E_T> \
  F LTGT (const A_T<E_T>&)

// All the unary operators that we care about.
#define MARRAY_UNOP_DECLS(A_T, E_T, PFX, API, LTGT) \
  MARRAY_UNOP (A_T, E_T, operator +, PFX, API, LTGT); \
  MARRAY_UNOP (A_T, E_T, operator -, PFX, API, LTGT);

// Generate forward declarations for unary operators.
#define MARRAY_UNOP_FWD_DECLS(A_T, API) \
  MARRAY_UNOP_DECLS (A_T, T, template <typename T>, API, )

// Generate friend declarations for the unary operators.
#define MARRAY_UNOP_FRIENDS(A_T, API) \
  MARRAY_UNOP_DECLS (A_T, T, friend, API, <>)

// Instantiate the unary operators.
#define MARRAY_UNOP_DEFS(A_T, E_T, API) \
  MARRAY_UNOP_DECLS (A_T, E_T, template, API, )

// A function that can be used to forward unary operations from derived
// classes back to us.
#define MARRAY_UNOP_FWD_FCN(R, F, T, C_X, X_T) \
  inline R \
  F (const X_T& x) \
  { \
    return R (F (C_X (x))); \
  }

// All the unary operators that we care about forwarding.
#define MARRAY_UNOP_FWD_DEFS(R, T, C_X, X_T) \
  MARRAY_UNOP_FWD_FCN (R, operator +, T, C_X, X_T) \
  MARRAY_UNOP_FWD_FCN (R, operator -, T, C_X, X_T)

// A macro that can be used to declare and instantiate binary operators.
#define MARRAY_BINOP_DECL(A_T, E_T, F, PFX, API, LTGT, X_T, Y_T) \
  PFX API A_T<E_T> \
  F LTGT (const X_T&, const Y_T&)

// All the binary operators that we care about.  We have two
// sets of macros since the MArray OP MArray operations use functions
// (product and quotient) instead of operators (*, /).
#define MARRAY_BINOP_DECLS(A_T, E_T, PFX, API, LTGT, X_T, Y_T) \
  MARRAY_BINOP_DECL (A_T, E_T, operator +, PFX, API, LTGT, X_T, Y_T); \
  MARRAY_BINOP_DECL (A_T, E_T, operator -, PFX, API, LTGT, X_T, Y_T); \
  MARRAY_BINOP_DECL (A_T, E_T, operator *, PFX, API, LTGT, X_T, Y_T); \
  MARRAY_BINOP_DECL (A_T, E_T, operator /, PFX, API, LTGT, X_T, Y_T);

#define MARRAY_AA_BINOP_DECLS(A_T, E_T, PFX, API, LTGT) \
  MARRAY_BINOP_DECL (A_T, E_T, operator +, PFX, API, LTGT, A_T<E_T>, A_T<E_T>); \
  MARRAY_BINOP_DECL (A_T, E_T, operator -, PFX, API, LTGT, A_T<E_T>, A_T<E_T>); \
  MARRAY_BINOP_DECL (A_T, E_T, quotient,   PFX, API, LTGT, A_T<E_T>, A_T<E_T>); \
  MARRAY_BINOP_DECL (A_T, E_T, product,    PFX, API, LTGT, A_T<E_T>, A_T<E_T>);

#define MDIAGARRAY2_DAS_BINOP_DECLS(A_T, E_T, PFX, API, LTGT, X_T, Y_T) \
  MARRAY_BINOP_DECL (A_T, E_T, operator *, PFX, API, LTGT, X_T, Y_T); \
  MARRAY_BINOP_DECL (A_T, E_T, operator /, PFX, API, LTGT, X_T, Y_T);

#define MDIAGARRAY2_SDA_BINOP_DECLS(A_T, E_T, PFX, API, LTGT, X_T, Y_T) \
  MARRAY_BINOP_DECL (A_T, E_T, operator *, PFX, API, LTGT, X_T, Y_T);

#define MDIAGARRAY2_DADA_BINOP_DECLS(A_T, E_T, PFX, API, LTGT) \
  MARRAY_BINOP_DECL (A_T, E_T, operator +, PFX, API, LTGT, A_T<E_T>, A_T<E_T>); \
  MARRAY_BINOP_DECL (A_T, E_T, operator -, PFX, API, LTGT, A_T<E_T>, A_T<E_T>); \
  MARRAY_BINOP_DECL (A_T, E_T, product,    PFX, API, LTGT, A_T<E_T>, A_T<E_T>);

// Generate forward declarations for binary operators.
#define MARRAY_BINOP_FWD_DECLS(A_T, API) \
  MARRAY_BINOP_DECLS (A_T, T, template <typename T>, API, , A_T<T>, T) \
  MARRAY_BINOP_DECLS (A_T, T, template <typename T>, API, , T, A_T<T>) \
  MARRAY_AA_BINOP_DECLS (A_T, T, template <typename T>, API, )

#define MDIAGARRAY2_BINOP_FWD_DECLS(A_T, API) \
  MDIAGARRAY2_DAS_BINOP_DECLS (A_T, T, template <typename T>, API, , A_T<T>, T) \
  MDIAGARRAY2_SDA_BINOP_DECLS (A_T, T, template <typename T>, API, , T, A_T<T>) \
  MDIAGARRAY2_DADA_BINOP_DECLS (A_T, T, template <typename T>, API, )

// Generate friend declarations for the binary operators.
#define MARRAY_BINOP_FRIENDS(A_T, API) \
  MARRAY_BINOP_DECLS (A_T, T, friend, API, <>, A_T<T>, T) \
  MARRAY_BINOP_DECLS (A_T, T, friend, API, <>, T, A_T<T>) \
  MARRAY_AA_BINOP_DECLS (A_T, T, friend, API, <>)

#define MDIAGARRAY2_BINOP_FRIENDS(A_T, API) \
  MDIAGARRAY2_DAS_BINOP_DECLS (A_T, T, friend, API, <>, A_T<T>, T) \
  MDIAGARRAY2_SDA_BINOP_DECLS (A_T, T, friend, API, <>, T, A_T<T>) \
  MDIAGARRAY2_DADA_BINOP_DECLS (A_T, T, friend, API, <>)

// Instantiate the binary operators.
#define MARRAY_BINOP_DEFS(A_T, E_T, API) \
  MARRAY_BINOP_DECLS (A_T, E_T, template, API, , A_T<E_T>, E_T) \
  MARRAY_BINOP_DECLS (A_T, E_T, template, API, , E_T, A_T<E_T>) \
  MARRAY_AA_BINOP_DECLS (A_T, E_T, template, API, )

#define MDIAGARRAY2_BINOP_DEFS(A_T, E_T, API) \
  MDIAGARRAY2_DAS_BINOP_DECLS (A_T, E_T, template, API, , A_T<E_T>, E_T) \
  MDIAGARRAY2_SDA_BINOP_DECLS (A_T, E_T, template, API, , E_T, A_T<E_T>) \
  MDIAGARRAY2_DADA_BINOP_DECLS (A_T, E_T, template, API, )

// A function that can be used to forward binary operations from derived
// classes back to us.
#define MARRAY_BINOP_FWD_FCN(R, F, T, C_X, X_T, C_Y, Y_T) \
  inline R \
  F (const X_T& x, const Y_T& y) \
  { \
    return R (F (C_X (x), C_Y (y))); \
  }

// The binary operators that we care about forwarding.  We have two
// sets of macros since the MArray OP MArray operations use functions
// (product and quotient) instead of operators (*, /).
#define MARRAY_BINOP_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator +, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator -, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator *, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator /, T, C_X, X_T, C_Y, Y_T)

#define MARRAY_AA_BINOP_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator +, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator -, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, product,    T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, quotient,   T, C_X, X_T, C_Y, Y_T)

#define MDIAGARRAY2_DAS_BINOP_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator *, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator /, T, C_X, X_T, C_Y, Y_T)

#define MDIAGARRAY2_SDA_BINOP_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator *, T, C_X, X_T, C_Y, Y_T)

#define MDIAGARRAY2_DADA_BINOP_FWD_DEFS(R, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator +, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, operator -, T, C_X, X_T, C_Y, Y_T) \
  MARRAY_BINOP_FWD_FCN (R, product,    T, C_X, X_T, C_Y, Y_T)

// Forward declarations for the MArray operators.
#define MARRAY_OPS_FORWARD_DECLS(A_T, API) \
  template <class T> \
  class A_T; \
 \
  MARRAY_OP_ASSIGN_FWD_DECLS (A_T, T, API) \
  MARRAY_OP_ASSIGN_FWD_DECLS (A_T, A_T<T>, API) \
  MARRAY_UNOP_FWD_DECLS (A_T, API) \
  MARRAY_BINOP_FWD_DECLS (A_T, API)

#define MDIAGARRAY2_OPS_FORWARD_DECLS(A_T, API) \
  template <class T> \
  class A_T; \
 \
  MARRAY_OP_ASSIGN_FWD_DECLS (A_T, A_T<T>, API) \
  MARRAY_UNOP_FWD_DECLS (A_T, API) \
  MDIAGARRAY2_BINOP_FWD_DECLS (A_T, API)

// Friend declarations for the MArray operators.
#define MARRAY_OPS_FRIEND_DECLS(A_T, API) \
  MARRAY_OP_ASSIGN_FRIENDS (A_T, T, API) \
  MARRAY_OP_ASSIGN_FRIENDS (A_T, A_T<T>, API) \
  MARRAY_UNOP_FRIENDS (A_T, API) \
  MARRAY_BINOP_FRIENDS (A_T, API)

#define MDIAGARRAY2_OPS_FRIEND_DECLS(A_T, API) \
  MARRAY_OP_ASSIGN_FRIENDS (A_T, A_T<T>, API) \
  MARRAY_UNOP_FRIENDS (A_T, API) \
  MDIAGARRAY2_BINOP_FRIENDS (A_T, API)

// The following macros are for external use.

// Instantiate all the MArray friends for MArray element type T.
#define INSTANTIATE_MARRAY_FRIENDS(T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArray, T, T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArray, T, MArray<T>, API) \
  MARRAY_UNOP_DEFS (MArray, T, API) \
  MARRAY_BINOP_DEFS (MArray, T, API)

// Instantiate all the MArray2 friends for MArray2 element type T.
#define INSTANTIATE_MARRAY2_FRIENDS(T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArray2, T, T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArray2, T, MArray2<T>, API) \
  MARRAY_UNOP_DEFS (MArray2, T, API) \
  MARRAY_BINOP_DEFS (MArray2, T, API)

// Instantiate all the MArrayN friends for MArrayN element type T.
#define INSTANTIATE_MARRAYN_FRIENDS(T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArrayN, T, T, API) \
  MARRAY_OP_ASSIGN_DEFS (MArrayN, T, MArrayN<T>, API) \
  MARRAY_UNOP_DEFS (MArrayN, T, API) \
  MARRAY_BINOP_DEFS (MArrayN, T, API)

// Instantiate all the MDiagArray2 friends for MDiagArray2 element type T.
#define INSTANTIATE_MDIAGARRAY2_FRIENDS(T, API) \
  MARRAY_OP_ASSIGN_DEFS (MDiagArray2, T, MDiagArray2<T>, API) \
  MARRAY_UNOP_DEFS (MDiagArray2, T, API) \
  MDIAGARRAY2_BINOP_DEFS (MDiagArray2, T, API)

// Define all the MArray forwarding functions for return type R and
// MArray element type T
#define MARRAY_FORWARD_DEFS(B, R, T) \
  MARRAY_OP_ASSIGN_FWD_DEFS \
    (R, T, dynamic_cast<B<T>&>, R, , T) \
 \
  MARRAY_OP_ASSIGN_FWD_DEFS \
    (R, T, \
     dynamic_cast<B<T>&>, R, dynamic_cast<const B<T>&>, R) \
 \
  MARRAY_UNOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R) \
 \
  MARRAY_BINOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R, , T) \
 \
  MARRAY_BINOP_FWD_DEFS \
    (R, T, , T, dynamic_cast<const B<T>&>, R) \
 \
  MARRAY_AA_BINOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R, dynamic_cast<const B<T>&>, R)

#define MDIAGARRAY2_FORWARD_DEFS(B, R, T) \
  MARRAY_OP_ASSIGN_FWD_DEFS \
    (R, T, \
     dynamic_cast<B<T>&>, R, dynamic_cast<const B<T>&>, R) \
 \
  MARRAY_UNOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R) \
 \
  MDIAGARRAY2_DAS_BINOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R, , T) \
 \
  MDIAGARRAY2_SDA_BINOP_FWD_DEFS \
    (R, T, , T, dynamic_cast<const B<T>&>, R) \
 \
  MDIAGARRAY2_DADA_BINOP_FWD_DEFS \
    (R, T, dynamic_cast<const B<T>&>, R, dynamic_cast<const B<T>&>, R)

#define MARRAY_NORM_BODY(TYPE, blas_norm, BLAS_NORM)	\
 \
  double retval = octave_NaN; \
 \
  octave_idx_type len = length (); \
 \
  if (len > 0) \
    { \
      const TYPE *d = data (); \
 \
      if (p == -1) \
	{ \
	  /* Frobenius norm.  */ \
	  retval = 0; \
 \
          /* precondition */ \
          double inf_norm = 0.; \
	  for (octave_idx_type i = 0; i < len; i++) \
	    { \
              double d_abs = std::abs (d[i]); \
              if (d_abs > inf_norm) \
                inf_norm = d_abs; \
            } \
          inf_norm = (inf_norm == octave_Inf || inf_norm == 0. ? 1.0 : \
		      inf_norm); \
          double scale = 1. / inf_norm; \
\
	  for (octave_idx_type i = 0; i < len; i++) \
	    { \
	      double d_abs = std::abs (d[i]) * scale; \
	      retval += d_abs * d_abs; \
	    } \
 \
	  retval = ::sqrt (retval) * inf_norm; \
	} \
      else if (p == 2) \
	F77_FCN (blas_norm, BLAS_NORM) (len, d, 1, retval); \
      else if (xisinf (p)) \
	{ \
	  octave_idx_type i = 0; \
 \
	  while (i < len && xisnan (d[i])) \
	    i++; \
 \
	  if (i < len) \
	    retval = std::abs (d[i]); \
 \
	  if (p > 0) \
	    { \
	      while (i < len) \
		{ \
		  double d_abs = std::abs (d[i++]); \
 \
		  if (d_abs > retval) \
		    retval = d_abs; \
		} \
	    } \
	  else \
	    { \
	      while (i < len) \
		{ \
		  double d_abs = std::abs (d[i++]); \
 \
		  if (d_abs < retval) \
		    retval = d_abs; \
		} \
	    } \
	} \
      else \
	{ \
	  retval = 0; \
 \
	  for (octave_idx_type i = 0; i < len; i++) \
	    { \
	      double d_abs = std::abs (d[i]); \
	      retval += pow (d_abs, p); \
	    } \
 \
	  retval = pow (retval, 1/p); \
	} \
    } \
 \
  return retval

// Now we have all the definitions we need.

#endif
