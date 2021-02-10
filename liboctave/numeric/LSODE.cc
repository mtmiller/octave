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

#include <cinttypes>
#include <sstream>

#include "LSODE.h"
#include "f77-fcn.h"
#include "lo-error.h"
#include "quit.h"

typedef F77_INT (*lsode_fcn_ptr) (const F77_INT&, const double&, double*,
                                  double*, F77_INT&);

typedef F77_INT (*lsode_jac_ptr) (const F77_INT&, const double&, double*,
                                  const F77_INT&, const F77_INT&, double*,
                                  const F77_INT&);

extern "C"
{
  F77_RET_T
  F77_FUNC (dlsode, DLSODE) (lsode_fcn_ptr, F77_INT&, F77_DBLE*, F77_DBLE&,
                             F77_DBLE&, F77_INT&, F77_DBLE&, const F77_DBLE*,
                             F77_INT&, F77_INT&, F77_INT&, F77_DBLE*,
                             F77_INT&, F77_INT*, F77_INT&, lsode_jac_ptr,
                             F77_INT&);
}

static ODEFunc::ODERHSFunc user_fun;
static ODEFunc::ODEJacFunc user_jac;
static ColumnVector *tmp_x;

static F77_INT
lsode_f (const F77_INT& neq, const double& time, double *, double *deriv,
         F77_INT& ierr)
{
  ColumnVector tmp_deriv;

  // NOTE: this won't work if LSODE passes copies of the state vector.
  //       In that case we have to create a temporary vector object
  //       and copy.

  tmp_deriv = (*user_fun) (*tmp_x, time);

  if (tmp_deriv.isempty ())
    ierr = -1;
  else
    {
      for (F77_INT i = 0; i < neq; i++)
        deriv[i] = tmp_deriv.elem (i);
    }

  return 0;
}

static F77_INT
lsode_j (const F77_INT& neq, const double& time, double *, const F77_INT&,
         const F77_INT&, double *pd, const F77_INT& nrowpd)
{
  Matrix tmp_jac (neq, neq);

  // NOTE: this won't work if LSODE passes copies of the state vector.
  //       In that case we have to create a temporary vector object
  //       and copy.

  tmp_jac = (*user_jac) (*tmp_x, time);

  for (F77_INT j = 0; j < neq; j++)
    for (F77_INT i = 0; i < neq; i++)
      pd[nrowpd * j + i] = tmp_jac (i, j);

  return 0;
}

ColumnVector
LSODE::do_integrate (double tout)
{
  ColumnVector retval;

  static F77_INT nn = 0;

  if (! initialized || restart || ODEFunc::reset || LSODE_options::reset)
    {
      integration_error = false;

      initialized = true;

      istate = 1;

      F77_INT n = octave::to_f77_int (size ());

      nn = n;

      octave_idx_type max_maxord = 0;

      if (integration_method () == "stiff")
        {
          max_maxord = 5;

          if (jac)
            method_flag = 21;
          else
            method_flag = 22;

          liw = 20 + n;
          lrw = 22 + n * (9 + n);
        }
      else
        {
          max_maxord = 12;

          method_flag = 10;

          liw = 20;
          lrw = 22 + 16 * n;
        }

      iwork.resize (dim_vector (liw, 1));

      for (F77_INT i = 4; i < 9; i++)
        iwork(i) = 0;

      rwork.resize (dim_vector (lrw, 1));

      for (F77_INT i = 4; i < 9; i++)
        rwork(i) = 0;

      octave_idx_type maxord = maximum_order ();

      if (maxord >= 0)
        {
          if (maxord > 0 && maxord <= max_maxord)
            {
              iwork(4) = octave::to_f77_int (maxord);
              iopt = 1;
            }
          else
            {
              // FIXME: Should this be a warning?
              (*current_liboctave_error_handler)
                ("lsode: invalid value for maximum order");
              integration_error = true;
              return retval;
            }
        }

      if (stop_time_set)
        {
          itask = 4;
          rwork(0) = stop_time;
          iopt = 1;
        }
      else
        {
          itask = 1;
        }

      restart = false;

      // ODEFunc

      // NOTE: this won't work if LSODE passes copies of the state vector.
      //       In that case we have to create a temporary vector object
      //       and copy.

      tmp_x = &x;

      user_fun = function ();
      user_jac = jacobian_function ();

      ColumnVector xdot = (*user_fun) (x, t);

      if (x.numel () != xdot.numel ())
        {
          // FIXME: Should this be a warning?
          (*current_liboctave_error_handler)
            ("lsode: inconsistent sizes for state and derivative vectors");

          integration_error = true;
          return retval;
        }

      ODEFunc::reset = false;

      // LSODE_options

      rel_tol = relative_tolerance ();
      abs_tol = absolute_tolerance ();

      F77_INT abs_tol_len = octave::to_f77_int (abs_tol.numel ());

      if (abs_tol_len == 1)
        itol = 1;
      else if (abs_tol_len == n)
        itol = 2;
      else
        {
          // FIXME: Should this be a warning?
          (*current_liboctave_error_handler)
            ("lsode: inconsistent sizes for state and absolute tolerance vectors");

          integration_error = true;
          return retval;
        }

      double iss = initial_step_size ();
      if (iss >= 0.0)
        {
          rwork(4) = iss;
          iopt = 1;
        }

      double maxss = maximum_step_size ();
      if (maxss >= 0.0)
        {
          rwork(5) = maxss;
          iopt = 1;
        }

      double minss = minimum_step_size ();
      if (minss >= 0.0)
        {
          rwork(6) = minss;
          iopt = 1;
        }

      F77_INT sl = octave::to_f77_int (step_limit ());
      if (sl > 0)
        {
          iwork(5) = sl;
          iopt = 1;
        }

      LSODE_options::reset = false;
    }

  double *px = x.fortran_vec ();

  double *pabs_tol = abs_tol.fortran_vec ();

  F77_INT *piwork = iwork.fortran_vec ();
  double *prwork = rwork.fortran_vec ();

  F77_INT tmp_istate = octave::to_f77_int (istate);

  F77_XFCN (dlsode, DLSODE, (lsode_f, nn, px, t, tout, itol, rel_tol,
                             pabs_tol, itask, tmp_istate, iopt, prwork, lrw,
                             piwork, liw, lsode_j, method_flag));

  istate = tmp_istate;

  switch (istate)
    {
    case 1:  // prior to initial integration step.
    case 2:  // lsode was successful.
      retval = x;
      t = tout;
      break;

    case -1:  // excess work done on this call (perhaps wrong mf).
    case -2:  // excess accuracy requested (tolerances too small).
    case -3:  // invalid input detected (see printed message).
    case -4:  // repeated error test failures (check all inputs).
    case -5:  // repeated convergence failures (perhaps bad Jacobian
              // supplied or wrong choice of mf or tolerances).
    case -6:  // error weight became zero during problem. (solution
              // component i vanished, and atol or atol(i) = 0.)
    case -13: // return requested in user-supplied function.
      integration_error = true;
      break;

    default:
      integration_error = true;
      (*current_liboctave_error_handler)
        ("unrecognized value of istate (= %" OCTAVE_IDX_TYPE_FORMAT ") "
         "returned from lsode", istate);
      break;
    }

  return retval;
}

