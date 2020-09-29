////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2020 The Octave Project Developers
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

#include <string>

#include "DASSL.h"

#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "interpreter-private.h"
#include "ovl.h"
#include "ov-fcn.h"
#include "ov-cell.h"
#include "pager.h"
#include "parse.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

#include "DASSL-opts.cc"

// Global pointer for user defined function required by dassl.
static octave_value dassl_fcn;

// Global pointer for optional user defined jacobian function.
static octave_value dassl_jac;

// Have we warned about imaginary values returned from user function?
static bool warned_fcn_imaginary = false;
static bool warned_jac_imaginary = false;

// Is this a recursive call?
static int call_depth = 0;

ColumnVector
dassl_user_function (const ColumnVector& x, const ColumnVector& xdot,
                     double t, octave_idx_type& ires)
{
  ColumnVector retval;

  assert (x.numel () == xdot.numel ());

  octave_value_list args;

  args(2) = t;
  args(1) = xdot;
  args(0) = x;

  if (dassl_fcn.is_defined ())
    {
      octave_value_list tmp;

      try
        {
          tmp = octave::feval (dassl_fcn, args, 1);
        }
      catch (octave::execution_exception& e)
        {
          err_user_supplied_eval (e, "dassl");
        }

      int tlen = tmp.length ();
      if (tlen == 0 || ! tmp(0).is_defined ())
        err_user_supplied_eval ("dassl");

      if (! warned_fcn_imaginary && tmp(0).iscomplex ())
        {
          warning ("dassl: ignoring imaginary part returned from user-supplied function");
          warned_fcn_imaginary = true;
        }

      retval = tmp(0).vector_value ();

      if (tlen > 1)
        ires = tmp(1).int_value ();

      if (retval.isempty ())
        err_user_supplied_eval ("dassl");
    }

  return retval;
}

Matrix
dassl_user_jacobian (const ColumnVector& x, const ColumnVector& xdot,
                     double t, double cj)
{
  Matrix retval;

  assert (x.numel () == xdot.numel ());

  octave_value_list args;

  args(3) = cj;
  args(2) = t;
  args(1) = xdot;
  args(0) = x;

  if (dassl_jac.is_defined ())
    {
      octave_value_list tmp;

      try
        {
          tmp = octave::feval (dassl_jac, args, 1);
        }
      catch (octave::execution_exception& e)
        {
          err_user_supplied_eval (e, "dassl");
        }

      int tlen = tmp.length ();
      if (tlen == 0 || ! tmp(0).is_defined ())
        err_user_supplied_eval ("dassl");

      if (! warned_jac_imaginary && tmp(0).iscomplex ())
        {
          warning ("dassl: ignoring imaginary part returned from user-supplied jacobian function");
          warned_jac_imaginary = true;
        }

      retval = tmp(0).matrix_value ();

      if (retval.isempty ())
        err_user_supplied_eval ("dassl");
    }

  return retval;
}

