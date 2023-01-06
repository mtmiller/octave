////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1998-2023 The Octave Project Developers
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

#include <cassert>

#include <limits>

#include "Array-util.h"
#include "oct-cmplx.h"
#include "quit.h"

#include "error.h"
#include "ovl.h"
#include "utils.h"

#include "dSparse.h"
#include "CSparse.h"
#include "ov-re-sparse.h"
#include "ov-cx-sparse.h"
#include "sparse-xpow.h"

OCTAVE_BEGIN_NAMESPACE(octave)

static inline bool
xisint (double x)
{
  return (octave::math::x_nint (x) == x
          && ((x >= 0 && x < std::numeric_limits<int>::max ())
              || (x <= 0 && x > std::numeric_limits<int>::min ())));
}

// Safer pow functions.  Only two make sense for sparse matrices, the
// others should all promote to full matrices.

octave_value
xpow (const SparseMatrix& a, double b)
{
  octave_value retval;

  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  if (nr == 0 || nc == 0)
    return SparseMatrix ();

  // If we are here, A is not empty ==> A needs to be square.
  if (nr != nc)
    error ("for A^b, A must be a square matrix.  Use .^ for elementwise power.");

  if (! xisint (b))
    error ("use full(a) ^ full(b)");

  int btmp = static_cast<int> (b);
  if (btmp == 0)
    {
      SparseMatrix tmp = SparseMatrix (nr, nr, nr);
      for (octave_idx_type i = 0; i < nr; i++)
        {
          tmp.data (i) = 1.0;
          tmp.ridx (i) = i;
        }
      for (octave_idx_type i = 0; i < nr + 1; i++)
        tmp.cidx (i) = i;

      retval = tmp;
    }
  else
    {
      SparseMatrix atmp;
      if (btmp < 0)
        {
          btmp = -btmp;

          octave_idx_type info;
          double rcond = 0.0;
          MatrixType mattyp (a);

          // FIXME: This causes an error if the input sparse matrix is all-zeros.
          // That behavior is inconsistent with A ^ b when A is a full all-zeros
          // matrix, which just returns Inf of the same size with a warning.
          atmp = a.inverse (mattyp, info, rcond, 1);

          if (info == -1)
            warning ("inverse: matrix singular to machine precision, rcond = %g", rcond);
        }
      else
        atmp = a;

      if (atmp.nnz () == 0)  // Fast return for all-zeros matrix
        return atmp;

      SparseMatrix result (atmp);

      btmp--;

      // There are two approaches to the actual exponentiation.
      // Exponentiation by squaring uses only a logarithmic number
      // of multiplications but the matrices it multiplies tend to be dense
      // towards the end.
      // Linear multiplication uses a linear number of multiplications
      // but one of the matrices it uses will be as sparse as the original
      // matrix.
      //
      // The time to multiply fixed-size matrices is strongly affected by their
      // sparsity. Denser matrices take much longer to multiply together.
      // See this URL for a worked-through example:
      // https://octave.discourse.group/t/3216/4
      //
      // The tradeoff is between many fast multiplications or a few slow ones.
      //
      // Large exponents favor the squaring technique, and sparse matrices
      // favor linear multiplication.
      //
      // We calculate a threshold based on the sparsity of the input
      // and use squaring for exponents larger than that.
      //
      // FIXME: Improve this threshold calculation.

      uint64_t sparsity = atmp.numel () / atmp.nnz (); // reciprocal of density
      int threshold = (sparsity >= 1000) ? 40
                      : (sparsity >=  100) ? 20
                      : 3;

      if (btmp > threshold) // use squaring technique
        {
          while (btmp > 0)
            {
              if (btmp & 1)
                result = result * atmp;

              btmp >>= 1;

              if (btmp > 0)
                atmp = atmp * atmp;
            }
        }
      else // use linear multiplication
        {
          for (int i = 0; i < btmp; i++)
            result = result * atmp;
        }

      retval = result;
    }

  return retval;
}

