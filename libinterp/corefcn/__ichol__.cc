////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2013-2023 The Octave Project Developers
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

#include <limits>

#include "oct-locbuf.h"
#include "oct-norm.h"

#include "defun.h"
#include "error.h"

#include "builtin-defun-decls.h"

OCTAVE_BEGIN_NAMESPACE(octave)

// Secondary functions for complex and real case used in ichol algorithms.
Complex ichol_mult_complex (Complex a, Complex b)
{
#if defined (HAVE_CXX_COMPLEX_SETTERS)
  b.imag (-b.imag ());
#elif defined (HAVE_CXX_COMPLEX_REFERENCE_ACCESSORS)
  b.imag () = -b.imag ();
#else
  b = b.conj ();
#endif
  return a * b;
}

double ichol_mult_real (double a, double b)
{
  return a * b;
}

bool ichol_checkpivot_complex (Complex pivot)
{
  if (fabs (pivot.imag ()) > std::numeric_limits<double>::epsilon())
    error ("ichol: non-real pivot encountered.  The matrix must be Hermitian.");
  else if (pivot.real () < 0)
    error ("ichol: negative pivot encountered");

  return true;
}

bool ichol_checkpivot_real (double pivot)
{
  if (pivot < 0)
    error ("ichol: negative pivot encountered");

  return true;
}

template <typename octave_matrix_t, typename T, T (*ichol_mult) (T, T),
          bool (*ichol_checkpivot) (T)>
void ichol_0 (octave_matrix_t& sm, const std::string michol = "off")
{
  const octave_idx_type n = sm.cols ();
  octave_idx_type j1, jend, j2, jrow, jjrow, j, jw, i, k, jj, r;
  T tl;

  char opt;
  enum {OFF, ON};
  if (michol == "on")
    opt = ON;
  else
    opt = OFF;

  // Input matrix pointers
  octave_idx_type *cidx = sm.cidx ();
  octave_idx_type *ridx = sm.ridx ();
  T *data = sm.data ();

  // Working arrays
  OCTAVE_LOCAL_BUFFER (octave_idx_type, Lfirst, n);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, Llist, n);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, iw, n);
  OCTAVE_LOCAL_BUFFER (T, dropsums, n);

  // Initialize working arrays
  for (i = 0; i < n; i++)
    {
      iw[i] = -1;
      Llist[i] = -1;
      Lfirst[i] = -1;
      dropsums[i] = 0;
    }

  // Loop over all columns
  for (k = 0; k < n; k++)
    {
      j1 = cidx[k];
      j2 = cidx[k+1];
      for (j = j1; j < j2; j++)
        iw[ridx[j]] = j;

      jrow = Llist[k];
      // Iterate over each non-zero element in the actual row.
      while (jrow != -1)
        {
          jjrow = Lfirst[jrow];
          jend = cidx[jrow+1];
          for (jj = jjrow; jj < jend; jj++)
            {
              r = ridx[jj];
              jw = iw[r];
              tl = ichol_mult (data[jj], data[jjrow]);
              if (jw != -1)
                data[jw] -= tl;
              else
                // Because of the symmetry of the matrix, we know
                // the drops in the column r are also in the column k.
                if (opt == ON)
                  {
                    dropsums[r] -= tl;
                    dropsums[k] -= tl;
                  }
            }
          // Update the linked list and the first entry of the actual column.
          if ((jjrow + 1) < jend)
            {
              Lfirst[jrow]++;
              j = jrow;
              jrow = Llist[jrow];
              Llist[j] = Llist[ridx[Lfirst[j]]];
              Llist[ridx[Lfirst[j]]] = j;
            }
          else
            jrow = Llist[jrow];
        }

      if (opt == ON)
        data[j1] += dropsums[k];

      // Test for j1 == j2 must be first to avoid invalid ridx[j1] access
      if (j1 == j2 || ridx[j1] != k)
        error ("ichol: encountered a pivot equal to 0");

      if (! ichol_checkpivot (data[j1]))
        break;

      data[cidx[k]] = std::sqrt (data[j1]);

      // Update Llist and Lfirst with the k-column information.  Also,
      // scale the column elements by the pivot and reset the working array iw.
      if (k < (n - 1))
        {
          iw[ridx[j1]] = -1;
          for (i = j1 + 1; i < j2; i++)
            {
              iw[ridx[i]] = -1;
              data[i] /= data[j1];
            }
          Lfirst[k] = j1;
          if ((Lfirst[k] + 1) < j2)
            {
              Lfirst[k]++;
              jjrow = ridx[Lfirst[k]];
              Llist[k] = Llist[jjrow];
              Llist[jjrow] = k;
            }
        }
    }
}

