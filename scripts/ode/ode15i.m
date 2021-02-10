########################################################################
##
## Copyright (C) 2016-2021 The Octave Project Developers
##
## See the file COPYRIGHT.md in the top-level directory of this
## distribution or <https://octave.org/copyright/>.
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.
##
########################################################################

## -*- texinfo -*-
## @deftypefn  {} {[@var{t}, @var{y}] =} ode15i (@var{fun}, @var{trange}, @var{y0}, @var{yp0})
## @deftypefnx {} {[@var{t}, @var{y}] =} ode15i (@var{fun}, @var{trange}, @var{y0}, @var{yp0}, @var{ode_opt})
## @deftypefnx {} {[@var{t}, @var{y}, @var{te}, @var{ye}, @var{ie}] =} ode15i (@dots{})
## @deftypefnx {} {@var{solution} =} ode15i (@dots{})
## @deftypefnx {} {} ode15i (@dots{})
## Solve a set of fully-implicit Ordinary Differential Equations (ODEs) or
## index 1 Differential Algebraic Equations (DAEs).
##
## @code{ode15i} uses a variable step, variable order BDF (Backward
## Differentiation Formula) method that ranges from order 1 to 5.
##
## @var{fun} is a function handle, inline function, or string containing the
## name of the function that defines the ODE: @code{0 = f(t,y,yp)}.  The
## function must accept three inputs where the first is time @var{t}, the
## second is the function value @var{y} (a column vector), and the third
## is the derivative value @var{yp} (a column vector).
##
## @var{trange} specifies the time interval over which the ODE will be
## evaluated.  Typically, it is a two-element vector specifying the initial and
## final times (@code{[tinit, tfinal]}).  If there are more than two elements
## then the solution will also be evaluated at these intermediate time
## instances.
##
## @var{y0} and @var{yp0} contain the initial values for the unknowns @var{y}
## and @var{yp}.  If they are row vectors then the solution @var{y} will be a
## matrix in which each column is the solution for the corresponding initial
## value in @var{y0} and @var{yp0}.
##
## @var{y0} and @var{yp0} must be consistent initial conditions, meaning that
## @code{f(t,y0,yp0) = 0} is satisfied.  The function @code{decic} may be used
## to compute consistent initial conditions given initial guesses.
##
## The optional fifth argument @var{ode_opt} specifies non-default options to
## the ODE solver.  It is a structure generated by @code{odeset}.
##
## The function typically returns two outputs.  Variable @var{t} is a
## column vector and contains the times where the solution was found.  The
## output @var{y} is a matrix in which each column refers to a different
## unknown of the problem and each row corresponds to a time in @var{t}.
##
## The output can also be returned as a structure @var{solution} which has a
## field @var{x} containing a row vector of times where the solution was
## evaluated and a field @var{y} containing the solution matrix such that each
## column corresponds to a time in @var{x}.  Use
## @w{@code{fieldnames (@var{solution})}} to see the other fields and
## additional information returned.
##
## If no output arguments are requested, and no @qcode{"OutputFcn"} is
## specified in @var{ode_opt}, then the @qcode{"OutputFcn"} is set to
## @code{odeplot} and the results of the solver are plotted immediately.
##
## If using the @qcode{"Events"} option then three additional outputs may be
## returned.  @var{te} holds the time when an Event function returned a zero.
## @var{ye} holds the value of the solution at time @var{te}.  @var{ie}
## contains an index indicating which Event function was triggered in the case
## of multiple Event functions.
##
## Example: Solve @nospell{Robertson's} equations:
##
## @smallexample
## @group
## function r = robertson_dae (@var{t}, @var{y}, @var{yp})
##   r = [ -(@var{yp}(1) + 0.04*@var{y}(1) - 1e4*@var{y}(2)*@var{y}(3))
##         -(@var{yp}(2) - 0.04*@var{y}(1) + 1e4*@var{y}(2)*@var{y}(3) + 3e7*@var{y}(2)^2)
##         @var{y}(1) + @var{y}(2) + @var{y}(3) - 1 ];
## endfunction
## [@var{t},@var{y}] = ode15i (@@robertson_dae, [0, 1e3], [1; 0; 0], [-1e-4; 1e-4; 0]);
## @end group
## @end smallexample
## @seealso{decic, odeset, odeget}
## @end deftypefn