octave_value
xpow (const SparseComplexMatrix& a, double b)
{
  octave_value retval;

  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  if (nr == 0 || nc == 0)
    return SparseMatrix ();

  // If we are here, A is not empty ==> A needs to be square.
  if (nr != nc)
    error ("for A^b, A must be a square matrix.  Use .^ for elementwise power.");

  if (! xisint (b))
    error ("use full(a) ^ full(b)");

  int btmp = static_cast<int> (b);
  if (btmp == 0)
    {
      SparseMatrix tmp = SparseMatrix (nr, nr, nr);
      for (octave_idx_type i = 0; i < nr; i++)
        {
          tmp.data (i) = 1.0;
          tmp.ridx (i) = i;
        }
      for (octave_idx_type i = 0; i < nr + 1; i++)
        tmp.cidx (i) = i;

      retval = tmp;
    }
  else
    {
      SparseComplexMatrix atmp;
      if (btmp < 0)
        {
          btmp = -btmp;

          octave_idx_type info;
          double rcond = 0.0;
          MatrixType mattyp (a);

          atmp = a.inverse (mattyp, info, rcond, 1);

          if (info == -1)
            warning ("inverse: matrix singular to machine precision, rcond = %g", rcond);
        }
      else
        atmp = a;

      if (atmp.nnz () == 0)  // Fast return for all-zeros matrix
        return atmp;

      SparseComplexMatrix result (atmp);

      btmp--;

      // Select multiplication sequence based on sparsity of atmp.
      // See the long comment in xpow (const SparseMatrix& a, double b)
      // for more details.
      //
      // FIXME: Improve this threshold calculation.

      uint64_t sparsity = atmp.numel () / atmp.nnz (); // reciprocal of density
      int threshold = (sparsity >= 1000) ? 40
                      : (sparsity >=  100) ? 20
                      : 3;

      if (btmp > threshold) // use squaring technique
        {
          while (btmp > 0)
            {
              if (btmp & 1)
                result = result * atmp;

              btmp >>= 1;

              if (btmp > 0)
                atmp = atmp * atmp;
            }
        }
      else // use linear multiplication
        {
          for (int i = 0; i < btmp; i++)
            result = result * atmp;
        }

      retval = result;
    }

  return retval;
}

// Safer pow functions that work elementwise for matrices.
//
//       op2 \ op1:   s   m   cs   cm
//            +--   +---+---+----+----+
//   scalar   |     | * | 3 |  * |  9 |
//                  +---+---+----+----+
//   matrix         | 1 | 4 |  7 | 10 |
//                  +---+---+----+----+
//   complex_scalar | * | 5 |  * | 11 |
//                  +---+---+----+----+
//   complex_matrix | 2 | 6 |  8 | 12 |
//                  +---+---+----+----+
//
//   * -> not needed.

// FIXME: these functions need to be fixed so that things
// like
//
//   a = -1; b = [ 0, 0.5, 1 ]; r = a .^ b
//
// and
//
//   a = -1; b = [ 0, 0.5, 1 ]; for i = 1:3, r(i) = a .^ b(i), end
//
// produce identical results.  Also, it would be nice if -1^0.5
// produced a pure imaginary result instead of a complex number with a
// small real part.  But perhaps that's really a problem with the math
// library...

// Handle special case of scalar-sparse-matrix .^ sparse-matrix.
// Forwarding to the scalar elem_xpow function and then converting the
// result back to a sparse matrix is a bit wasteful but it does not
// seem worth the effort to optimize -- how often does this case come up
// in practice?

template <typename S, typename SM>
inline octave_value
scalar_xpow (const S& a, const SM& b)
{
  octave_value val = elem_xpow (a, b);

  if (val.iscomplex ())
    return SparseComplexMatrix (val.complex_matrix_value ());
  else
    return SparseMatrix (val.matrix_value ());
}

