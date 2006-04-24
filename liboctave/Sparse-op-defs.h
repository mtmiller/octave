/*

Copyright (C) 2004 David Bateman
Copyright (C) 1998-2004 Andy Adler

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#if !defined (octave_sparse_op_defs_h)
#define octave_sparse_op_defs_h 1

#include "Array-util.h"

#define SPARSE_BIN_OP_DECL(R, OP, X, Y) \
  extern R OP (const X&, const Y&)

#define SPARSE_CMP_OP_DECL(OP, X, Y) \
  extern SparseBoolMatrix OP (const X&, const Y&)

#define SPARSE_BOOL_OP_DECL(OP, X, Y) \
  extern SparseBoolMatrix OP (const X&, const Y&)

// matrix by scalar operations.

#define SPARSE_SMS_BIN_OP_DECLS(R1, R2, M, S)  \
  SPARSE_BIN_OP_DECL (R1, operator +, M, S); \
  SPARSE_BIN_OP_DECL (R1, operator -, M, S); \
  SPARSE_BIN_OP_DECL (R2, operator *, M, S); \
  SPARSE_BIN_OP_DECL (R2, operator /, M, S);

#define SPARSE_SMS_BIN_OP_1(R, F, OP, M, S)	\
  R \
  F (const M& m, const S& s) \
  { \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
 \
    R r (nr, nc, (0.0 OP s)); \
 \
    for (octave_idx_type j = 0; j < nc; j++) \
      for (octave_idx_type i = m.cidx (j); i < m.cidx (j+1); i++) \
        r.elem (m.ridx (i), j) = m.data (i) OP s; \
    return r; \
  }

#define SPARSE_SMS_BIN_OP_2(R, F, OP, M, S)	\
  R \
  F (const M& m, const S& s) \
  { \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    octave_idx_type nz = m.nnz (); \
 \
    R r (nr, nc, nz); \
 \
    for (octave_idx_type i = 0; i < nz; i++) \
      { \
	r.data(i) = m.data(i) OP s; \
	r.ridx(i) = m.ridx(i); \
      } \
    for (octave_idx_type i = 0; i < nc + 1; i++) \
      r.cidx(i) = m.cidx(i); \
    \
    r.maybe_compress (true); \
    return r; \
  }

#define SPARSE_SMS_BIN_OPS(R1, R2, M, S) \
  SPARSE_SMS_BIN_OP_1 (R1, operator +, +, M, S) \
  SPARSE_SMS_BIN_OP_1 (R1, operator -, -, M, S) \
  SPARSE_SMS_BIN_OP_2 (R2, operator *, *, M, S) \
  SPARSE_SMS_BIN_OP_2 (R2, operator /, /, M, S)

#define SPARSE_SMS_CMP_OP_DECLS(M, S) \
  SPARSE_CMP_OP_DECL (mx_el_lt, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_le, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_ge, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_gt, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_eq, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M, S);

#define SPARSE_SMS_EQNE_OP_DECLS(M, S) \
  SPARSE_CMP_OP_DECL (mx_el_eq, M, S); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M, S);

#define SPARSE_SMS_CMP_OP(F, OP, M, MZ, MC, S, SZ, SC)	\
  SparseBoolMatrix \
  F (const M& m, const S& s) \
  { \
    /* Count num of non-zero elements */ \
    octave_idx_type nel = 0; \
    octave_idx_type nz = m.nnz (); \
    if (MC (MZ) OP SC (s))   \
      nel += m.numel() - nz; \
    for (octave_idx_type i = 0; i < nz; i++) \
      if (MC (m.data (i)) OP SC (s)) \
        nel++;	\
    \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    SparseBoolMatrix r (nr, nc, nel); \
    \
    if (nr > 0 && nc > 0) \
      { \
	if (MC (MZ) OP SC (s))	\
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = 0; i < nr; i++) \
		  { \
		    bool el =  MC (m.elem(i, j)) OP SC (s); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
	else \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++) \
		  { \
		    bool el =  MC (m.data(i)) OP SC (s); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = m.ridx(i); \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	\
    return r; \
  }

#define SPARSE_SMS_CMP_OPS(M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_lt, <,  M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_le, <=, M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_ge, >=, M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_gt, >,  M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_eq, ==, M, MZ,   , S, SZ,   )	\
  SPARSE_SMS_CMP_OP (mx_el_ne, !=, M, MZ,   , S, SZ,   )

#define SPARSE_SMS_EQNE_OPS(M, MZ, CM, S, SZ, CS)	\
  SPARSE_SMS_CMP_OP (mx_el_eq, ==, M, MZ,   , S, SZ,   )	\
  SPARSE_SMS_CMP_OP (mx_el_ne, !=, M, MZ,   , S, SZ,   )

#define SPARSE_SMS_BOOL_OP_DECLS(M, S) \
  SPARSE_BOOL_OP_DECL (mx_el_and, M, S); \
  SPARSE_BOOL_OP_DECL (mx_el_or,  M, S);

