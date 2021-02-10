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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cmath>

#include <istream>
#include <limits>
#include <ostream>

#include "Array-util.h"
#include "Range.h"
#include "lo-error.h"
#include "lo-mappers.h"
#include "lo-utils.h"

bool
Range::all_elements_are_ints (void) const
{
  // If the base and increment are ints, the final value in the range will also
  // be an integer, even if the limit is not.  If there is one or fewer
  // elements only the base needs to be an integer.

  return (! (octave::math::isnan (rng_base) || octave::math::isnan (rng_inc))
          && (octave::math::nint_big (rng_base) == rng_base || rng_numel < 1)
          && (octave::math::nint_big (rng_inc) == rng_inc || rng_numel <= 1));
}

octave_idx_type
Range::nnz (void) const
{
  octave_idx_type retval = 0;

  if (! isempty ())
    {
      if ((rng_base > 0.0 && rng_limit > 0.0)
          || (rng_base < 0.0 && rng_limit < 0.0))
        {
          // All elements have the same sign, hence there are no zeros.
          retval = rng_numel;
        }
      else if (rng_inc != 0.0)
        {
          if (rng_base == 0.0 || rng_limit == 0.0)
            // Exactly one zero at beginning or end of range.
            retval = rng_numel - 1;
          else if ((rng_base / rng_inc) != std::floor (rng_base / rng_inc))
            // Range crosses negative/positive without hitting zero.
            retval = rng_numel;
          else
            // Range crosses negative/positive and hits zero.
            retval = rng_numel - 1;
        }
      else
        {
          // All elements are equal (rng_inc = 0) but not positive or negative,
          // therefore all elements are zero.
          retval = 0;
        }
    }

  return retval;
}

Matrix
Range::matrix_value (void) const
{
  if (rng_numel > 0 && cache.isempty ())
    {
      cache.resize (1, rng_numel);

      // The first element must always be *exactly* the base.
      // E.g, -0 would otherwise become +0 in the loop (-0 + 0*increment).
      cache(0) = rng_base;

      double b = rng_base;
      double increment = rng_inc;
      for (octave_idx_type i = 1; i < rng_numel - 1; i++)
        cache.xelem (i) = b + i * increment;

      cache.xelem (rng_numel - 1) = rng_limit;
    }

  return cache;
}

double
Range::checkelem (octave_idx_type i) const
{
  if (i < 0 || i >= rng_numel)
    octave::err_index_out_of_range (2, 2, i+1, rng_numel, dims ());

  if (i == 0)
    return rng_base;
  else if (i < rng_numel - 1)
    return rng_base + i * rng_inc;
  else
    return rng_limit;
}

double
Range::checkelem (octave_idx_type i, octave_idx_type j) const
{
  // Ranges are *always* row vectors.
  if (i != 0)
    octave::err_index_out_of_range (1, 1, i+1, rng_numel, dims ());

  return checkelem (j);
}

double
Range::elem (octave_idx_type i) const
{
  if (i == 0)
    return rng_base;
  else if (i < rng_numel - 1)
    return rng_base + i * rng_inc;
  else
    return rng_limit;
}

// Helper class used solely for idx_vector.loop () function call
class __rangeidx_helper
{
public:
  __rangeidx_helper (double *a, double b, double i, double l, octave_idx_type n)
    : array (a), base (b), inc (i), limit (l), nmax (n-1) { }

  void operator () (octave_idx_type i)
  {
    if (i == 0)
      *array++ = base;
    else if (i < nmax)
      *array++ = base + i * inc;
    else
      *array++ = limit;
  }

private:

  double *array, base, inc, limit;
  octave_idx_type nmax;

};