/*
%!assert (sparse (2) .^ [3, 4], sparse ([8, 16]))
%!assert <47775> (sparse (2i) .^ [3, 4], sparse ([-0-8i, 16]))

%!test <*63080>
%! Z = sparse ([]);
%! A = sparse (zeros (0, 2));
%! B = sparse (zeros (2, 0));
%! assert (Z ^  1, Z);
%! assert (Z ^  0, Z);
%! assert (Z ^ -1, Z);
%! assert (A ^  1, Z);
%! assert (A ^  0, Z);
%! assert (A ^ -1, Z);
%! assert (B ^  1, Z);
%! assert (B ^  0, Z);
%! assert (B ^ -1, Z);

%!test <*63080>
%! A = sparse (zeros (2, 2));
%! assert (A ^  1, A);
%! assert (A ^  0, sparse (eye (2, 2)));

%!test <63080>
%! A = sparse (zeros (2, 2));
%! assert (A ^ -1, sparse (inf (2, 2)));

*/

// -*- 1 -*-
octave_value
elem_xpow (double a, const SparseMatrix& b)
{
  octave_value retval;

  octave_idx_type nr = b.rows ();
  octave_idx_type nc = b.cols ();

  double d1, d2;

  if (a < 0.0 && ! b.all_integers (d1, d2))
    {
      Complex atmp (a);
      ComplexMatrix result (nr, nc);

      for (octave_idx_type j = 0; j < nc; j++)
        {
          for (octave_idx_type i = 0; i < nr; i++)
            {
              octave_quit ();
              result(i, j) = std::pow (atmp, b(i, j));
            }
        }

      retval = result;
    }
  else
    {
      Matrix result (nr, nc);

      for (octave_idx_type j = 0; j < nc; j++)
        {
          for (octave_idx_type i = 0; i < nr; i++)
            {
              octave_quit ();
              result(i, j) = std::pow (a, b(i, j));
            }
        }

      retval = result;
    }

  return retval;
}

// -*- 2 -*-
octave_value
elem_xpow (double a, const SparseComplexMatrix& b)
{
  octave_idx_type nr = b.rows ();
  octave_idx_type nc = b.cols ();

  Complex atmp (a);
  ComplexMatrix result (nr, nc);

  for (octave_idx_type j = 0; j < nc; j++)
    {
      for (octave_idx_type i = 0; i < nr; i++)
        {
          octave_quit ();
          result(i, j) = std::pow (atmp, b(i, j));
        }
    }

  return result;
}

// -*- 3 -*-
octave_value
elem_xpow (const SparseMatrix& a, double b)
{
  // FIXME: What should a .^ 0 give?  Matlab gives a
  // sparse matrix with same structure as a, which is strictly
  // incorrect.  Keep compatibility.

  octave_value retval;

  octave_idx_type nz = a.nnz ();

  if (b <= 0.0)
    {
      octave_idx_type nr = a.rows ();
      octave_idx_type nc = a.cols ();

      if (! xisint (b) && a.any_element_is_negative ())
        {
          ComplexMatrix result (nr, nc, Complex (std::pow (0.0, b)));

          // FIXME: avoid apparent GNU libm bug by
          // converting A and B to complex instead of just A.
          Complex btmp (b);

          for (octave_idx_type j = 0; j < nc; j++)
            for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
              {
                octave_quit ();

                Complex atmp (a.data (i));

                result(a.ridx (i), j) = std::pow (atmp, btmp);
              }

          retval = octave_value (result);
        }
      else
        {
          Matrix result (nr, nc, (std::pow (0.0, b)));

          for (octave_idx_type j = 0; j < nc; j++)
            for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
              {
                octave_quit ();
                result(a.ridx (i), j) = std::pow (a.data (i), b);
              }

          retval = octave_value (result);
        }
    }
  else if (! xisint (b) && a.any_element_is_negative ())
    {
      SparseComplexMatrix result (a);

      for (octave_idx_type i = 0; i < nz; i++)
        {
          octave_quit ();

          // FIXME: avoid apparent GNU libm bug by
          // converting A and B to complex instead of just A.

          Complex atmp (a.data (i));
          Complex btmp (b);

          result.data (i) = std::pow (atmp, btmp);
        }

      result.maybe_compress (true);

      retval = result;
    }
  else
    {
      SparseMatrix result (a);

      for (octave_idx_type i = 0; i < nz; i++)
        {
          octave_quit ();
          result.data (i) = std::pow (a.data (i), b);
        }

      result.maybe_compress (true);

      retval = result;
    }

  return retval;
}

