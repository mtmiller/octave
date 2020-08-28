########################################################################
##
## Copyright (C) 2008-2020 The Octave Project Developers
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
## @deftypefn  {} {@var{q} =} quadgk (@var{f}, @var{a}, @var{b})
## @deftypefnx {} {@var{q} =} quadgk (@var{f}, @var{a}, @var{b}, @var{abstol})
## @deftypefnx {} {@var{q} =} quadgk (@var{f}, @var{a}, @var{b}, @var{abstol}, @var{trace})
## @deftypefnx {} {@var{q} =} quadgk (@var{f}, @var{a}, @var{b}, @var{prop}, @var{val}, @dots{})
## @deftypefnx {} {[@var{q}, @var{err}] =} quadgk (@dots{})
##
## Numerically evaluate the integral of @var{f} from @var{a} to @var{b}
## using adaptive @nospell{Gauss-Kronrod} quadrature.
##
## @var{f} is a function handle, inline function, or string containing the name
## of the function to evaluate.  The function @var{f} must be vectorized and
## return a vector of output values when given a vector of input values.
##
## @var{a} and @var{b} are the lower and upper limits of integration.  Either
## or both limits may be infinite or contain weak end singularities.  Variable
## transformation will be used to treat any infinite intervals and weaken the
## singularities.  For example:
##
## @example
## quadgk (@@(x) 1 ./ (sqrt (x) .* (x + 1)), 0, Inf)
## @end example
##
## @noindent
## Note that the formulation of the integrand uses the element-by-element
## operator @code{./} and all user functions to @code{quadgk} should do the
## same.
##
## The optional argument @var{tol} defines the absolute tolerance used to stop
## the integration procedure.  The default value is 1e-10 (1e-5 for single).
##
## The algorithm used by @code{quadgk} involves subdividing the integration
## interval and evaluating each subinterval.  If @var{trace} is true then after
## computing each of these partial integrals display: (1) the number of
## subintervals at this step, (2) the current estimate of the error @var{err},
## (3) the current estimate for the integral @var{q}.
##
## The behavior of the algorithm can be configured by passing arguments
## to @code{quadgk} as pairs @qcode{"@var{prop}", @var{val}}.  Valid properties
## are
##
## @table @code
## @item AbsTol
## Define the absolute error tolerance for the quadrature.  The default
## absolute tolerance is 1e-10 (1e-5 for single).
##
## @item RelTol
## Define the relative error tolerance for the quadrature.  The default
## relative tolerance is 1e-6 (1e-4 for single).
##
## @item MaxIntervalCount
## @code{quadgk} initially subdivides the interval on which to perform the
## quadrature into 10 intervals.  Subintervals that have an unacceptable error
## are subdivided and re-evaluated.  If the number of subintervals exceeds 650
## subintervals at any point then a poor convergence is signaled and the
## current estimate of the integral is returned.  The property
## @qcode{"MaxIntervalCount"} can be used to alter the number of subintervals
## that can exist before exiting.
##
## @item WayPoints
## Discontinuities in the first derivative of the function to integrate can be
## flagged with the @qcode{"WayPoints"} property.  This forces the ends of a
## subinterval to fall on the breakpoints of the function and can result in
## significantly improved estimation of the error in the integral, faster
## computation, or both.  For example,
##
## @example
## quadgk (@@(x) abs (1 - x.^2), 0, 2, "Waypoints", 1)
## @end example
##
## @noindent
## signals the breakpoint in the integrand at @code{@var{x} = 1}.
##
## @item Trace
## If logically true @code{quadgk} prints information on the convergence of the
## quadrature at each iteration.
## @end table
##
## If any of @var{a}, @var{b}, or @var{waypoints} is complex then the
## quadrature is treated as a contour integral along a piecewise continuous
## path defined by
## @code{[@var{a}, @var{waypoints}(1), @var{waypoints}(2), @dots{}, @var{b}]}.
## In this case the integral is assumed to have no edge singularities.  For
## example,
##
## @example
## @group
## quadgk (@@(z) log (z), 1+1i, 1+1i, "WayPoints",
##         [-1+1i, -1-1i, +1-1i])
## @end group
## @end example
##
## @noindent
## integrates @code{log (z)} along the square defined by
## @code{[1+1i, -1+1i, -1-1i, +1-1i]}.
##
## The result of the integration is returned in @var{q}.
##
## @var{err} is an approximate bound on the error in the integral
## @w{@code{abs (@var{q} - @var{I})}}, where @var{I} is the exact value of the
## integral.
##
## Reference: @nospell{L.F. Shampine},
## @cite{"Vectorized adaptive quadrature in @sc{matlab}"}, Journal of
## Computational and Applied Mathematics, pp.@: 131--140, Vol 211, Issue 2,
## Feb 2008.
##
## @seealso{quad, quadv, quadl, quadcc, trapz, dblquad, triplequad, integral,
##           integral2, integral3}
## @end deftypefn