#define SPARSE_SMS_BOOL_OP(F, OP, M, S, LHS_ZERO, RHS_ZERO) \
  SparseBoolMatrix \
  F (const M& m, const S& s) \
  { \
    /* Count num of non-zero elements */ \
    octave_idx_type nel = 0; \
    octave_idx_type nz = m.nnz (); \
    if (LHS_ZERO OP (s != RHS_ZERO)) \
      nel += m.numel() - nz; \
    for (octave_idx_type i = 0; i < nz; i++) \
      if ((m.data(i) != LHS_ZERO) OP (s != RHS_ZERO))\
        nel++;	\
    \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    SparseBoolMatrix r (nr, nc, nel); \
    \
    if (nr > 0 && nc > 0) \
      { \
	if (LHS_ZERO OP (s != RHS_ZERO)) \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = 0; i < nr; i++) \
		  { \
		    bool el = (m.elem(i, j) != LHS_ZERO) OP (s != RHS_ZERO); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
	else \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++) \
		  { \
		    bool el = (m.data(i) != LHS_ZERO) OP (s != RHS_ZERO); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = m.ridx(i); \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	\
    return r; \
  }

#define SPARSE_SMS_BOOL_OPS2(M, S, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMS_BOOL_OP (mx_el_and, &&, M, S, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMS_BOOL_OP (mx_el_or,  ||, M, S, LHS_ZERO, RHS_ZERO)

#define SPARSE_SMS_BOOL_OPS(M, S, ZERO) \
  SPARSE_SMS_BOOL_OPS2(M, S, ZERO, ZERO)

#define SPARSE_SMS_OP_DECLS(R1, R2, M, S) \
  SPARSE_SMS_BIN_OP_DECLS (R1, R2, M, S)	 \
  SPARSE_SMS_CMP_OP_DECLS (M, S) \
  SPARSE_SMS_BOOL_OP_DECLS (M, S)

// scalar by matrix operations.

#define SPARSE_SSM_BIN_OP_DECLS(R1, R2, S, M)    \
  SPARSE_BIN_OP_DECL (R1, operator +, S, M); \
  SPARSE_BIN_OP_DECL (R1, operator -, S, M); \
  SPARSE_BIN_OP_DECL (R2, operator *, S, M); \
  SPARSE_BIN_OP_DECL (R2, operator /, S, M);

#define SPARSE_SSM_BIN_OP_1(R, F, OP, S, M) \
  R \
  F (const S& s, const M& m) \
  { \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
 \
    R r (nr, nc, (s OP 0.0)); \
 \
    for (octave_idx_type j = 0; j < nc; j++) \
      for (octave_idx_type i = m.cidx (j); i < m.cidx (j+1); i++) \
        r.elem (m.ridx (i), j) = s OP m.data (i); \
 \
    return r; \
  }

#define SPARSE_SSM_BIN_OP_2(R, F, OP, S, M) \
  R \
  F (const S& s, const M& m) \
  { \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    octave_idx_type nz = m.nnz (); \
 \
    R r (nr, nc, nz); \
 \
    for (octave_idx_type i = 0; i < nz; i++) \
      { \
	r.data(i) = s OP m.data(i); \
	r.ridx(i) = m.ridx(i); \
      } \
    for (octave_idx_type i = 0; i < nc + 1; i++) \
      r.cidx(i) = m.cidx(i); \
 \
    r.maybe_compress(true); \
    return r; \
  }

#define SPARSE_SSM_BIN_OPS(R1, R2, S, M) \
  SPARSE_SSM_BIN_OP_1 (R1, operator +, +, S, M) \
  SPARSE_SSM_BIN_OP_1 (R1, operator -, -, S, M) \
  SPARSE_SSM_BIN_OP_2 (R2, operator *, *, S, M) \
  SPARSE_SSM_BIN_OP_2 (R2, operator /, /, S, M)

#define SPARSE_SSM_CMP_OP_DECLS(S, M) \
  SPARSE_CMP_OP_DECL (mx_el_lt, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_le, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_ge, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_gt, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_eq, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_ne, S, M);

#define SPARSE_SSM_EQNE_OP_DECLS(S, M) \
  SPARSE_CMP_OP_DECL (mx_el_eq, S, M); \
  SPARSE_CMP_OP_DECL (mx_el_ne, S, M);

#define SPARSE_SSM_CMP_OP(F, OP, S, SZ, SC, M, MZ, MC)	\
  SparseBoolMatrix \
  F (const S& s, const M& m) \
  { \
    /* Count num of non-zero elements */ \
    octave_idx_type nel = 0; \
    octave_idx_type nz = m.nnz (); \
    if (SC (s) OP MC (MZ))   \
      nel += m.numel() - nz; \
    for (octave_idx_type i = 0; i < nz; i++) \
      if (SC (s) OP MC (m.data (i))) \
        nel++;	\
    \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    SparseBoolMatrix r (nr, nc, nel); \
    \
    if (nr > 0 && nc > 0) \
      { \
	if (SC (s) OP MC (MZ))\
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = 0; i < nr; i++) \
		  { \
		    bool el = SC (s) OP MC (m.elem(i, j)); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
	else \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++) \
		  { \
		    bool el =  SC (s) OP MC (m.data(i)); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = m.ridx(i); \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	\
    return r; \
  }

#define SPARSE_SSM_CMP_OPS(S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_lt, <,  S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_le, <=, S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_ge, >=, S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_gt, >,  S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_eq, ==, S, SZ,   , M, MZ,   )	\
  SPARSE_SSM_CMP_OP (mx_el_ne, !=, S, SZ,   , M, MZ,   )

#define SPARSE_SSM_EQNE_OPS(S, SZ, SC, M, MZ, MC)	\
  SPARSE_SSM_CMP_OP (mx_el_eq, ==, S, SZ,   , M, MZ,   )	\
  SPARSE_SSM_CMP_OP (mx_el_ne, !=, S, SZ,   , M, MZ,   )

#define SPARSE_SSM_BOOL_OP_DECLS(S, M) \
  SPARSE_BOOL_OP_DECL (mx_el_and, S, M); \
  SPARSE_BOOL_OP_DECL (mx_el_or,  S, M); \

#define SPARSE_SSM_BOOL_OP(F, OP, S, M, LHS_ZERO, RHS_ZERO) \
  SparseBoolMatrix \
  F (const S& s, const M& m) \
  { \
    /* Count num of non-zero elements */ \
    octave_idx_type nel = 0; \
    octave_idx_type nz = m.nnz (); \
    if ((s != LHS_ZERO) OP  RHS_ZERO) \
      nel += m.numel() - nz; \
    for (octave_idx_type i = 0; i < nz; i++) \
      if ((s != LHS_ZERO) OP m.data(i) != RHS_ZERO) \
        nel++;	\
    \
    octave_idx_type nr = m.rows (); \
    octave_idx_type nc = m.cols (); \
    SparseBoolMatrix r (nr, nc, nel); \
    \
    if (nr > 0 && nc > 0) \
      { \
	if ((s != LHS_ZERO) OP RHS_ZERO) \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = 0; i < nr; i++) \
		  { \
		    bool el = (s != LHS_ZERO) OP (m.elem(i, j) != RHS_ZERO); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
	else \
	  { \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < nc; j++) \
	      { \
		for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++) \
		  { \
		    bool el = (s != LHS_ZERO) OP (m.data(i) != RHS_ZERO); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = m.ridx(i); \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	\
    return r; \
  }

#define SPARSE_SSM_BOOL_OPS2(S, M, LHS_ZERO, RHS_ZERO) \
  SPARSE_SSM_BOOL_OP (mx_el_and, &&, S, M, LHS_ZERO, RHS_ZERO) \
  SPARSE_SSM_BOOL_OP (mx_el_or,  ||, S, M, LHS_ZERO, RHS_ZERO)

#define SPARSE_SSM_BOOL_OPS(S, M, ZERO) \
  SPARSE_SSM_BOOL_OPS2(S, M, ZERO, ZERO)

#define SPARSE_SSM_OP_DECLS(R1, R2, S, M) \
  SPARSE_SSM_BIN_OP_DECLS (R1, R2, S, M)	 \
  SPARSE_SSM_CMP_OP_DECLS (S, M) \
  SPARSE_SSM_BOOL_OP_DECLS (S, M) \

// matrix by matrix operations.

#define SPARSE_SMSM_BIN_OP_DECLS(R1, R2, M1, M2)	\
  SPARSE_BIN_OP_DECL (R1, operator +, M1, M2); \
  SPARSE_BIN_OP_DECL (R1, operator -, M1, M2); \
  SPARSE_BIN_OP_DECL (R2, product,    M1, M2); \
  SPARSE_BIN_OP_DECL (R2, quotient,   M1, M2);

#define SPARSE_SMSM_BIN_OP_1(R, F, OP, M1, M2)	\
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
	r = R (m1_nr, m1_nc, (m1.nnz () + m2.nnz ())); \
        \
        octave_idx_type jx = 0; \
        r.cidx (0) = 0; \
        for (octave_idx_type i = 0 ; i < m1_nc ; i++) \
          { \
            octave_idx_type  ja = m1.cidx(i); \
            octave_idx_type  ja_max = m1.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = m2.cidx(i); \
            octave_idx_type  jb_max = m2.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (m1.ridx(ja) < m2.ridx(jb)))) \
                  { \
                    r.ridx(jx) = m1.ridx(ja); \
                    r.data(jx) = m1.data(ja) OP 0.; \
                    jx++; \
                    ja++; \
                    ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (m2.ridx(jb) < m1.ridx(ja)) ) ) \
                  { \
		    r.ridx(jx) = m2.ridx(jb); \
		    r.data(jx) = 0. OP m2.data(jb); \
		    jx++; \
                    jb++; \
                    jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
		     if ((m1.data(ja) OP m2.data(jb)) != 0.) \
	               { \
                          r.data(jx) = m1.data(ja) OP m2.data(jb); \
                          r.ridx(jx) = m1.ridx(ja); \
                          jx++; \
                       } \
                     ja++; \
                     ja_lt_max= ja < ja_max; \
                     jb++; \
                     jb_lt_max= jb < jb_max; \
                  } \
              } \
            r.cidx(i+1) = jx; \
          } \
        \
	r.maybe_compress (); \
      } \
 \
    return r; \
  }