std::string
LSODE::error_message (void) const
{
  std::string retval;

  std::ostringstream buf;
  buf << t;
  std::string t_curr = buf.str ();

  switch (istate)
    {
    case 1:
      retval = "prior to initial integration step";
      break;

    case 2:
      retval = "successful exit";
      break;

    case 3:
      retval = "prior to continuation call with modified parameters";
      break;

    case -1:
      retval = "excess work on this call (t = " + t_curr +
               "; perhaps wrong integration method)";
      break;

    case -2:
      retval = "excess accuracy requested (tolerances too small)";
      break;

    case -3:
      retval = "invalid input detected (see printed message)";
      break;

    case -4:
      retval = "repeated error test failures (t = " + t_curr +
               "; check all inputs)";
      break;

    case -5:
      retval = "repeated convergence failures (t = " + t_curr +
               "; perhaps bad Jacobian supplied or wrong choice of integration method or tolerances)";
      break;

    case -6:
      retval = "error weight became zero during problem. (t = " + t_curr +
               "; solution component i vanished, and atol or atol(i) == 0)";
      break;

    case -13:
      retval = "return requested in user-supplied function (t = "
               + t_curr + ')';
      break;

    default:
      retval = "unknown error state";
      break;
    }

  return retval;
}

Matrix
LSODE::do_integrate (const ColumnVector& tout)
{
  Matrix retval;

  octave_idx_type n_out = tout.numel ();
  F77_INT n = octave::to_f77_int (size ());

  if (n_out > 0 && n > 0)
    {
      retval.resize (n_out, n);

      for (F77_INT i = 0; i < n; i++)
        retval.elem (0, i) = x.elem (i);

      for (octave_idx_type j = 1; j < n_out; j++)
        {
          ColumnVector x_next = do_integrate (tout.elem (j));

          if (integration_error)
            return retval;

          for (F77_INT i = 0; i < n; i++)
            retval.elem (j, i) = x_next.elem (i);
        }
    }

  return retval;
}

Matrix
LSODE::do_integrate (const ColumnVector& tout, const ColumnVector& tcrit)
{
  Matrix retval;

  octave_idx_type n_out = tout.numel ();
  F77_INT n = octave::to_f77_int (size ());

  if (n_out > 0 && n > 0)
    {
      retval.resize (n_out, n);

      for (F77_INT i = 0; i < n; i++)
        retval.elem (0, i) = x.elem (i);

      octave_idx_type n_crit = tcrit.numel ();

      if (n_crit > 0)
        {
          octave_idx_type i_crit = 0;
          octave_idx_type i_out = 1;
          double next_crit = tcrit.elem (0);
          double next_out;
          while (i_out < n_out)
            {
              bool do_restart = false;

              next_out = tout.elem (i_out);
              if (i_crit < n_crit)
                next_crit = tcrit.elem (i_crit);

              bool save_output = false;
              double t_out;

              if (next_crit == next_out)
                {
                  set_stop_time (next_crit);
                  t_out = next_out;
                  save_output = true;
                  i_out++;
                  i_crit++;
                  do_restart = true;
                }
              else if (next_crit < next_out)
                {
                  if (i_crit < n_crit)
                    {
                      set_stop_time (next_crit);
                      t_out = next_crit;
                      save_output = false;
                      i_crit++;
                      do_restart = true;
                    }
                  else
                    {
                      clear_stop_time ();
                      t_out = next_out;
                      save_output = true;
                      i_out++;
                    }
                }
              else
                {
                  set_stop_time (next_crit);
                  t_out = next_out;
                  save_output = true;
                  i_out++;
                }

              ColumnVector x_next = do_integrate (t_out);

              if (integration_error)
                return retval;

              if (save_output)
                {
                  for (F77_INT i = 0; i < n; i++)
                    retval.elem (i_out-1, i) = x_next.elem (i);
                }

              if (do_restart)
                force_restart ();
            }
        }
      else
        {
          retval = do_integrate (tout);

          if (integration_error)
            return retval;
        }
    }

  return retval;
}