DEFMETHOD (dassl, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn {} {[@var{x}, @var{xdot}, @var{istate}, @var{msg}] =} dassl (@var{fcn}, @var{x_0}, @var{xdot_0}, @var{t}, @var{t_crit})
Solve a set of differential-algebraic equations.

@code{dassl} solves the set of equations
@tex
$$ 0 = f (x, \dot{x}, t) $$
with
$$ x(t_0) = x_0, \dot{x}(t_0) = \dot{x}_0 $$
@end tex
@ifnottex

@example
0 = f (x, xdot, t)
@end example

@noindent
with

@example
x(t_0) = x_0, xdot(t_0) = xdot_0
@end example

@end ifnottex
The solution is returned in the matrices @var{x} and @var{xdot},
with each row in the result matrices corresponding to one of the
elements in the vector @var{t}.  The first element of @var{t}
should be @math{t_0} and correspond to the initial state of the
system @var{x_0} and its derivative @var{xdot_0}, so that the first
row of the output @var{x} is @var{x_0} and the first row
of the output @var{xdot} is @var{xdot_0}.

The first argument, @var{fcn}, is a string, inline, or function handle
that names the function @math{f} to call to compute the vector of
residuals for the set of equations.  It must have the form

@example
@var{res} = f (@var{x}, @var{xdot}, @var{t})
@end example

@noindent
in which @var{x}, @var{xdot}, and @var{res} are vectors, and @var{t} is a
scalar.

If @var{fcn} is a two-element string array or a two-element cell array
of strings, inline functions, or function handles, the first element names
the function @math{f} described above, and the second element names a
function to compute the modified Jacobian

@tex
$$
J = {\partial f \over \partial x}
  + c {\partial f \over \partial \dot{x}}
$$
@end tex
@ifnottex

@example
@group
      df       df
jac = -- + c ------
      dx     d xdot
@end group
@end example

@end ifnottex

The modified Jacobian function must have the form

@example
@group

@var{jac} = j (@var{x}, @var{xdot}, @var{t}, @var{c})

@end group
@end example

The second and third arguments to @code{dassl} specify the initial
condition of the states and their derivatives, and the fourth argument
specifies a vector of output times at which the solution is desired,
including the time corresponding to the initial condition.

The set of initial states and derivatives are not strictly required to
be consistent.  In practice, however, @sc{dassl} is not very good at
determining a consistent set for you, so it is best if you ensure that
the initial values result in the function evaluating to zero.

The fifth argument is optional, and may be used to specify a set of
times that the DAE solver should not integrate past.  It is useful for
avoiding difficulties with singularities and points where there is a
discontinuity in the derivative.

After a successful computation, the value of @var{istate} will be
greater than zero (consistent with the Fortran version of @sc{dassl}).

If the computation is not successful, the value of @var{istate} will be
less than zero and @var{msg} will contain additional information.

You can use the function @code{dassl_options} to set optional
parameters for @code{dassl}.
@seealso{daspk, dasrt, lsode}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 4 || nargin > 5)
    print_usage ();

  warned_fcn_imaginary = false;
  warned_jac_imaginary = false;

  octave_value_list retval (4);

  octave::unwind_protect_var<int> restore_var (call_depth);
  call_depth++;

  if (call_depth > 1)
    error ("dassl: invalid recursive call");

  std::string fcn_name, fname, jac_name, jname;

  dassl_fcn = octave_value ();
  dassl_jac = octave_value ();

  octave_value f_arg = args(0);

  std::list<std::string> fcn_param_names ({"x", "xdot", "t"});
  std::list<std::string> jac_param_names ({"x", "xdot", "t", "cj"});

  if (f_arg.iscell ())
    {
      Cell c = f_arg.cell_value ();
      if (c.numel () == 1)
        f_arg = c(0);
      else if (c.numel () == 2)
        {
          dassl_fcn = octave::get_function_handle (interp, c(0),
                                                   fcn_param_names);

          if (dassl_fcn.is_defined ())
            {
              dassl_jac = octave::get_function_handle (interp, c(1),
                                                       jac_param_names);

              if (dassl_jac.is_undefined ())
                dassl_fcn = octave_value ();
            }
        }
      else
        error ("dassl: incorrect number of elements in cell array");
    }

  if (dassl_fcn.is_undefined () && ! f_arg.iscell ())
    {
      if (f_arg.is_function_handle () || f_arg.is_inline_function ())
        dassl_fcn = f_arg;
      else
        {
          switch (f_arg.rows ())
            {
            case 1:
              dassl_fcn = octave::get_function_handle (interp, f_arg,
                                                       fcn_param_names);
              break;

            case 2:
              {
                string_vector tmp = f_arg.string_vector_value ();

                dassl_fcn = octave::get_function_handle (interp, tmp(0),
                                                         fcn_param_names);

                if (dassl_fcn.is_defined ())
                  {
                    dassl_jac = octave::get_function_handle (interp, tmp(1),
                                                             jac_param_names);

                    if (dassl_jac.is_undefined ())
                      dassl_fcn = octave_value ();
                  }
              }
              break;

            default:
              error ("dassl: first arg should be a string or 2-element string array");
            }
        }
    }

  if (dassl_fcn.is_undefined ())
    error ("dassl: FCN argument is not a valid function name or handle");

  ColumnVector state = args(1).xvector_value ("dassl: initial state X_0 must be a vector");

  ColumnVector deriv = args(2).xvector_value ("dassl: initial derivatives XDOT_0 must be a vector");

  ColumnVector out_times = args(3).xvector_value ("dassl: output time variable T must be a vector");

  ColumnVector crit_times;
  int crit_times_set = 0;
  if (nargin > 4)
    {
      crit_times = args(4).xvector_value ("dassl: list of critical times T_CRIT must be a vector");

      crit_times_set = 1;
    }

  if (state.numel () != deriv.numel ())
    error ("dassl: X and XDOT_0 must have the same size");

  double tzero = out_times (0);

  DAEFunc func (dassl_user_function);
  if (dassl_jac.is_defined ())
    func.set_jacobian_function (dassl_user_jacobian);

  DASSL dae (state, deriv, tzero, func);

  dae.set_options (dassl_opts);

  Matrix output;
  Matrix deriv_output;

  if (crit_times_set)
    output = dae.integrate (out_times, deriv_output, crit_times);
  else
    output = dae.integrate (out_times, deriv_output);

  std::string msg = dae.error_message ();

  if (dae.integration_ok ())
    {
      retval(0) = output;
      retval(1) = deriv_output;
    }
  else
    {
      if (nargout < 3)
        error ("dassl: %s", msg.c_str ());

      retval(0) = Matrix ();
      retval(1) = Matrix ();
    }

  retval(2) = static_cast<double> (dae.integration_state ());
  retval(3) = msg;

  return retval;
}