#define SPARSE_SMSM_BIN_OP_2(R, F, OP, M1, M2)	\
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
        r = R (m1_nr, m1_nc, (m1.nnz () > m2.nnz () ? m1.nnz () : m2.nnz ())); \
        \
        octave_idx_type jx = 0; \
	r.cidx (0) = 0; \
        for (octave_idx_type i = 0 ; i < m1_nc ; i++) \
          { \
            octave_idx_type  ja = m1.cidx(i); \
            octave_idx_type  ja_max = m1.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = m2.cidx(i); \
            octave_idx_type  jb_max = m2.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (m1.ridx(ja) < m2.ridx(jb)))) \
                  { \
                     ja++; ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (m2.ridx(jb) < m1.ridx(ja)) ) ) \
                  { \
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
		     if ((m1.data(ja) OP m2.data(jb)) != 0.) \
	               { \
                          r.data(jx) = m1.data(ja) OP m2.data(jb); \
                          r.ridx(jx) = m1.ridx(ja); \
                          jx++; \
                       } \
                     ja++; ja_lt_max= ja < ja_max; \
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
              } \
            r.cidx(i+1) = jx; \
          } \
        \
	r.maybe_compress (); \
      } \
 \
    return r; \
  }

#define SPARSE_SMSM_BIN_OP_3(R, F, OP, M1, M2)	\
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
 \
        /* FIXME Kludge... Always double/Complex, so Complex () */ \
        r = R (m1_nr, m1_nc, (Complex () OP Complex ())); \
        \
        for (octave_idx_type i = 0 ; i < m1_nc ; i++) \
          { \
            octave_idx_type  ja = m1.cidx(i); \
            octave_idx_type  ja_max = m1.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = m2.cidx(i); \
            octave_idx_type  jb_max = m2.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (m1.ridx(ja) < m2.ridx(jb)))) \
                  { \
		    /* keep those kludges coming */ \
                    r.elem(m1.ridx(ja),i) = m1.data(ja) OP Complex (); \
                    ja++; \
                    ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (m2.ridx(jb) < m1.ridx(ja)) ) ) \
                  { \
		    /* keep those kludges coming */ \
                    r.elem(m2.ridx(jb),i) = Complex () OP m2.data(jb);	\
                    jb++; \
                    jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
                    r.elem(m1.ridx(ja),i) = m1.data(ja) OP m2.data(jb); \
                    ja++; \
                    ja_lt_max= ja < ja_max; \
                    jb++; \
                    jb_lt_max= jb < jb_max; \
                  } \
              } \
          } \
	r.maybe_compress (true); \
      } \
 \
    return r; \
  }

// Note that SM ./ SM needs to take into account the NaN and Inf values
// implied by the division by zero.
// FIXME Are the NaNs double(NaN) or Complex(NaN,Nan) in the complex
// case?
#define SPARSE_SMSM_BIN_OPS(R1, R2, M1, M2)  \
  SPARSE_SMSM_BIN_OP_1 (R1, operator +,  +, M1, M2) \
  SPARSE_SMSM_BIN_OP_1 (R1, operator -,  -, M1, M2) \
  SPARSE_SMSM_BIN_OP_2 (R2, product,     *, M1, M2) \
  SPARSE_SMSM_BIN_OP_3 (R2, quotient,    /, M1, M2)