Array<double>
Range::index (const idx_vector& i) const
{
  Array<double> retval;

  octave_idx_type n = rng_numel;

  if (i.is_colon ())
    {
      retval = matrix_value ().reshape (dim_vector (rng_numel, 1));
    }
  else
    {
      if (i.extent (n) != n)
        octave::err_index_out_of_range (1, 1, i.extent (n), n, dims ()); // throws

      dim_vector rd = i.orig_dimensions ();
      octave_idx_type il = i.length (n);

      // taken from Array.cc.
      if (n != 1 && rd.isvector ())
        rd = dim_vector (1, il);

      retval.clear (rd);

      // idx_vector loop across all values in i,
      // executing __rangeidx_helper (i) for each i
      i.loop (n, __rangeidx_helper (retval.fortran_vec (),
                                    rng_base, rng_inc, rng_limit, rng_numel));
    }

  return retval;
}

// NOTE: max and min only return useful values if numel > 0.
//       do_minmax_body() in max.cc avoids calling Range::min/max if numel == 0.

double
Range::min (void) const
{
  double retval = 0.0;
  if (rng_numel > 0)
    {
      if (rng_inc > 0)
        retval = rng_base;
      else
        {
          retval = rng_base + (rng_numel - 1) * rng_inc;

          // Require '<=' test.  See note in max ().
          if (retval <= rng_limit)
            retval = rng_limit;
        }

    }
  return retval;
}

double
Range::max (void) const
{
  double retval = 0.0;
  if (rng_numel > 0)
    {
      if (rng_inc > 0)
        {
          retval = rng_base + (rng_numel - 1) * rng_inc;

          // On some machines (x86 with extended precision floating point
          // arithmetic, for example) it is possible that we can overshoot the
          // limit by approximately the machine precision even though we were
          // very careful in our calculation of the number of elements.
          // Therefore, we clip the result to the limit if it overshoots.
          // The test also includes equality (>= rng_limit) to have expressions
          // such as -5:1:-0 result in a -0 endpoint.
          if (retval >= rng_limit)
            retval = rng_limit;
        }
      else
        retval = rng_base;
    }
  return retval;
}

void
Range::sort_internal (bool ascending)
{
  if ((ascending && rng_base > rng_limit && rng_inc < 0.0)
      || (! ascending && rng_base < rng_limit && rng_inc > 0.0))
    {
      std::swap (rng_base, rng_limit);
      rng_inc = -rng_inc;
      clear_cache ();
    }
}

void
Range::sort_internal (Array<octave_idx_type>& sidx, bool ascending)
{
  octave_idx_type nel = numel ();

  sidx.resize (dim_vector (1, nel));

  octave_idx_type *psidx = sidx.fortran_vec ();

  bool reverse = false;

  if ((ascending && rng_base > rng_limit && rng_inc < 0.0)
      || (! ascending && rng_base < rng_limit && rng_inc > 0.0))
    {
      std::swap (rng_base, rng_limit);
      rng_inc = -rng_inc;
      clear_cache ();
      reverse = true;
    }

  octave_idx_type tmp = (reverse ? nel - 1 : 0);
  octave_idx_type stp = (reverse ? -1 : 1);

  for (octave_idx_type i = 0; i < nel; i++, tmp += stp)
    psidx[i] = tmp;
}

Matrix
Range::diag (octave_idx_type k) const
{
  return matrix_value ().diag (k);
}

Range
Range::sort (octave_idx_type dim, sortmode mode) const
{
  Range retval = *this;

  if (dim == 1)
    {
      if (mode == ASCENDING)
        retval.sort_internal (true);
      else if (mode == DESCENDING)
        retval.sort_internal (false);
    }
  else if (dim != 0)
    (*current_liboctave_error_handler) ("Range::sort: invalid dimension");

  return retval;
}

Range
Range::sort (Array<octave_idx_type>& sidx, octave_idx_type dim,
             sortmode mode) const
{
  Range retval = *this;

  if (dim == 1)
    {
      if (mode == ASCENDING)
        retval.sort_internal (sidx, true);
      else if (mode == DESCENDING)
        retval.sort_internal (sidx, false);
    }
  else if (dim != 0)
    (*current_liboctave_error_handler) ("Range::sort: invalid dimension");

  return retval;
}

sortmode
Range::issorted (sortmode mode) const
{
  if (rng_numel > 1 && rng_inc > 0)
    mode = (mode == DESCENDING) ? UNSORTED : ASCENDING;
  else if (rng_numel > 1 && rng_inc < 0)
    mode = (mode == ASCENDING) ? UNSORTED : DESCENDING;
  else
    mode = (mode == UNSORTED) ? ASCENDING : mode;

  return mode;
}