/*
## dassl-1.m
##
## Test dassl() function
##
## Author: David Billinghurst (David.Billinghurst@riotinto.com.au)
##         Comalco Research and Technology
##         20 May 1998
##
## Problem
##
##    y1' = -y2,   y1(0) = 1
##    y2' =  y1,   y2(0) = 0
##
## Solution
##
##    y1(t) = cos(t)
##    y2(t) = sin(t)
##
%!function res = __f (x, xdot, t)
%!  res = [xdot(1)+x(2); xdot(2)-x(1)];
%!endfunction

%!test
%!
%! x0 = [1; 0];
%! xdot0 = [0; 1];
%! t = (0:1:10)';
%!
%! tol = 100 * dassl_options ("relative tolerance");
%!
%! [x, xdot] = dassl ("__f", x0, xdot0, t);
%!
%! y = [cos(t), sin(t)];
%!
%! assert (x, y, tol);

## dassl-2.m
##
## Test dassl() function
##
## Author: David Billinghurst (David.Billinghurst@riotinto.com.au)
##         Comalco Research and Technology
##         20 May 1998
##
## Based on SLATEC quick check for DASSL by Linda Petzold
##
## Problem
##
##   x1' + 10*x1 = 0,   x1(0) = 1
##   x1  + x2    = 1,   x2(0) = 0
##
##
## Solution
##
##  x1(t) = exp(-10*t)
##  x2(t) = 1 - x(1)
##
%!function res = __f (x, xdot, t)
%!  res = [xdot(1)+10*x(1); x(1)+x(2)-1];
%!endfunction

%!test
%!
%! x0 = [1; 0];
%! xdot0 = [-10; 10];
%! t = (0:0.2:1)';
%!
%! tol = 500 * dassl_options ("relative tolerance");
%!
%! [x, xdot] = dassl ("__f", x0, xdot0, t);
%!
%! y = [exp(-10*t), 1-exp(-10*t)];
%!
%! assert (x, y, tol);

%!test
%! old_tol = dassl_options ("absolute tolerance");
%! dassl_options ("absolute tolerance", eps);
%! assert (dassl_options ("absolute tolerance") == eps);
%! ## Restore old value of tolerance
%! dassl_options ("absolute tolerance", old_tol);

%!error dassl_options ("foo", 1, 2)
*/