#define SPARSE_SMSM_CMP_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_lt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_le, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ge, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_gt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_SMSM_EQNE_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_SMSM_CMP_OP(F, OP, M1, C1, M2, C2)	\
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if (C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j))) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j)); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_SMSM_CMP_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_SMSM_CMP_OP (mx_el_lt, <,  M1, C1, M2, C2) \
  SPARSE_SMSM_CMP_OP (mx_el_le, <=, M1, C1, M2, C2) \
  SPARSE_SMSM_CMP_OP (mx_el_ge, >=, M1, C1, M2, C2) \
  SPARSE_SMSM_CMP_OP (mx_el_gt, >,  M1, C1, M2, C2) \
  SPARSE_SMSM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_SMSM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_SMSM_EQNE_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_SMSM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_SMSM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_SMSM_BOOL_OP_DECLS(M1, M2) \
  SPARSE_BOOL_OP_DECL (mx_el_and, M1, M2); \
  SPARSE_BOOL_OP_DECL (mx_el_or,  M1, M2);

#define SPARSE_SMSM_BOOL_OP(F, OP, M1, M2, LHS_ZERO, RHS_ZERO) \
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if ((m1.elem(i, j) != LHS_ZERO) \
		    OP (m2.elem(i, j) != RHS_ZERO)) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = (m1.elem(i, j) != LHS_ZERO) \
		      OP (m2.elem(i, j) != RHS_ZERO);	  \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_SMSM_BOOL_OPS2(M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMSM_BOOL_OP (mx_el_and, &&, M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMSM_BOOL_OP (mx_el_or,  ||, M1, M2, LHS_ZERO, RHS_ZERO) \

#define SPARSE_SMSM_BOOL_OPS(M1, M2, ZERO) \
  SPARSE_SMSM_BOOL_OPS2(M1, M2, ZERO, ZERO)

#define SPARSE_SMSM_OP_DECLS(R1, R2, M1, M2) \
  SPARSE_SMSM_BIN_OP_DECLS (R1, R2, M1, M2) \
  SPARSE_SMSM_CMP_OP_DECLS (M1, M2) \
  SPARSE_SMSM_BOOL_OP_DECLS (M1, M2)

// matrix by matrix operations.

#define SPARSE_MSM_BIN_OP_DECLS(R1, R2, M1, M2)	\
  SPARSE_BIN_OP_DECL (R1, operator +, M1, M2); \
  SPARSE_BIN_OP_DECL (R1, operator -, M1, M2); \
  SPARSE_BIN_OP_DECL (R2, product,    M1, M2); \
  SPARSE_BIN_OP_DECL (R2, quotient,   M1, M2);

#define SPARSE_MSM_BIN_OP_1(R, F, OP, M1, M2)	\
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
        r = R (m1_nr, m1_nc); \
        \
        for (octave_idx_type j = 0; j < m1_nc; j++) \
	  for (octave_idx_type i = 0; i < m1_nr; i++) \
	    r.elem (i, j) = m1.elem (i, j) OP m2.elem (i, j); \
      } \
    return r; \
  }

#define SPARSE_MSM_BIN_OP_2(R, F, OP, M1, M2, ZERO) \
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
	/* Count num of non-zero elements */ \
	octave_idx_type nel = 0; \
	for (octave_idx_type j = 0; j < m1_nc; j++) \
	  for (octave_idx_type i = 0; i < m1_nr; i++) \
	    if ((m1.elem(i, j) OP m2.elem(i, j)) != ZERO) \
	      nel++; \
	\
        r = R (m1_nr, m1_nc, nel); \
        \
	octave_idx_type ii = 0; \
	r.cidx (0) = 0; \
        for (octave_idx_type j = 0 ; j < m1_nc ; j++) \
          { \
	    for (octave_idx_type i = 0 ; i < m1_nr ; i++)	\
	      {	\
	        if ((m1.elem(i, j) OP m2.elem(i, j)) != ZERO) \
		  { \
		    r.data (ii) = m1.elem(i, j) OP m2.elem(i,j); \
		    r.ridx (ii++) = i; \
		  } \
	      } \
	    r.cidx(j+1) = ii; \
	  } \
      } \
 \
    return r; \
  }

// FIXME Pass a specific ZERO value
#define SPARSE_MSM_BIN_OPS(R1, R2, M1, M2) \
  SPARSE_MSM_BIN_OP_1 (R1, operator +,  +, M1, M2) \
  SPARSE_MSM_BIN_OP_1 (R1, operator -,  -, M1, M2) \
  SPARSE_MSM_BIN_OP_2 (R2, product,     *, M1, M2, 0.0) \
  SPARSE_MSM_BIN_OP_2 (R2, quotient,    /, M1, M2, 0.0)

#define SPARSE_MSM_CMP_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_lt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_le, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ge, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_gt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_MSM_EQNE_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_MSM_CMP_OP(F, OP, M1, C1, M2, C2)	\
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if (C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j))) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j)); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_MSM_CMP_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_MSM_CMP_OP (mx_el_lt, <,  M1, C1, M2, C2) \
  SPARSE_MSM_CMP_OP (mx_el_le, <=, M1, C1, M2, C2) \
  SPARSE_MSM_CMP_OP (mx_el_ge, >=, M1, C1, M2, C2) \
  SPARSE_MSM_CMP_OP (mx_el_gt, >,  M1, C1, M2, C2) \
  SPARSE_MSM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_MSM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_MSM_EQNE_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_MSM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_MSM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_MSM_BOOL_OP_DECLS(M1, M2) \
  SPARSE_BOOL_OP_DECL (mx_el_and, M1, M2); \
  SPARSE_BOOL_OP_DECL (mx_el_or,  M1, M2);