function varargout = ode15i (fun, trange, y0, yp0, varargin)

  if (nargin < 4)
    print_usage ();
  endif

  solver = "ode15i";

  n = numel (y0);

  if (nargin > 4)
   options = varargin{1};
  else
   options = odeset ();
  endif

  ## Check fun, trange, y0, yp0
  fun = check_default_input (fun, trange, solver, y0, yp0);

  if (! isempty (options.Jacobian))
    if (ischar (options.Jacobian))
      if (! exist (options.Jacobian))
        error ("Octave:invalid-input-arg",
               ['ode15i: "Jacobian" function "' options.Jacobian '" not found']);
      endif
      options.Jacobian = str2func (options.Jacobian);
    endif
  endif

  if (! isempty (options.OutputFcn))
    if (ischar (options.OutputFcn))
      if (! exist (options.OutputFcn))
        error ("Octave:invalid-input-arg",
               ['ode15i: "OutputFcn" function "' options.OutputFcn '" not found']);
      endif
      options.OutputFcn = str2func (options.OutputFcn);
    endif
    if (! is_function_handle (options.OutputFcn))
      error ("Octave:invalid-input-arg",
             'ode15i: "OutputFcn" must be a valid function handle');
    endif
  endif

  if (! isempty (options.Events))
    if (ischar (options.Events))
      if (! exist (options.Events))
        error ("Octave:invalid-input-arg",
               ['ode15i: "Events" function "' options.Events '" not found']);
      endif
      options.Events = str2func (options.Events);
    endif
    if (! is_function_handle (options.Events))
      error ("Octave:invalid-input-arg",
             'ode15i: "Events" must be a valid function handle');
    endif
  endif

  [defaults, classes, attributes] = odedefaults (n, trange(1), trange(end));

  persistent ignorefields = {"NonNegative", "Mass", ...
                             "MStateDependence", "MvPattern", ...
                             "MassSingular", "InitialSlope", "BDF"};

  defaults   = rmfield (defaults, ignorefields);
  classes    = rmfield (classes, ignorefields);
  attributes = rmfield (attributes, ignorefields);

  classes    = odeset (classes, "Vectorized", {});
  attributes = odeset (attributes, "Jacobian", {}, "Vectorized", {});

  options = odemergeopts ("ode15i", options, defaults,
                          classes, attributes, solver);

  ## Jacobian
  options.havejac       = false;
  options.havejacsparse = false;
  options.havejacfun    = false;

  if (! isempty (options.Jacobian))
    options.havejac = true;
    if (iscell (options.Jacobian))
      if (numel (options.Jacobian) == 2)
        J1 = options.Jacobian{1};
        J2 = options.Jacobian{2};
        if (   ! issquare (J1) || ! issquare (J2)
            || rows (J1) != n || rows (J2) != n
            || ! isnumeric (J1) || ! isnumeric (J2)
            || ! isreal (J1) || ! isreal (J2))
          error ("Octave:invalid-input-arg",
                 'ode15i: "Jacobian" matrices must be real square matrices');
        endif
        if (issparse (J1) && issparse (J2))
          options.havejacsparse = true;  # Jac is sparse cell
        endif
      else
        error ("Octave:invalid-input-arg",
               'ode15i: invalid value assigned to field "Jacobian"');
      endif

    elseif (is_function_handle (options.Jacobian))
      options.havejacfun = true;
      if (nargin (options.Jacobian) == 3)
        [J1, J2] = options.Jacobian (trange(1), y0, yp0);

        if (   ! issquare (J1) || rows (J1) != n
            || ! isnumeric (J1) || ! isreal (J1)
            || ! issquare (J2) || rows (J2) != n
            || ! isnumeric (J2) || ! isreal (J2))
          error ("Octave:invalid-input-arg",
                 'ode15i: "Jacobian" function must evaluate to a real square matrix');
        endif
        if (issparse (J1) && issparse (J2))
          options.havejacsparse = true;  # Jac is sparse fun
        endif
      else
        error ("Octave:invalid-input-arg",
               'ode15i: invalid value assigned to field "Jacobian"');
      endif
    else
      error ("Octave:invalid-input-arg",
             'ode15i: "Jacobian" field must be a function handle or 2-element cell array of square matrices');
    endif
  endif

  ## Abstol and Reltol
  options.haveabstolvec = false;

  if (numel (options.AbsTol) != 1 && numel (options.AbsTol) != n)
    error ("Octave:invalid-input-arg",
           'ode15i: invalid value assigned to field "AbsTol"');

  elseif (numel (options.AbsTol) == n)
    options.haveabstolvec = true;
  endif

  ## Stats
  options.havestats = strcmpi (options.Stats, "on");

  ## Don't use Refine when the output is a structure
  if (nargout == 1)
    options.Refine = 1;
  endif

  ## OutputFcn and OutputSel
  if (isempty (options.OutputFcn) && nargout == 0)
    options.OutputFcn = @odeplot;
    options.haveoutputfunction = true;
  else
    options.haveoutputfunction = ! isempty (options.OutputFcn);
  endif

  options.haveoutputselection = ! isempty (options.OutputSel);
  if (options.haveoutputselection)
    options.OutputSel = options.OutputSel - 1;
  endif

  ## Events
  options.haveeventfunction = ! isempty (options.Events);

  ## 3 arguments in the event callback of ode15i
  [t, y, te, ye, ie] = __ode15__ (fun, trange, y0, yp0, options, 3);

  if (nargout == 2)
    varargout{1} = t;
    varargout{2} = y;
  elseif (nargout == 1)
    varargout{1}.x = t.';  # Time stamps saved in field x (row vector)
    varargout{1}.y = y.';  # Results are saved in field y (row vector)
    varargout{1}.solver = solver;
    if (options.haveeventfunction)
      varargout{1}.xe = te.';  # Time info when an event occurred
      varargout{1}.ye = ye.';  # Results when an event occurred
      varargout{1}.ie = ie.';  # Index info which event occurred
    endif
  elseif (nargout > 2)
    varargout = cell (1,5);
    varargout{1} = t;
    varargout{2} = y;
    if (options.haveeventfunction)
      varargout{3} = te;  # Time info when an event occurred
      varargout{4} = ye;  # Results when an event occurred
      varargout{5} = ie;  # Index info which event occurred
    endif
  endif