// -*- 4 -*-
octave_value
elem_xpow (const SparseMatrix& a, const SparseMatrix& b)
{
  octave_value retval;

  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a.numel () == 1 && b.numel () > 1)
    return scalar_xpow (a(0), b);

  if (nr != b_nr || nc != b_nc)
    octave::err_nonconformant ("operator .^", nr, nc, b_nr, b_nc);

  int convert_to_complex = 0;
  for (octave_idx_type j = 0; j < nc; j++)
    for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
      {
        if (a.data(i) < 0.0)
          {
            double btmp = b (a.ridx (i), j);
            if (! xisint (btmp))
              {
                convert_to_complex = 1;
                goto done;
              }
          }
      }

done:

  // This is a dumb operator for sparse matrices anyway, and there is
  // no sensible way to handle the 0.^0 versus the 0.^x cases.  Therefore
  // allocate a full matrix filled for the 0.^0 case and shrink it later
  // as needed.

  if (convert_to_complex)
    {
      SparseComplexMatrix complex_result (nr, nc, Complex (1.0, 0.0));

      for (octave_idx_type j = 0; j < nc; j++)
        {
          for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
            {
              octave_quit ();
              complex_result.xelem (a.ridx (i), j)
                = std::pow (Complex (a.data (i)), Complex (b(a.ridx (i), j)));
            }
        }
      complex_result.maybe_compress (true);
      retval = complex_result;
    }
  else
    {
      SparseMatrix result (nr, nc, 1.0);

      for (octave_idx_type j = 0; j < nc; j++)
        {
          for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
            {
              octave_quit ();
              result.xelem (a.ridx (i), j) = std::pow (a.data (i),
                                             b(a.ridx (i), j));
            }
        }
      result.maybe_compress (true);
      retval = result;
    }

  return retval;
}

// -*- 5 -*-
octave_value
elem_xpow (const SparseMatrix& a, const Complex& b)
{
  octave_value retval;

  if (b == 0.0)
    // Can this case ever happen, due to automatic retyping with maybe_mutate?
    retval = octave_value (NDArray (a.dims (), 1));
  else
    {
      octave_idx_type nz = a.nnz ();
      SparseComplexMatrix result (a);

      for (octave_idx_type i = 0; i < nz; i++)
        {
          octave_quit ();
          result.data (i) = std::pow (Complex (a.data (i)), b);
        }

      result.maybe_compress (true);

      retval = result;
    }

  return retval;
}

// -*- 6 -*-
octave_value
elem_xpow (const SparseMatrix& a, const SparseComplexMatrix& b)
{
  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a.numel () == 1 && b.numel () > 1)
    return scalar_xpow (a(0), b);

  if (nr != b_nr || nc != b_nc)
    octave::err_nonconformant ("operator .^", nr, nc, b_nr, b_nc);

  SparseComplexMatrix result (nr, nc, Complex (1.0, 0.0));
  for (octave_idx_type j = 0; j < nc; j++)
    {
      for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
        {
          octave_quit ();
          result.xelem (a.ridx(i), j) = std::pow (a.data (i), b(a.ridx (i), j));
        }
    }

  result.maybe_compress (true);

  return result;
}

// -*- 7 -*-
octave_value
elem_xpow (const Complex& a, const SparseMatrix& b)
{
  octave_idx_type nr = b.rows ();
  octave_idx_type nc = b.cols ();

  ComplexMatrix result (nr, nc);

  for (octave_idx_type j = 0; j < nc; j++)
    {
      for (octave_idx_type i = 0; i < nr; i++)
        {
          octave_quit ();
          double btmp = b (i, j);
          if (xisint (btmp))
            result (i, j) = std::pow (a, static_cast<int> (btmp));
          else
            result (i, j) = std::pow (a, btmp);
        }
    }

  return result;
}