#define SPARSE_MSM_BOOL_OP(F, OP, M1, M2, LHS_ZERO, RHS_ZERO) \
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if ((m1.elem(i, j) != LHS_ZERO) \
		    OP (m2.elem(i, j) != RHS_ZERO)) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = (m1.elem(i, j) != LHS_ZERO) \
		      OP (m2.elem(i, j) != RHS_ZERO);	  \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_MSM_BOOL_OPS2(M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_MSM_BOOL_OP (mx_el_and, &&, M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_MSM_BOOL_OP (mx_el_or,  ||, M1, M2, LHS_ZERO, RHS_ZERO) \

#define SPARSE_MSM_BOOL_OPS(M1, M2, ZERO) \
  SPARSE_MSM_BOOL_OPS2(M1, M2, ZERO, ZERO)

#define SPARSE_MSM_OP_DECLS(R1, R2, M1, M2) \
  SPARSE_MSM_BIN_OP_DECLS (R1, R2, M1, M2) \
  SPARSE_MSM_CMP_OP_DECLS (M1, M2) \
  SPARSE_MSM_BOOL_OP_DECLS (M1, M2)

// matrix by matrix operations.

#define SPARSE_SMM_BIN_OP_DECLS(R1, R2, M1, M2)	\
  SPARSE_BIN_OP_DECL (R1, operator +, M1, M2); \
  SPARSE_BIN_OP_DECL (R1, operator -, M1, M2); \
  SPARSE_BIN_OP_DECL (R2, product,    M1, M2); \
  SPARSE_BIN_OP_DECL (R2, quotient,   M1, M2);

#define SPARSE_SMM_BIN_OP_1(R, F, OP, M1, M2)	\
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
        r = R (m1_nr, m1_nc); \
        \
        for (octave_idx_type j = 0; j < m1_nc; j++) \
	  for (octave_idx_type i = 0; i < m1_nr; i++) \
	    r.elem (i, j) = m1.elem (i, j) OP m2.elem (i, j); \
      } \
    return r; \
  }

#define SPARSE_SMM_BIN_OP_2(R, F, OP, M1, M2, ZERO) \
  R \
  F (const M1& m1, const M2& m2) \
  { \
    R r; \
 \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
 \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
 \
    if (m1_nr != m2_nr || m1_nc != m2_nc) \
      gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
    else \
      { \
	/* Count num of non-zero elements */ \
	octave_idx_type nel = 0; \
	for (octave_idx_type j = 0; j < m1_nc; j++) \
	  for (octave_idx_type i = 0; i < m1_nr; i++) \
	    if ((m1.elem(i, j) OP m2.elem(i, j)) != ZERO) \
	      nel++; \
	\
        r = R (m1_nr, m1_nc, nel); \
        \
	octave_idx_type ii = 0; \
	r.cidx (0) = 0; \
        for (octave_idx_type j = 0 ; j < m1_nc ; j++) \
          { \
	    for (octave_idx_type i = 0 ; i < m1_nr ; i++)	\
	      {	\
	        if ((m1.elem(i, j) OP m2.elem(i, j)) != ZERO) \
		  { \
		    r.data (ii) = m1.elem(i, j) OP m2.elem(i,j); \
		    r.ridx (ii++) = i; \
		  } \
	      } \
	    r.cidx(j+1) = ii; \
	  } \
      } \
 \
    return r; \
  }

// FIXME Pass a specific ZERO value
#define SPARSE_SMM_BIN_OPS(R1, R2, M1, M2) \
  SPARSE_SMM_BIN_OP_1 (R1, operator +,  +, M1, M2) \
  SPARSE_SMM_BIN_OP_1 (R1, operator -,  -, M1, M2) \
  SPARSE_SMM_BIN_OP_2 (R2, product,     *, M1, M2, 0.0) \
  SPARSE_SMM_BIN_OP_2 (R2, quotient,    /, M1, M2, 0.0)

#define SPARSE_SMM_CMP_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_lt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_le, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ge, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_gt, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_SMM_EQNE_OP_DECLS(M1, M2) \
  SPARSE_CMP_OP_DECL (mx_el_eq, M1, M2); \
  SPARSE_CMP_OP_DECL (mx_el_ne, M1, M2);

#define SPARSE_SMM_CMP_OP(F, OP, M1, C1, M2, C2)	\
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if (C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j))) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = C1 (m1.elem(i, j)) OP C2 (m2.elem(i, j)); \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_SMM_CMP_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_SMM_CMP_OP (mx_el_lt, <,  M1, C1, M2, C2) \
  SPARSE_SMM_CMP_OP (mx_el_le, <=, M1, C1, M2, C2) \
  SPARSE_SMM_CMP_OP (mx_el_ge, >=, M1, C1, M2, C2) \
  SPARSE_SMM_CMP_OP (mx_el_gt, >,  M1, C1, M2, C2) \
  SPARSE_SMM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_SMM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_SMM_EQNE_OPS(M1, Z1, C1, M2, Z2, C2)  \
  SPARSE_SMM_CMP_OP (mx_el_eq, ==, M1,   , M2,   ) \
  SPARSE_SMM_CMP_OP (mx_el_ne, !=, M1,   , M2,   )

#define SPARSE_SMM_BOOL_OP_DECLS(M1, M2) \
  SPARSE_BOOL_OP_DECL (mx_el_and, M1, M2); \
  SPARSE_BOOL_OP_DECL (mx_el_or,  M1, M2);