DEFUN (__ichol0__, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{L} =} __ichol0__ (@var{A}, @var{michol})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 2)
    print_usage ();

  std::string michol = args(1).string_value ();

  // In ICHOL0 algorithm the zero-pattern of the input matrix is preserved
  // so its structure does not change during the algorithm.  The same input
  // matrix is used to build the output matrix due to that fact.
  if (! args(0).iscomplex ())
    {
      SparseMatrix sm = Ftril (args(0))(0).sparse_matrix_value ();
      ichol_0 <SparseMatrix, double, ichol_mult_real,
              ichol_checkpivot_real> (sm, michol);
      return ovl (sm);
    }
  else
    {
      SparseComplexMatrix sm
        = Ftril (args(0))(0).sparse_complex_matrix_value ();
      ichol_0 <SparseComplexMatrix, Complex, ichol_mult_complex,
              ichol_checkpivot_complex> (sm, michol);
      return ovl (sm);
    }
}

template <typename octave_matrix_t, typename T,  T (*ichol_mult) (T, T),
          bool (*ichol_checkpivot) (T)>
void ichol_t (const octave_matrix_t& sm, octave_matrix_t& L, const T *cols_norm,
              const T droptol, const std::string michol = "off")
{

  const octave_idx_type n = sm.cols ();
  octave_idx_type j, jrow, jend, jjrow, i, k, jj, total_len,
                  w_len, max_len, ind;
  char opt;
  enum {OFF, ON};
  if (michol == "on")
    opt = ON;
  else
    opt = OFF;

  // Input matrix pointers
  octave_idx_type *cidx = sm.cidx ();
  octave_idx_type *ridx = sm.ridx ();
  T *data = sm.data ();

  // Output matrix data structures.  Because the final zero pattern pattern of
  // the output matrix is not known due to fill-in elements, a heuristic
  // approach has been adopted for memory allocation.  The size of ridx_out_l
  // and data_out_l is incremented 10% of their actual size (nnz (A) in the
  // beginning).  If that amount is less than n, their size is just incremented
  // in n elements.  This way the number of reallocations decreases throughout
  // the process, obtaining a good performance.
  max_len = sm.nnz ();
  max_len += (0.1 * max_len) > n ? 0.1 * max_len : n;
  Array <octave_idx_type> cidx_out_l (dim_vector (n + 1, 1));
  octave_idx_type *cidx_l = cidx_out_l.fortran_vec ();
  Array <octave_idx_type> ridx_out_l (dim_vector (max_len, 1));
  octave_idx_type *ridx_l = ridx_out_l.fortran_vec ();
  Array <T> data_out_l (dim_vector (max_len, 1));
  T *data_l = data_out_l.fortran_vec ();

  // Working arrays
  OCTAVE_LOCAL_BUFFER (T, w_data, n);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, Lfirst, n);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, Llist, n);
  OCTAVE_LOCAL_BUFFER (T, col_drops, n);
  std::vector<octave_idx_type> vec (n, 0);
  std::vector<bool> mark (n, false);

  T zero = T (0);
  cidx_l[0] = cidx[0];
  for (i = 0; i < n; i++)
    {
      Llist[i] = -1;
      Lfirst[i] = -1;
      w_data[i] = 0;
      col_drops[i] = zero;
    }

  total_len = 0;
  for (k = 0; k < n; k++)
    {
      ind = 0;
      for (j = cidx[k]; j < cidx[k+1]; j++)
        {
          w_data[ridx[j]] = data[j];
          // Mark column index, intentionally outside the if-clause to ensure
          // that mark[k] will be set to true as well.
          mark[ridx[j]] = true;
          if (ridx[j] != k)
            {
              vec[ind] = ridx[j];
              ind++;
            }
        }
      jrow = Llist[k];
      while (jrow != -1)
        {
          jjrow = Lfirst[jrow];
          jend = cidx_l[jrow+1];
          for (jj = jjrow; jj < jend; jj++)
            {
              j = ridx_l[jj];
              // If the element in the j position of the row is zero,
              // then it will become non-zero, so we add it to the
              // vector that tracks non-zero elements in the working row.
              if (! mark[j])
                {
                  mark[j] = true;
                  vec[ind] = j;
                  ind++;
                }
              w_data[j] -= ichol_mult (data_l[jj], data_l[jjrow]);
            }
          // Update the actual column first element and
          // update the linked list of the jrow row.
          if ((jjrow + 1) < jend)
            {
              Lfirst[jrow]++;
              j = jrow;
              jrow = Llist[jrow];
              Llist[j] = Llist[ridx_l[Lfirst[j]]];
              Llist[ridx_l[Lfirst[j]]] = j;
            }
          else
            jrow = Llist[jrow];
        }

      // Resizing output arrays
      if ((max_len - total_len) < n)
        {
          max_len += (0.1 * max_len) > n ? 0.1 * max_len : n;
          data_out_l.resize (dim_vector (max_len, 1));
          data_l = data_out_l.fortran_vec ();
          ridx_out_l.resize (dim_vector (max_len, 1));
          ridx_l = ridx_out_l.fortran_vec ();
        }

      // The sorting of the non-zero elements of the working column can be
      // handled in a couple of ways.  The most efficient two I found, are
      // keeping the elements in an ordered binary search tree dynamically or
      // keep them unsorted in a vector and at the end of the outer iteration
      // order them.  The last approach exhibits lower execution times.
      std::sort (vec.begin (), vec.begin () + ind);

      data_l[total_len] = w_data[k];
      ridx_l[total_len] = k;
      w_len = 1;

      // Extract the non-zero elements of working column and
      // drop the elements that are lower than droptol * cols_norm[k].
      for (i = 0; i < ind ; i++)
        {
          jrow = vec[i];
          if (w_data[jrow] != zero)
            {
              if (std::abs (w_data[jrow]) < (droptol * cols_norm[k]))
                {
                  if (opt == ON)
                    {
                      col_drops[k] += w_data[jrow];
                      col_drops[jrow] += w_data[jrow];
                    }
                }
              else
                {
                  data_l[total_len + w_len] = w_data[jrow];
                  ridx_l[total_len + w_len] = jrow;
                  w_len++;
                }
            }
          // Clear mark, vec, and w_data.  However, mark[k] is not set to zero.
          mark[jrow] = false;
          vec[i] = 0;
          w_data[jrow] = zero;
        }

      // Compensate column sums --> michol option
      if (opt == ON)
        data_l[total_len] += col_drops[k];

      if (data_l[total_len] == zero)
        error ("ichol: encountered a pivot equal to 0");

      if (! ichol_checkpivot (data_l[total_len]))
        break;

      // Once elements are dropped and compensation of column sums are done,
      // scale the elements by the pivot.
      data_l[total_len] = std::sqrt (data_l[total_len]);
      for (jj = total_len + 1; jj < (total_len + w_len); jj++)
        data_l[jj] /= data_l[total_len];
      total_len += w_len;
      // Check if there are too many elements to be indexed with
      // octave_idx_type type due to fill-in during the process.
      if (total_len < 0)
        error ("ichol: integer overflow.  Too many fill-in elements in L");

      cidx_l[k+1] = cidx_l[k] - cidx_l[0] + w_len;

      // Update Llist and Lfirst with the k-column information.
      if (k < (n - 1))
        {
          Lfirst[k] = cidx_l[k];
          if ((Lfirst[k] + 1) < cidx_l[k+1])
            {
              Lfirst[k]++;
              jjrow = ridx_l[Lfirst[k]];
              Llist[k] = Llist[jjrow];
              Llist[jjrow] = k;
            }
        }
    }

  // Build the output matrices
  L = octave_matrix_t (n, n, total_len);

  for (i = 0; i <= n; i++)
    L.cidx (i) = cidx_l[i];

  for (i = 0; i < total_len; i++)
    {
      L.ridx (i) = ridx_l[i];
      L.data (i) = data_l[i];
    }
}