endfunction


%!demo
%! ## Solve Robertson's equations with ode15i
%! fun = @(t, y, yp) [-(yp(1) + 0.04*y(1) - 1e4*y(2)*y(3));
%!                    -(yp(2) - 0.04*y(1) + 1e4*y(2)*y(3) + 3e7*y(2)^2);
%!                    y(1) + y(2) + y(3) - 1];
%!
%! opt = odeset ("RelTol", 1e-4, "AbsTol", [1e-8, 1e-14, 1e-6]);
%! y0 = [1; 0; 0];
%! yp0 = [-1e-4; 1e-4; 0];
%! tspan = [0 4*logspace(-6, 6)];
%!
%! [t, y] = ode15i (fun, tspan, y0, yp0, opt);
%!
%! y(:,2) = 1e4 * y(:, 2);
%! figure (2);
%! semilogx (t, y, "o");
%! xlabel ("time");
%! ylabel ("species concentration");
%! title ("Robertson DAE problem with a Conservation Law");
%! legend ("y1", "y2", "y3");

%!function res = rob (t, y, yp)
%!  res =[-(yp(1) + 0.04*y(1) - 1e4*y(2)*y(3));
%!        -(yp(2) - 0.04*y(1) + 1e4*y(2)*y(3) + 3e7*y(2)^2);
%!        y(1) + y(2) + y(3) - 1];
%!endfunction
%!
%!function ref = fref ()
%!  ref = [100, 0.617234887614937, 0.000006153591397, 0.382758958793666];
%!endfunction
%!
%!function ref2 = fref2 ()
%!  ref2 = [4e6 0 0 1];
%!endfunction
%!
%!function [DFDY, DFDYP] = jacfundense (t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3),  1e4*y(2);
%!           0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!              1,                  1,         1];
%!  DFDYP = [-1,  0, 0;
%!            0, -1, 0;
%!            0,  0, 0];
%!endfunction
%!
%!function [DFDY, DFDYP] = jacfunsparse (t, y, yp)
%!  DFDY = sparse ([-0.04,           1e4*y(3),  1e4*y(2);
%!                   0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!                      1,                  1,         1]);
%!  DFDYP = sparse ([-1,  0, 0;
%!                    0, -1, 0;
%!                    0,  0, 0]);
%!endfunction
%!
%!function [DFDY, DFDYP] = jacwrong (t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3);
%!           0.04, -1e4*y(3)-6e7*y(2)];
%!  DFDYP = [-1,  0;
%!            0, -1];
%!endfunction
%!
%!function [DFDY, DFDYP, A] = jacwrong2 (t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3),  1e4*y(2);
%!           0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!              1,                  1,         1];
%!  DFDYP = [-1,  0, 0;
%!            0, -1, 0;
%!            0,  0, 0];
%!  A = DFDY;
%!endfunction
%!
%!function [val, isterminal, direction] = ff (t, y, yp)
%!  isterminal = [0, 1];
%!  if (t < 1e1)
%!    val = [-1, -2];
%!  else
%!    val = [1, 3];
%!  endif
%!
%!  direction = [1, 0];
%!endfunction