#define SPARSE_SMM_BOOL_OP(F, OP, M1, M2, LHS_ZERO, RHS_ZERO) \
  SparseBoolMatrix \
  F (const M1& m1, const M2& m2) \
  { \
    SparseBoolMatrix r; \
    \
    octave_idx_type m1_nr = m1.rows (); \
    octave_idx_type m1_nc = m1.cols (); \
    \
    octave_idx_type m2_nr = m2.rows (); \
    octave_idx_type m2_nc = m2.cols (); \
    \
    if (m1_nr == m2_nr && m1_nc == m2_nc) \
      { \
	if (m1_nr != 0 || m1_nc != 0) \
	  { \
	    /* Count num of non-zero elements */ \
	    octave_idx_type nel = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      for (octave_idx_type i = 0; i < m1_nr; i++) \
		if ((m1.elem(i, j) != LHS_ZERO) \
		    OP (m2.elem(i, j) != RHS_ZERO)) \
		  nel++; \
            \
            r = SparseBoolMatrix (m1_nr, m1_nc, nel); \
            \
	    octave_idx_type ii = 0; \
	    r.cidx (0) = 0; \
	    for (octave_idx_type j = 0; j < m1_nc; j++) \
	      { \
	        for (octave_idx_type i = 0; i < m1_nr; i++) \
		  { \
		    bool el = (m1.elem(i, j) != LHS_ZERO) \
		      OP (m2.elem(i, j) != RHS_ZERO);	  \
		    if (el) \
		      { \
			r.data(ii) = el; \
			r.ridx(ii++) = i; \
		      } \
		  } \
		r.cidx(j+1) = ii; \
	      } \
	  } \
      }	      \
    else \
      { \
	if ((m1_nr != 0 || m1_nc != 0) && (m2_nr != 0 || m2_nc != 0)) \
	  gripe_nonconformant (#F, m1_nr, m1_nc, m2_nr, m2_nc); \
      } \
    return r; \
  }

#define SPARSE_SMM_BOOL_OPS2(M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMM_BOOL_OP (mx_el_and, &&, M1, M2, LHS_ZERO, RHS_ZERO) \
  SPARSE_SMM_BOOL_OP (mx_el_or,  ||, M1, M2, LHS_ZERO, RHS_ZERO) \

#define SPARSE_SMM_BOOL_OPS(M1, M2, ZERO) \
  SPARSE_SMM_BOOL_OPS2(M1, M2, ZERO, ZERO)

#define SPARSE_SMM_OP_DECLS(R1, R2, M1, M2) \
  SPARSE_SMM_BIN_OP_DECLS (R1, R2, M1, M2) \
  SPARSE_SMM_CMP_OP_DECLS (M1, M2) \
  SPARSE_SMM_BOOL_OP_DECLS (M1, M2)

// Avoid some code duplication.  Maybe we should use templates.

#define SPARSE_CUMSUM(RET_TYPE, ELT_TYPE, FCN)	\
 \
  octave_idx_type nr = rows (); \
  octave_idx_type nc = cols (); \
 \
  RET_TYPE retval; \
 \
  if (nr > 0 && nc > 0) \
    { \
      if ((nr == 1 && dim == -1) || dim == 1) \
	/* Ugly!! Is there a better way? */ \
        retval = transpose (). FCN (0) .transpose (); \
      else \
	{ \
          octave_idx_type nel = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
            { \
              ELT_TYPE t = ELT_TYPE (); \
	      for (octave_idx_type j = cidx (i); j < cidx (i+1); j++)	\
                { \
                  t += data(j); \
                  if (t != ELT_TYPE ()) \
                    if (j == cidx(i+1) - 1) \
		      nel += nr - ridx(j); \
                    else \
		      nel += ridx(j+1) - ridx(j); \
                } \
	    } \
	  retval = RET_TYPE (nr, nc, nel); \
          retval.cidx(0) = 0; \
	  octave_idx_type ii = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
            { \
              ELT_TYPE t = ELT_TYPE (); \
	      for (octave_idx_type j = cidx (i); j < cidx (i+1); j++)	\
                { \
                  t += data(j); \
                  if (t != ELT_TYPE ()) \
                    { \
                      if (j == cidx(i+1) - 1) \
                        { \
                          for (octave_idx_type k = ridx(j); k < nr; k++) \
                            { \
                               retval.data (ii) = t; \
                               retval.ridx (ii++) = k; \
                            } \
                        } \
		      else \
			{ \
                          for (octave_idx_type k = ridx(j); k < ridx(j+1); k++) \
                            { \
                               retval.data (ii) = t; \
                               retval.ridx (ii++) = k; \
                            } \
                        } \
                    } \
                } \
              retval.cidx(i+1) = ii; \
	    } \
	} \
    } \
  else \
    retval = RET_TYPE (nr,nc); \
 \
  return retval


#define SPARSE_CUMPROD(RET_TYPE, ELT_TYPE, FCN)	\
 \
  octave_idx_type nr = rows (); \
  octave_idx_type nc = cols (); \
 \
  RET_TYPE retval; \
 \
  if (nr > 0 && nc > 0) \
    { \
      if ((nr == 1 && dim == -1) || dim == 1) \
	/* Ugly!! Is there a better way? */ \
        retval = transpose (). FCN (0) .transpose (); \
      else \
	{ \
          octave_idx_type nel = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
            { \
	      octave_idx_type jj = 0; \
	      for (octave_idx_type j = cidx (i); j < cidx (i+1); j++) \
                { \
		  if (jj == ridx(j)) \
                    { \
                      nel++; \
                      jj++; \
                    } \
                  else \
                    break; \
                } \
	    } \
	  retval = RET_TYPE (nr, nc, nel); \
          retval.cidx(0) = 0; \
	  octave_idx_type ii = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
            { \
              ELT_TYPE t = ELT_TYPE (1.); \
	      octave_idx_type jj = 0; \
	      for (octave_idx_type j = cidx (i); j < cidx (i+1); j++) \
                { \
		  if (jj == ridx(j)) \
                    { \
                      t *= data(j); \
                      retval.data(ii) = t; \
                      retval.ridx(ii++) = jj++; \
                    } \
                  else \
                    break; \
                } \
              retval.cidx(i+1) = ii; \
	    } \
	} \
    } \
  else \
    retval = RET_TYPE (nr,nc); \
 \
  return retval

#define SPARSE_BASE_REDUCTION_OP(RET_TYPE, EL_TYPE, ROW_EXPR, COL_EXPR, \
			         INIT_VAL, MT_RESULT) \
 \
  octave_idx_type nr = rows (); \
  octave_idx_type nc = cols (); \
 \
  RET_TYPE retval; \
 \
  if (nr > 0 && nc > 0) \
    { \
      if ((nr == 1 && dim == -1) || dim == 1) \
	{ \
	  OCTAVE_LOCAL_BUFFER (EL_TYPE, tmp, nr); \
          \
	  for (octave_idx_type i = 0; i < nr; i++) \
	    { \
	      tmp[i] = INIT_VAL; \
	      for (octave_idx_type j = 0; j < nc; j++) \
		{ \
		  ROW_EXPR; \
		} \
	    } \
	  octave_idx_type nel = 0; \
	  for (octave_idx_type i = 0; i < nr; i++) \
	    if (tmp[i] != EL_TYPE ())  \
	      nel++ ; \
	  retval = RET_TYPE (nr, static_cast<octave_idx_type> (1), nel); \
	  retval.cidx(0) = 0; \
	  retval.cidx(1) = nel; \
	  nel = 0; \
	  for (octave_idx_type i = 0; i < nr; i++) \
	    if (tmp[i] != EL_TYPE ())  \
	      { \
		retval.data(nel) = tmp[i]; \
		retval.ridx(nel++) = i; \
	      } \
	} \
      else \
	{ \
	  OCTAVE_LOCAL_BUFFER (EL_TYPE, tmp, nc); \
          \
	  for (octave_idx_type j = 0; j < nc; j++) \
	    { \
	      tmp[j] = INIT_VAL; \
	      for (octave_idx_type i = 0; i < nr; i++) \
		{ \
		  COL_EXPR; \
		} \
	    } \
	  octave_idx_type nel = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
	    if (tmp[i] != EL_TYPE ())  \
	      nel++ ; \
	  retval = RET_TYPE (static_cast<octave_idx_type> (1), nc, nel); \
	  retval.cidx(0) = 0; \
	  nel = 0; \
	  for (octave_idx_type i = 0; i < nc; i++) \
	    if (tmp[i] != EL_TYPE ())  \
	      { \
		retval.data(nel) = tmp[i]; \
		retval.ridx(nel++) = 0; \
		retval.cidx(i+1) = retval.cidx(i) + 1; \
	      } \
	    else \
	      retval.cidx(i+1) = retval.cidx(i); \
	} \
    } \
  else if (nc == 0 && (nr == 0 || (nr == 1 && dim == -1))) \
    { \
      retval = RET_TYPE (static_cast<octave_idx_type> (1), \
                         static_cast<octave_idx_type> (1), \
                         static_cast<octave_idx_type> (1)); \
      retval.cidx(0) = 0; \
      retval.cidx(1) = 1; \
      retval.ridx(0) = 0; \
      retval.data(0) = MT_RESULT; \
    } \
  else if (nr == 0 && (dim == 0 || dim == -1)) \
    { \
      retval = RET_TYPE (static_cast<octave_idx_type> (1), nc, nc); \
      retval.cidx (0) = 0; \
      for (octave_idx_type i = 0; i < nc ; i++) \
        { \
          retval.ridx (i) = 0; \
          retval.cidx (i+1) = i; \
	  retval.data (i) = MT_RESULT; \
	} \
    } \
  else if (nc == 0 && dim == 1) \
    { \
      retval = RET_TYPE (nr, static_cast<octave_idx_type> (1), nr); \
      retval.cidx(0) = 0; \
      retval.cidx(1) = nr; \
      for (octave_idx_type i = 0; i < nr; i++) \
	{ \
	  retval.ridx(i) = i; \
	  retval.data(i) = MT_RESULT; \
	} \
    } \
  else \
    retval.resize (nr > 0, nc > 0); \
 \
  return retval