void
Range::set_base (double b)
{
  if (rng_base != b)
    {
      rng_base = b;

      init ();
    }
}

void
Range::set_limit (double l)
{
  if (rng_limit != l)
    {
      rng_limit = l;

      init ();
    }
}

void
Range::set_inc (double i)
{
  if (rng_inc != i)
    {
      rng_inc = i;

      init ();
    }
}

std::ostream&
operator << (std::ostream& os, const Range& a)
{
  double b = a.base ();
  double increment = a.inc ();
  octave_idx_type nel = a.numel ();

  if (nel > 1)
    {
      // First element must be the base *exactly* (e.g., -0).
      os << b << ' ';
      for (octave_idx_type i = 1; i < nel-1; i++)
        os << b + i * increment << ' ';
    }

  // Print out the last element exactly, rather than a calculated last element.
  os << a.rng_limit << "\n";

  return os;
}

std::istream&
operator >> (std::istream& is, Range& a)
{
  is >> a.rng_base;
  if (is)
    {
      double tmp_rng_limit;
      is >> tmp_rng_limit;

      if (is)
        is >> a.rng_inc;

      // Clip the rng_limit to the true limit, rebuild numel, clear cache
      a.set_limit (tmp_rng_limit);
    }

  return is;
}

Range
operator - (const Range& r)
{
  return Range (-r.base (), -r.limit (), -r.inc (), r.numel ());
}

Range operator + (double x, const Range& r)
{
  Range result (x + r.base (), x + r.limit (), r.inc (), r.numel ());
  // Check whether new range was constructed properly.  A non-finite
  // value (Inf or NaN) requires that the output be of the same size
  // as the original range with all values set to the non-finite value.
  if (result.rng_numel < 0)
    result.cache = x + r.matrix_value ();

  return result;
}

Range operator + (const Range& r, double x)
{
  Range result (r.base () + x, r.limit () + x, r.inc (), r.numel ());
  if (result.rng_numel < 0)
    result.cache = r.matrix_value () + x;

  return result;
}

Range operator - (double x, const Range& r)
{
  Range result (x - r.base (), x - r.limit (), -r.inc (), r.numel ());
  if (result.rng_numel < 0)
    result.cache = x - r.matrix_value ();

  return result;
}

Range operator - (const Range& r, double x)
{
  Range result (r.base () - x, r.limit () - x, r.inc (), r.numel ());
  if (result.rng_numel < 0)
    result.cache = r.matrix_value () - x;

  return result;
}

Range operator * (double x, const Range& r)
{
  Range result (x * r.base (), x * r.limit (), x * r.inc (), r.numel ());
  if (result.rng_numel < 0)
    result.cache = x * r.matrix_value ();

  return result;
}

Range operator * (const Range& r, double x)
{
  Range result (r.base () * x, r.limit () * x, r.inc () * x, r.numel ());
  if (result.rng_numel < 0)
    result.cache = r.matrix_value () * x;

  return result;
}