function [q, err] = quadgk (f, a, b, varargin)

  if (nargin < 3)
    print_usage ();
  endif

  if (b < a)
    ## Reverse integration
    [q, err] = quadgk (f, b, a, varargin{:});
    q = -q;
    return;
  endif

  abstol = [];
  reltol = [];
  waypoints = [];
  maxint = 650;
  trace = false;

  ## Parse options if present.
  if (nargin > 3)
    if (! ischar (varargin{1}))
      if (! isempty (varargin{1}))
        abstol = varargin{1};
        reltol = 0;
      endif
      if (nargin > 4)
        trace = varargin{2};
      endif
      if (nargin > 5)
        error ("quadgk: can not pass additional arguments to user function");
      endif
    else
      if (mod (nargin - 3, 2) != 0)
        error ("quadgk: property/value options must occur in pairs");
      endif

      idx = 1;
      while (idx < nargin - 3)
        if (! ischar (varargin{idx}))
          error ("quadgk: property PROP must be a string");
        endif
        str = varargin{idx++};
        switch (tolower (str))
          case "reltol"
            reltol = varargin{idx++};
          case "abstol"
            abstol = varargin{idx++};
          case "waypoints"
            waypoints = varargin{idx++}(:);
            if (isreal (waypoints))
              waypoints(waypoints < a | waypoints > b) = [];
            endif
          case "maxintervalcount"
            maxint = varargin{idx++};
          case "trace"
            trace = varargin{idx++};
          otherwise
            error ("quadgk: unknown property '%s'", str);
        endswitch
      endwhile
    endif
  endif

  issingle = (isa (a, "single") || isa (b, "single")
              || isa (waypoints, "single"));

  if (isempty (abstol))
    abstol = ifelse (issingle, 1e-5, 1e-10);
  elseif (! isscalar (abstol) || abstol < 0)
    error ("quadgk: ABSTOL must be a scalar >=0");
  endif

  if (isempty (reltol))
    reltol = ifelse (issingle, 1e-4, 1e-6);
  elseif (! isscalar (reltol) || reltol < 0)
    error ("quadgk: RELTOL must be a scalar >=0");
  endif

  ## Convert function given as a string to a function handle
  if (ischar (f))
    f = @(x) feval (f, x);
  endif

  ## Use variable substitution to weaken endpoint singularities and
  ## to perform integration with endpoints at infinity.
  ## No transform for contour integrals.
  if (iscomplex (a) || iscomplex (b) || iscomplex (waypoints))
    ## contour integral, no transform
    subs = [a; waypoints; b];
    h = sum (abs (diff (subs)));
    h0 = h;
    trans = @(t) t;
  elseif (isinf (a) && isinf (b))
    ## Standard infinite to finite integral transformation.
    ##   \int_{-\infinity_^\infinity f(x) dx = \int_-1^1 f (g(t)) g'(t) dt
    ## where
    ##   g(t)  = t / (1 - t^2)
    ##   g'(t) =  (1 + t^2) / (1 - t^2) ^ 2
    ## waypoint transform is then
    ##   t =  (2 * g(t)) ./ (1 + sqrt(1 + 4 * g(t) .^ 2))
    if (! isempty (waypoints))
      trans = @(x) (2 * x) ./ (1 + sqrt (1 + 4 * x .^ 2));
      subs = [-1; trans(waypoints); 1];
    else
      subs = linspace (-1, 1, 11)';
    endif
    h = 2;
    h0 = b - a;
    trans = @(t) t ./ (1 - t.^2);
    f = @(t) f (t ./ (1 - t .^ 2)) .* (1 + t .^ 2) ./ ((1 - t .^ 2) .^ 2);
  elseif (isinf (a))
    ## Formula defined in Shampine paper as two separate steps.
    ## One to weaken singularity at finite end, then a second to transform to
    ## a finite interval.  The singularity weakening transform is
    ##   \int_{-\infinity}^b f(x) dx =
    ##               - \int_{-\infinity}^0 f (b - t^2) 2 t dt
    ## (note minus sign) and the finite interval transform is
    ##   \int_{-\infinity}^0 f(b - t^2)  2 t dt =
    ##                  \int_{-1}^0 f (b - g(s) ^ 2) 2 g(s) g'(s) ds
    ## where
    ##   g(s)  = s / (1 + s)
    ##   g'(s) = 1 / (1 + s) ^ 2
    ## waypoint transform is then
    ##   t = sqrt (b - x)
    ##   s =  - t / (t + 1)
    if (! isempty (waypoints))
      tmp = sqrt (b - waypoints);
      trans = @(x) - x ./ (x + 1);
      subs = [-1; trans(tmp); 0];
    else
      subs = linspace (-1, 0, 11)';
    endif
    h = 1;
    h0 = b - a;
    trans = @(t) b - (t ./ (1 + t)).^2;
    f = @(s) - 2 * s .* f (b -  (s ./ (1 + s)) .^ 2) ./ ((1 + s) .^ 3);
  elseif (isinf (b))
    ## Formula defined in Shampine paper as two separate steps.
    ## One to weaken singularity at finite end, then a second to transform to
    ## a finite interval.  The singularity weakening transform is
    ##   \int_a^\infinity f(x) dx = \int_0^\infinity f (a + t^2) 2 t dt
    ## and the finite interval transform is
    ##  \int_0^\infinity f(a + t^2)  2 t dt =
    ##           \int_0^1 f (a + g(s) ^ 2) 2 g(s) g'(s) ds
    ## where
    ##   g(s)  = s / (1 - s)
    ##   g'(s) = 1 / (1 - s) ^ 2
    ## waypoint transform is then
    ##   t = sqrt (x - a)
    ##   s = t / (t + 1)
    if (! isempty (waypoints))
      tmp = sqrt (waypoints - a);
      trans = @(x) x ./ (x + 1);
      subs = [0; trans(tmp); 1];
    else
      subs = linspace (0, 1, 11)';
    endif
    h = 1;
    h0 = b - a;
    trans = @(t) a + (t ./ (1 - t)).^2;
    f = @(s) 2 * s .* f (a +  (s ./ (1 - s)) .^ 2) ./ ((1 - s) .^ 3);
  else
    ## Davis, Rabinowitz, "Methods of Numerical Integration" p441 2ed.
    ## Presented in section 5 of the Shampine paper as
    ##   g(t) = ((b - a) / 2) * (t / 2 * (3 - t^2)) + (b + a) / 2
    ##   g'(t) = ((b-a)/4) * (3 - 3t^2);
    ## waypoint transform can then be found by solving for t with
    ## Maxima (solve (c + 3*t -  3^3, t);).  This gives 3 roots, two of
    ## which are complex for values between a and b and so can be ignored.
    ## The third is
    ##  c = (-4*x + 2*(b+a)) / (b-a);
    ##  k = ((sqrt(c^2 - 4) + c)/2)^(1/3);
    ##  t = (sqrt(3)* 1i * (1 - k^2) - (1 + k^2)) / 2 / k;
    if (! isempty (waypoints))
      trans = @__quadgk_finite_waypoint__;
      subs = [-1; trans(waypoints, a, b); 1];
    else
      subs = linspace (-1, 1, 11)';
    endif
    h = 2;
    h0 = b - a;
    trans = @(t) ((b - a) ./ 4) * t .* (3 - t.^2) + (b + a) ./ 2;
    f = @(t) f((b - a) ./ 4 .* t .* (3 - t.^2) + (b + a) ./ 2) .* ...
         3 .* (b - a) ./ 4 .* (1 - t.^2);
  endif

  ## Split interval into at least 10 subinterval with a 15 point
  ## Gauss-Kronrod rule giving a minimum of 150 function evaluations.
  while (length (subs) < 11)
    subs = [subs.' ; subs(1:end-1).' + diff(subs.') ./ 2, NaN](:)(1 : end - 1);
  endwhile
  subs = [subs(1:end-1), subs(2:end)];

  warn_id = "Octave:quadgk:warning-termination";

  if (issingle)
    eps1 = eps ("single");
  else
    eps1 = eps ("double");
  endif

  ## Initial evaluation of the integrand on the subintervals
  [q_subs, q_errs] = __quadgk_eval__ (f, subs, eps1, trans);
  q0 = sum (q_subs);
  err0 = sum (q_errs);

  first = true;
  while (true)
    ## Quit if any evaluations are not finite (Inf or NaN).
    if (any (! isfinite (q_subs)))
      warning (warn_id, "quadgk: non-finite integrand encountered");
      q = q0;
      err = err0;
      break;
    endif

    tol = max (abstol, reltol .* abs (q0));

    ## If the global error estimate is met then exit
    if (err0 < tol)
      q = q0;
      err = err0;
      break;
    endif

    ## Accept the subintervals that meet the convergence criteria.
    idx = find (abs (q_errs) < tol .* abs (diff (subs, [], 2)) ./ h);
    if (first)
      q = sum (q_subs(idx));
      err = sum (q_errs(idx));
      first = false;
    else
      q0 = q + sum (q_subs);
      err0 = err + sum (q_errs);
      q += sum (q_subs(idx));
      err += sum (q_errs(idx));
    endif
    subs(idx,:) = [];

    ## If no remaining subintervals exit
    if (rows (subs) == 0)
      break;
    endif

    if (trace)
      disp ([rows(subs), err, q0]);
    endif

    ## Split remaining subintervals in two
    mid = (subs(:,2) + subs(:,1)) ./ 2;
    subs = [subs(:,1), mid; mid, subs(:,2)];

    ## If the maximum subinterval count is met accept remaining subinterval
    ## and exit
    if (rows (subs) > maxint)
      warning (warn_id, "quadgk: maximum interval count (%d) exceeded", maxint);
      q += sum (q_subs);
      err += sum (q_errs);
      break;
    endif

    ## Evaluation of the integrand on the remaining subintervals
    [q_subs, q_errs] = __quadgk_eval__ (f, subs, eps1, trans);
  endwhile

  if (err > max (abstol, reltol * abs (q)))
    warning (warn_id,
             "quadgk: Error tolerance not met.  Estimated error %g", err);
  endif

endfunction

## FIXME: too_close output is never used in function that calls this one.
function [q, err, too_close] = __quadgk_eval__ (f, subs, eps1, trans)
  ## A (15,7) point pair of Gauss-Kronrod quadrature rules.
  ## The abscissa and weights are copied directly from dqk15w.f from quadpack.

  persistent abscissa = [-0.9914553711208126e+00, -0.9491079123427585e+00, ...
                         -0.8648644233597691e+00, -0.7415311855993944e+00, ...
                         -0.5860872354676911e+00, -0.4058451513773972e+00, ...
                         -0.2077849550078985e+00,  0.0000000000000000e+00, ...
                          0.2077849550078985e+00,  0.4058451513773972e+00, ...
                          0.5860872354676911e+00,  0.7415311855993944e+00, ...
                          0.8648644233597691e+00,  0.9491079123427585e+00, ...
                          0.9914553711208126e+00];

  persistent weights15 = ...
      diag ([0.2293532201052922e-01,  0.6309209262997855e-01, ...
             0.1047900103222502e+00,  0.1406532597155259e+00, ...
             0.1690047266392679e+00,  0.1903505780647854e+00, ...
             0.2044329400752989e+00,  0.2094821410847278e+00, ...
             0.2044329400752989e+00,  0.1903505780647854e+00, ...
             0.1690047266392679e+00,  0.1406532597155259e+00, ...
             0.1047900103222502e+00,  0.6309209262997855e-01, ...
             0.2293532201052922e-01]);

  persistent weights7 = ...
      diag ([0.1294849661688697e+00,  0.2797053914892767e+00, ...
             0.3818300505051889e+00,  0.4179591836734694e+00, ...
             0.3818300505051889e+00,  0.2797053914892767e+00, ...
             0.1294849661688697e+00]);

  halfwidth = diff (subs, [], 2) ./ 2;
  center = sum (subs, 2) ./ 2;
  t = (halfwidth * abscissa) + center;
  x = trans ([t(:,1), t(:,end)]);

  ## Shampine suggests 100 * eps1, beginning of section 6.
  if (any (abs (diff (x, [], 2) ./ max (abs (x), [], 2))) < 100 * eps1)
    too_close = true;
    q = 0;
    err = 0;
    return;
  endif

  too_close = false;
  y = reshape (f (t(:)), size (t));

  ## This is faster than using bsxfun as the * operator can use a
  ## single BLAS call, rather than rows (sub) calls to the @times function.
  q = sum (y * weights15, 2) .* halfwidth;
  err = abs (sum (y(:,2:2:end) * weights7, 2) .* halfwidth - q);

endfunction

function t = __quadgk_finite_waypoint__ (x, a, b)
  c = (-4 .* x + 2.* (b + a)) ./ (b - a);
  k = ((sqrt (c .^ 2 - 4) + c) ./ 2) .^ (1/3);
  t = real ((sqrt(3) .* 1i * (1 - k .^ 2) - (1 + k .^ 2)) ./ 2 ./ k);
endfunction


%!assert (quadgk (@sin,-pi,pi), 0, 1e-10)
%!assert (quadgk (inline ("sin"),-pi,pi), 0, 1e-10)
%!assert (quadgk ("sin",-pi,pi), 0, 1e-10)
%!assert (quadgk (@sin,-pi,pi, "WayPoints", 0, "MaxIntervalCount", 100,
%!                "RelTol", 1e-3, "AbsTol", 1e-6, "trace", false), 0, 1e-6)
%!assert (quadgk (@sin,-pi,pi, 1e-6, false), 0, 1e-6)
%!assert <*51867> (quadgk (@(x) x, 0, 0), 0, 0)

%!assert (quadgk (@sin,-pi,0), -2, 1e-10)
%!assert (quadgk (@sin,0,pi), 2, 1e-10)
%!assert (quadgk (@(x) 1./sqrt (x),0,1), 2, 1e-10)
%!assert (quadgk (@(x) abs (1 - x.^2),0,2, "Waypoints", 1), 2, 1e-10)
%!assert (quadgk (@(x) 1./(sqrt (x) .* (x+1)),0,Inf), pi, 1e-10)
%!assert <*57614> (quadgk (@(z) exp(z)./z, 1, 1,
%!                        "Waypoints", [1+i, -1+i, -1-i, 1-i]),
%!                 complex (0, 2*pi), 1e-10)
%!assert <*57614> (quadgk (@(z) exp(z)./z, 1, 1,
%!                        "Waypoints", [1-i, -1-i, -1+i, 1+i]),
%!                 complex (0, -2*pi), 1e-10)
%!assert (quadgk (@(z) log (z),1+1i,1+1i, "WayPoints", [1-1i, -1,-1i, -1+1i]),
%!        complex (0, pi), 1e-10)
%!assert (quadgk (@(x) exp (-x .^ 2),-Inf,Inf), sqrt (pi), -1e-6)
%!assert (quadgk (@(x) exp (-x .^ 2),-Inf,0), sqrt (pi)/2, -1e-6)
%!test
%! f = @(x) x .^ 5 .* exp (-x) .* sin (x);
%! assert (quadgk (f, 0, Inf, "RelTol", 1e-8, "AbsTol", 1e-12), -15, -1e-8);

## Test input validation
%!error quadgk (@sin)
%!error quadgk (@sin, 0)
%!error <can not pass additional arguments> quadgk (@sin, 0, 1, 1e-6, true, 4)
%!error <options must occur in pairs> quadgk (@sin, 0, 1, "DummyArg")
%!error <PROP must be a string> quadgk (@sin, 0, 1, "AbsTol", 1e-6, 2, 3)
%!error <unknown property 'foo'> quadgk (@sin, 0, 1, "foo", 3)
%!error <ABSTOL must be a scalar> quadgk (@sin, 0, 1, ones (2,2))
%!error <ABSTOL must be a scalar .=0> quadgk (@sin, 0, 1, -1)
%!error <RELTOL must be a scalar> quadgk (@sin, 0, 1, "RelTol", ones (2,2))
%!error <RELTOL must be a scalar> quadgk (@sin, 0, 1, "RelTol", -1)