#define SPARSE_REDUCTION_OP_ROW_EXPR(OP) \
  tmp[i] OP elem (i, j)

#define SPARSE_REDUCTION_OP_COL_EXPR(OP) \
  tmp[j] OP elem (i, j)

#define SPARSE_REDUCTION_OP(RET_TYPE, EL_TYPE, OP, INIT_VAL, MT_RESULT)	\
  SPARSE_BASE_REDUCTION_OP (RET_TYPE, EL_TYPE, \
			SPARSE_REDUCTION_OP_ROW_EXPR (OP), \
			SPARSE_REDUCTION_OP_COL_EXPR (OP), \
			INIT_VAL, MT_RESULT)

#define SPARSE_ANY_ALL_OP_ROW_CODE(TEST_OP, TEST_TRUE_VAL) \
  if (elem (i, j) TEST_OP 0.0) \
    { \
      tmp[i] = TEST_TRUE_VAL; \
      break; \
    }

#define SPARSE_ANY_ALL_OP_COL_CODE(TEST_OP, TEST_TRUE_VAL) \
  if (elem (i, j) TEST_OP 0.0) \
    { \
      tmp[j] = TEST_TRUE_VAL; \
      break; \
    }

#define SPARSE_ANY_ALL_OP(DIM, INIT_VAL, TEST_OP, TEST_TRUE_VAL) \
  SPARSE_BASE_REDUCTION_OP (SparseBoolMatrix, char, \
			SPARSE_ANY_ALL_OP_ROW_CODE (TEST_OP, TEST_TRUE_VAL), \
			SPARSE_ANY_ALL_OP_COL_CODE (TEST_OP, TEST_TRUE_VAL), \
			INIT_VAL, INIT_VAL)

#define SPARSE_ALL_OP(DIM) SPARSE_ANY_ALL_OP (DIM, true, ==, false)

#define SPARSE_ANY_OP(DIM) SPARSE_ANY_ALL_OP (DIM, false, !=, true)