DEFUN (__icholt__, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{L} =} __icholt__ (@var{A}, @var{droptol}, @var{michol})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 3)
    print_usage ();

  double droptol = args(1).double_value ();
  std::string michol = args(2).string_value ();

  if (! args(0).iscomplex ())
    {
      SparseMatrix L;
      SparseMatrix sm_l = Ftril (args(0))(0).sparse_matrix_value ();
      RowVector sm_col_norms = xcolnorms (sm_l, 1);
      ichol_t <SparseMatrix,
              double, ichol_mult_real, ichol_checkpivot_real>
              (sm_l, L, sm_col_norms.fortran_vec (), droptol, michol);

      return ovl (L);
    }
  else
    {
      SparseComplexMatrix L;
      SparseComplexMatrix sm_l
        = Ftril (args(0))(0).sparse_complex_matrix_value ();
      Array <Complex> cols_norm = xcolnorms (sm_l, 1);
      ichol_t <SparseComplexMatrix,
              Complex, ichol_mult_complex, ichol_checkpivot_complex>
              (sm_l, L, cols_norm.fortran_vec (),
               Complex (droptol), michol);

      return ovl (L);
    }
}

/*
%!test <*51736>
%! k = 4;
%! n = 2^k;
%! Afull = diag (ones (n,1)) / ...
%!          2+triu ([zeros(n,2), [n.^-[1;1;2]*ones(1,n-2);zeros(n-3,n-2)]], 1);
%! A = sparse (Afull + Afull.');
%! opts.type = "ict";
%! opts.droptol = 0;
%! L = ichol (A, opts);
%! assert (norm (A - L*L.'), 0, 2*eps);
*/

OCTAVE_END_NAMESPACE(octave)