// C  See Knuth, Art Of Computer Programming, Vol. 1, Problem 1.2.4-5.
// C
// C===Tolerant FLOOR function.
// C
// C    X  -  is given as a Double Precision argument to be operated on.
// C          It is assumed that X is represented with M mantissa bits.
// C    CT -  is   given   as   a   Comparison   Tolerance   such   that
// C          0.LT.CT.LE.3-SQRT(5)/2. If the relative difference between
// C          X and A whole number is  less  than  CT,  then  TFLOOR  is
// C          returned   as   this   whole   number.   By  treating  the
// C          floating-point numbers as a finite ordered set  note  that
// C          the  heuristic  EPS=2.**(-(M-1))   and   CT=3*EPS   causes
// C          arguments  of  TFLOOR/TCEIL to be treated as whole numbers
// C          if they are  exactly  whole  numbers  or  are  immediately
// C          adjacent to whole number representations.  Since EPS,  the
// C          "distance"  between  floating-point  numbers  on  the unit
// C          interval, and M, the number of bits in X'S mantissa, exist
// C          on  every  floating-point   computer,   TFLOOR/TCEIL   are
// C          consistently definable on every floating-point computer.
// C
// C          For more information see the following references:
// C    (1) P. E. Hagerty, "More On Fuzzy Floor And Ceiling," APL  QUOTE
// C        QUAD 8(4):20-24, June 1978. Note that TFLOOR=FL5.
// C    (2) L. M. Breed, "Definitions For Fuzzy Floor And Ceiling",  APL
// C        QUOTE QUAD 8(3):16-23, March 1978. This paper cites FL1 through
// C        FL5, the history of five years of evolutionary development of
// C        FL5 - the seven lines of code below - by open collaboration
// C        and corroboration of the mathematical-computing community.
// C
// C  Penn State University Center for Academic Computing
// C  H. D. Knoble - August, 1978.

static inline double
tfloor (double x, double ct)
{
// C---------FLOOR(X) is the largest integer algebraically less than
// C         or equal to X; that is, the unfuzzy FLOOR function.

//  DINT (X) = X - DMOD (X, 1.0);
//  FLOOR (X) = DINT (X) - DMOD (2.0 + DSIGN (1.0, X), 3.0);

// C---------Hagerty's FL5 function follows...

  double q = 1.0;

  if (x < 0.0)
    q = 1.0 - ct;

  double rmax = q / (2.0 - ct);

  double t1 = 1.0 + std::floor (x);
  t1 = (ct / q) * (t1 < 0.0 ? -t1 : t1);
  t1 = (rmax < t1 ? rmax : t1);
  t1 = (ct > t1 ? ct : t1);
  t1 = std::floor (x + t1);

  if (x <= 0.0 || (t1 - x) < rmax)
    return t1;
  else
    return t1 - 1.0;
}

static inline bool
teq (double u, double v,
     double ct = 3.0 * std::numeric_limits<double>::epsilon ())
{
  double tu = std::abs (u);
  double tv = std::abs (v);

  return std::abs (u - v) < ((tu > tv ? tu : tv) * ct);
}

octave_idx_type
Range::numel_internal (void) const
{
  octave_idx_type retval = -1;

  if (rng_inc == 0
      || (rng_limit > rng_base && rng_inc < 0)
      || (rng_limit < rng_base && rng_inc > 0))
    {
      retval = 0;
    }
  else
    {
      double ct = 3.0 * std::numeric_limits<double>::epsilon ();

      double tmp = tfloor ((rng_limit - rng_base + rng_inc) / rng_inc, ct);

      octave_idx_type n_elt = (tmp > 0.0 ? static_cast<octave_idx_type> (tmp)
                                         : 0);

      // If the final element that we would compute for the range is equal to
      // the limit of the range, or is an adjacent floating point number,
      // accept it.  Otherwise, try a range with one fewer element.  If that
      // fails, try again with one more element.
      //
      // I'm not sure this is very good, but it seems to work better than just
      // using tfloor as above.  For example, without it, the expression
      // 1.8:0.05:1.9 fails to produce the expected result of [1.8, 1.85, 1.9].

      if (! teq (rng_base + (n_elt - 1) * rng_inc, rng_limit))
        {
          if (teq (rng_base + (n_elt - 2) * rng_inc, rng_limit))
            n_elt--;
          else if (teq (rng_base + n_elt * rng_inc, rng_limit))
            n_elt++;
        }

      retval = (n_elt < std::numeric_limits<octave_idx_type>::max () - 1)
               ? n_elt : -1;
    }

  return retval;
}

double
Range::limit_internal (void) const
{
  double new_limit;

  if (rng_inc > 0)
    new_limit = max ();
  else
    new_limit = min ();

  // If result must be an integer then force the new_limit to be one.
  if (all_elements_are_ints ())
    new_limit = std::round (new_limit);

  return new_limit;
}

void
Range::init (void)
{
  rng_numel = numel_internal ();
  rng_limit = limit_internal ();

  clear_cache ();
}