#define SPARSE_SPARSE_MUL( RET_TYPE, RET_EL_TYPE, EL_TYPE ) \
  octave_idx_type nr = m.rows (); \
  octave_idx_type nc = m.cols (); \
  \
  octave_idx_type a_nr = a.rows (); \
  octave_idx_type a_nc = a.cols (); \
  \
  if (nc != a_nr) \
    { \
      gripe_nonconformant ("operator *", nr, nc, a_nr, a_nc); \
      return RET_TYPE (); \
    } \
  else \
    { \
      OCTAVE_LOCAL_BUFFER (octave_idx_type, w, nr); \
      for (octave_idx_type i = 0; i < nr; i++) \
	w[i] = 0; \
      \
      octave_idx_type nel = 0; \
      \
      for (octave_idx_type i = 0; i < a_nc; i++) \
        { \
          for (octave_idx_type j = a.cidx(i); j < a.cidx(i+1); j++) \
            { \
              octave_idx_type  col = a.ridx(j); \
              for (octave_idx_type k = m.cidx(col) ; k < m.cidx(col+1); k++) \
		{ \
		  if (w[m.ridx(k)] < i + 1) \
                    { \
		      w[m.ridx(k)] = i + 1; \
		      nel++; \
		    } \
	          OCTAVE_QUIT; \
		} \
	    } \
	} \
      \
      if (nel == 0) \
	return RET_TYPE (nr, a_nc); \
      else \
	{  \
          for (octave_idx_type i = 0; i < nr; i++) \
	    w[i] = 0; \
	  \
          OCTAVE_LOCAL_BUFFER (RET_EL_TYPE, Xcol, nr); \
          \
	  RET_TYPE retval (nr, a_nc, nel); \
	  octave_idx_type ii = 0; \
	  /* The optimal break-point as estimated from simulations */ \
	  /* Note that Mergesort is O(nz log(nz)) while searching all */ \
	  /* values is O(nr), where nz here is non-zero per row of */ \
	  /* length nr. The test itself was then derived from the */ \
	  /* simulation with random square matrices and the observation */ \
	  /* of the number of non-zero elements in the output matrix */ \
	  /* it was found that the breakpoints were */ \
	  /*   nr: 500  1000  2000  5000 10000 */ \
	  /*   nz:   6    25    97   585  2202 */ \
	  /* The below is a simplication of the 'polyfit'-ed parameters */ \
	  /* to these breakpoints */ \
	  if (nr > 43000 || ((nr * nr) * double(a_nc)) / 43000 > nel) \
	    { \
	      octave_idx_type *ri = retval.xridx(); \
	      octave_sort<octave_idx_type> sort; \
	      \
	      retval.xcidx(0) = 0; \
	      for (octave_idx_type i = 0; i < a_nc ; i++) \
		{ \
		  for (octave_idx_type j = a.cidx(i); j < a.cidx(i+1); j++) \
		    { \
		      octave_idx_type col = a.ridx(j); \
		      EL_TYPE tmpval = a.data(j); \
		      for (octave_idx_type k = m.cidx(col) ; \
			   k < m.cidx(col+1); k++) \
			{ \
			  OCTAVE_QUIT; \
			  octave_idx_type row = m.ridx(k); \
			  if (w[row] < i + 1) \
			    { \
			      w[row] = i + 1; \
			      retval.xridx(ii++) = row;\
			      Xcol[row] = tmpval * m.data(k); \
			    } \
			  else \
			    Xcol[row] += tmpval * m.data(k); \
			} \
		    } \
	          sort.sort (ri + retval.xcidx(i), ii - retval.xcidx(i)); \
	          for (octave_idx_type k = retval.xcidx(i); k < ii; k++) \
		    retval.xdata(k) = Xcol[retval.xridx(k)]; \
		  retval.xcidx(i+1) = ii; \
		}  \
	      retval.maybe_compress (true);\
	    }				   \
	  else \
	    { \
	      retval.xcidx(0) = 0; \
	      for (octave_idx_type i = 0; i < a_nc ; i++) \
		{ \
		  for (octave_idx_type j = a.cidx(i); j < a.cidx(i+1); j++) \
		    { \
		      octave_idx_type col = a.ridx(j); \
		      EL_TYPE tmpval = a.data(j); \
		      for (octave_idx_type k = m.cidx(col) ; \
			   k < m.cidx(col+1); k++) \
			{ \
			  OCTAVE_QUIT; \
			  octave_idx_type row = m.ridx(k); \
			  if (w[row] < i + 1) \
			    { \
			      w[row] = i + 1; \
			      Xcol[row] = tmpval * m.data(k); \
			    } \
			  else \
			    Xcol[row] += tmpval * m.data(k); \
			} \
		    } \
		  for (octave_idx_type k = 0; k < nr; k++) \
		    if (w[k] == i + 1 && Xcol[k] != 0.) \
		      { \
		        retval.xdata(ii) = Xcol[k]; \
		        retval.xridx(ii++) = k; \
		      } \
		  retval.xcidx(i+1) = ii; \
		}  \
	      retval.maybe_compress ();\
	    } \
	  return retval; \
	} \
    }

#define SPARSE_FULL_MUL( RET_TYPE, EL_TYPE, ZERO ) \
  octave_idx_type nr = m.rows (); \
  octave_idx_type nc = m.cols (); \
  \
  octave_idx_type a_nr = a.rows (); \
  octave_idx_type a_nc = a.cols (); \
  \
  if (nc != a_nr) \
    { \
      gripe_nonconformant ("operator *", nr, nc, a_nr, a_nc); \
      return RET_TYPE (); \
    } \
  else \
    { \
      RET_TYPE retval (nr, a_nc, ZERO); \
      \
      for (octave_idx_type i = 0; i < a_nc ; i++) \
	{ \
	  for (octave_idx_type j = 0; j < a_nr; j++) \
	    { \
              OCTAVE_QUIT; \
	      \
              EL_TYPE tmpval = a.elem(j,i); \
	      for (octave_idx_type k = m.cidx(j) ; k < m.cidx(j+1); k++) \
	        retval.elem (m.ridx(k),i) += tmpval * m.data(k); \
	    } \
        } \
      return retval; \
    }

#define FULL_SPARSE_MUL( RET_TYPE, EL_TYPE, ZERO ) \
  octave_idx_type nr = m.rows (); \
  octave_idx_type nc = m.cols (); \
  \
  octave_idx_type a_nr = a.rows (); \
  octave_idx_type a_nc = a.cols (); \
  \
  if (nc != a_nr) \
    { \
      gripe_nonconformant ("operator *", nr, nc, a_nr, a_nc); \
      return RET_TYPE (); \
    } \
  else \
    { \
      RET_TYPE retval (nr, a_nc, ZERO); \
      \
      for (octave_idx_type i = 0; i < a_nc ; i++) \
	{ \
	   for (octave_idx_type j = a.cidx(i); j < a.cidx(i+1); j++) \
	     { \
	        octave_idx_type col = a.ridx(j); \
	        EL_TYPE tmpval = a.data(j); \
                OCTAVE_QUIT; \
	        \
	        for (octave_idx_type k = 0 ; k < nr; k++) \
	          retval.elem (k,i) += tmpval * m.elem(k,col); \
	    } \
        } \
      return retval; \
    }

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