## anonymous function instead of real function
%!testif HAVE_SUNDIALS
%! ref = 0.049787079136413;
%! ff = @(t, u, udot)  udot + 3 * u;
%! [t, y] = ode15i (ff, 0:1, 1, -3);
%! assert ([t(end), y(end)], [1, ref], 1e-3);

## function passed as string
%!testif HAVE_SUNDIALS
%! [t, y] = ode15i ("rob", [0, 100, 200], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! assert ([t(2), y(2,:)], fref, 1e-3);

##  solve in intermediate step
%!testif HAVE_SUNDIALS
%! [t, y] = ode15i (@rob, [0, 100, 200], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! assert ([t(2), y(2,:)], fref, 1e-3);

## numel(trange) = 2 final value
%!testif HAVE_SUNDIALS
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! assert ([t(end), y(end,:)], fref, 1e-5);

## With empty options
%!testif HAVE_SUNDIALS
%! opt = odeset ();
%! [t, y] = ode15i (@rob, [0, 1e6, 2e6, 3e6, 4e6], [1; 0; 0],
%!                  [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref2, 1e-3);
%! opt = odeset ();

## Without options
%!testif HAVE_SUNDIALS
%! [t, y] = ode15i (@rob, [0, 1e6, 2e6, 3e6, 4e6], [1; 0; 0],[-1e-4; 1e-4; 0]);
%! assert ([t(end), y(end,:)], fref2, 1e-3);

## InitialStep option
%!testif HAVE_SUNDIALS
%! opt = odeset ("InitialStep", 1e-8);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert (t(2)-t(1), 1e-8, 1e-9);

## MaxStep option
%!testif HAVE_SUNDIALS
%! opt = odeset ("MaxStep", 1e-3);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! assert (t(5)-t(4), 1e-3, 1e-3);

## AbsTol scalar option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", 1e-8);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## AbsTol scalar and RelTol option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", 1e-8, "RelTol", 1e-6);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## AbsTol vector option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", [1e-8, 1e-14, 1e-6]);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## AbsTol vector and RelTol option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", [1e-8, 1e-14,1e-6], "RelTol", 1e-6);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## RelTol option
%!testif HAVE_SUNDIALS
%! opt = odeset ("RelTol", 1e-6);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## Jacobian fun dense
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", @jacfundense);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## Jacobian fun dense as string
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", "jacfundense");
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## Jacobian fun sparse
%!testif HAVE_SUNDIALS_SUNLINSOL_KLU
%! opt = odeset ("Jacobian", @jacfunsparse, "AbsTol", 1e-7, "RelTol", 1e-7);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);

## Solve in backward direction starting at t=100
%!testif HAVE_SUNDIALS
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! [t2, y2] = ode15i (@rob, [100, 0], Yref', YPref);
%! assert ([t2(end), y2(end,:)], [0, 1, 0, 0], 2e-2);

## Solve in backward direction with MaxStep option
#%!testif HAVE_SUNDIALS
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! opt = odeset ("MaxStep", 1e-2);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! [t2, y2] = ode15i (@rob, [100, 0], Yref', YPref, opt);
%! assert ([t2(end), y2(end,:)], [0, 1, 0, 0], 2e-2);
%! assert (t2(9)-t2(10), 1e-2, 1e-2);

## Solve in backward direction starting with intermediate step
#%!testif HAVE_SUNDIALS
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! [t2, y2] = ode15i (@rob, [100, 5, 0], Yref', YPref);
%! assert ([t2(end), y2(end,:)], [0, 1, 0, 0], 2e-2);

## Refine
%!testif HAVE_SUNDIALS
%! opt = odeset ("Refine", 3);
%! [t, y] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! [t2, y2] = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert (numel (t2), numel (t) * 3, 3);

## Refine ignored if numel (trange) > 2
%!testif HAVE_SUNDIALS
%! opt = odeset ("Refine", 3);
%! [t, y] = ode15i (@rob, [0, 10, 100], [1; 0; 0], [-1e-4; 1e-4; 0]);
%! [t2, y2] = ode15i (@rob, [0, 10, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert (numel (t2), numel (t));

## Events option add further elements in sol
%!testif HAVE_SUNDIALS
%! opt = odeset ("Events", @ff);
%! sol = ode15i (@rob, [0, 100], [1; 0; 0], [-1e-4; 1e-4; 0], opt);
%! assert (isfield (sol, "ie"));
%! assert (sol.ie, [1, 2]);
%! assert (isfield (sol, "xe"));
%! assert (isfield (sol, "ye"));
%! assert (sol.x(end), 10, 1);

## Events option, five output arguments
%!testif HAVE_SUNDIALS
%! opt = odeset ("Events", @ff);
%! [t, y, te, ye, ie] = ode15i (@rob, [0, 100], [1; 0; 0],
%!                              [-1e-4; 1e-4; 0], opt);
%! assert (t(end), 10, 1);
%! assert (te, [10; 10], 0.2);
%! assert (ie, [1; 2]);

## Initial solutions as row vectors
%!testif HAVE_SUNDIALS
%! A = eye (2);
%! [tout, yout] = ode15i (@(t, y, yp) A * y - A * yp, ...
%! [0, 1], [1, 1], [1, 1]);
%! assert (size (yout), [20, 2]);

%!testif HAVE_SUNDIALS
%! A = eye (2);
%! [tout, yout] = ode15i (@(t, y, yp) A * y - A * yp, ...
%! [0, 1], [1, 1], [1; 1]);
%! assert (size (yout), [20, 2]);

## Jacobian fun wrong dimension
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", @jacwrong);
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" function must evaluate to a real square matrix');

## Jacobian cell dense wrong dimension
%!testif HAVE_SUNDIALS
%! DFDY = [-0.04, 1;
%!          0.04, 1];
%! DFDYP = [-1,  0, 0;
%!           0, -1, 0;
%!           0,  0, 0];
%! opt = odeset ("Jacobian", {DFDY, DFDYP});
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" matrices must be real square matrices');

## Jacobian cell sparse wrong dimension
%!testif HAVE_SUNDIALS_SUNLINSOL_KLU
%! DFDY = sparse ([-0.04, 1;
%!                  0.04, 1]);
%! DFDYP = sparse ([-1,  0, 0;
%!                   0, -1, 0;
%!                   0,  0, 0]);
%! opt = odeset ("Jacobian", {DFDY, DFDYP});
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" matrices must be real square matrices');

## Jacobian cell wrong number of matrices
%!testif HAVE_SUNDIALS
%! A = [1 2 3; 4 5 6; 7 8 9];
%! opt = odeset ("Jacobian", {A,A,A});
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       'invalid value assigned to field "Jacobian"');

## Jacobian single matrix
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", [1 2 3; 4 5 6; 7 8 9]);
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" field must be a function handle or 2-element cell array of square matrices');

## Jacobian single matrix wrong dimension
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", [1 2 3; 4 5 6]);
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" field must be a function handle or 2-element cell array of square matrices');

## Jacobian strange field
%!testif HAVE_SUNDIALS
%! opt = odeset ("Jacobian", "_5yVNhWVJWJn47RKnzxPsyb_");
%! fail ("[t, y] = ode15i (@rob, [0, 4e6], [1; 0; 0], [-1e-4; 1e-4; 0], opt)",
%!       '"Jacobian" function "_5yVNhWVJWJn47RKnzxPsyb_" not found');

%!function ydot = fun (t, y, yp)
%!  ydot = [y - yp];
%!endfunction

%!testif HAVE_SUNDIALS
%! fail ("ode15i ()", "Invalid call to ode15i");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1)", "Invalid call to ode15i");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1, 1)", "Invalid call to ode15i");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1, 1, 1)", "Invalid call to ode15i");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1, 1, 1, 1)", "ode15i: fun must be of class:");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1, 1, 1, 1, 1)", "ode15i: fun must be of class:");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (1, 1, 1, 1, 1, 1)", "ode15i: fun must be of class:");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (@fun, 1, 1, 1)",
%!       "ode15i: invalid value assigned to field 'trange'");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (@fun, [1, 1], 1, 1)",
%!       "ode15i: invalid value assigned to field 'trange'");