// -*- 8 -*-
octave_value
elem_xpow (const Complex& a, const SparseComplexMatrix& b)
{
  octave_idx_type nr = b.rows ();
  octave_idx_type nc = b.cols ();

  ComplexMatrix result (nr, nc);
  for (octave_idx_type j = 0; j < nc; j++)
    for (octave_idx_type i = 0; i < nr; i++)
      {
        octave_quit ();
        result (i, j) = std::pow (a, b (i, j));
      }

  return result;
}

// -*- 9 -*-
octave_value
elem_xpow (const SparseComplexMatrix& a, double b)
{
  octave_value retval;

  if (b <= 0)
    {
      octave_idx_type nr = a.rows ();
      octave_idx_type nc = a.cols ();

      ComplexMatrix result (nr, nc, Complex (std::pow (0.0, b)));

      if (xisint (b))
        {
          for (octave_idx_type j = 0; j < nc; j++)
            for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
              {
                octave_quit ();
                result (a.ridx (i), j)
                  = std::pow (a.data (i), static_cast<int> (b));
              }
        }
      else
        {
          for (octave_idx_type j = 0; j < nc; j++)
            for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
              {
                octave_quit ();
                result (a.ridx (i), j) = std::pow (a.data (i), b);
              }
        }

      retval = result;
    }
  else
    {
      octave_idx_type nz = a.nnz ();

      SparseComplexMatrix result (a);

      if (xisint (b))
        {
          for (octave_idx_type i = 0; i < nz; i++)
            {
              octave_quit ();
              result.data (i) = std::pow (a.data (i), static_cast<int> (b));
            }
        }
      else
        {
          for (octave_idx_type i = 0; i < nz; i++)
            {
              octave_quit ();
              result.data (i) = std::pow (a.data (i), b);
            }
        }

      result.maybe_compress (true);

      retval = result;
    }

  return retval;
}

// -*- 10 -*-
octave_value
elem_xpow (const SparseComplexMatrix& a, const SparseMatrix& b)
{
  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a.numel () == 1 && b.numel () > 1)
    return scalar_xpow (a(0), b);

  if (nr != b_nr || nc != b_nc)
    octave::err_nonconformant ("operator .^", nr, nc, b_nr, b_nc);

  SparseComplexMatrix result (nr, nc, Complex (1.0, 0.0));
  for (octave_idx_type j = 0; j < nc; j++)
    {
      for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
        {
          octave_quit ();
          double btmp = b(a.ridx (i), j);

          if (xisint (btmp))
            result.xelem (a.ridx (i), j) = std::pow (a.data (i),
                                           static_cast<int> (btmp));
          else
            result.xelem (a.ridx (i), j) = std::pow (a.data (i), btmp);
        }
    }

  result.maybe_compress (true);

  return result;
}

// -*- 11 -*-
octave_value
elem_xpow (const SparseComplexMatrix& a, const Complex& b)
{
  octave_value retval;

  if (b == 0.0)
    // Can this case ever happen, due to automatic retyping with maybe_mutate?
    retval = octave_value (NDArray (a.dims (), 1));
  else
    {

      octave_idx_type nz = a.nnz ();

      SparseComplexMatrix result (a);

      for (octave_idx_type i = 0; i < nz; i++)
        {
          octave_quit ();
          result.data (i) = std::pow (a.data (i), b);
        }

      result.maybe_compress (true);

      retval = result;
    }

  return retval;
}

// -*- 12 -*-
octave_value
elem_xpow (const SparseComplexMatrix& a, const SparseComplexMatrix& b)
{
  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a.numel () == 1 && b.numel () > 1)
    return scalar_xpow (a(0), b);

  if (nr != b_nr || nc != b_nc)
    octave::err_nonconformant ("operator .^", nr, nc, b_nr, b_nc);

  SparseComplexMatrix result (nr, nc, Complex (1.0, 0.0));
  for (octave_idx_type j = 0; j < nc; j++)
    {
      for (octave_idx_type i = a.cidx (j); i < a.cidx (j+1); i++)
        {
          octave_quit ();
          result.xelem (a.ridx (i), j) = std::pow (a.data (i),
                                         b(a.ridx (i), j));
        }
    }
  result.maybe_compress (true);

  return result;
}


OCTAVE_END_NAMESPACE(octave)