%!testif HAVE_SUNDIALS
%! fail ("ode15i (@fun, [1, 2], 1, [1, 2])",
%!       "ode15i: y0 must have 2 elements");

%!testif HAVE_SUNDIALS
%! opt = odeset ("RelTol", "_5yVNhWVJWJn47RKnzxPsyb_");
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       "ode15i: RelTol must be of class:");

%!testif HAVE_SUNDIALS
%! opt = odeset ("RelTol", [1, 2]);
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       "ode15i: RelTol must be scalar");

%!testif HAVE_SUNDIALS
%! opt = odeset ("RelTol", -2);
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       "ode15i: RelTol must be positive");

%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", "_5yVNhWVJWJn47RKnzxPsyb_");
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       "ode15i: AbsTol must be of class:");

%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", -1);
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       "ode15i: AbsTol must be positive");

%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", [1, 1, 1]);
%! fail ("[t, y] = ode15i (@fun, [0, 2], 2, 2, opt)",
%!       'ode15i: invalid value assigned to field "AbsTol"');

%!testif HAVE_SUNDIALS
%! A = zeros (2);
%! fail ("ode15i (@(t, y, yp) A * y - A * yp, [0, 1], eye (2), [1, 1])",
%!       "ode15i: Y0 must be a numeric vector");

%!testif HAVE_SUNDIALS
%! A = zeros (2);
%! fail ("ode15i (@(t, y, yp) A * y - A * yp, [0, 1], [1, 1], eye (2))",
%!       "ode15i: YP0 must be a numeric vector");
