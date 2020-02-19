########################################################################
##
## Copyright (C) 2005-2020 The Octave Project Developers
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
## @deftypefn  {} {} fplot (@var{fn})
## @deftypefnx {} {} fplot (@var{fn}, @var{limits})
## @deftypefnx {} {} fplot (@dots{}, @var{tol})
## @deftypefnx {} {} fplot (@dots{}, @var{n})
## @deftypefnx {} {} fplot (@dots{}, @var{fmt})
## @deftypefnx {} {} fplot (@dots{}, @var{property}, @var{value}, @dots{})
## @deftypefnx {} {} fplot (@var{hax}, @dots{})
## @deftypefnx {} {[@var{x}, @var{y}] =} fplot (@dots{})
## Plot a function @var{fn} within the range defined by @var{limits}.
##
## @var{fn} is a function handle, inline function, or string containing the
## name of the function to evaluate.
##
## The limits of the plot are of the form @w{@code{[@var{xlo}, @var{xhi}]}} or
## @w{@code{[@var{xlo}, @var{xhi}, @var{ylo}, @var{yhi}]}}.  If no limits
## are specified the default is @code{[-5, 5]}.
##
## The next three arguments are all optional and any number of them may be
## given in any order.
##
## @var{tol} is the relative tolerance to use for the plot and defaults
## to 2e-3 (.2%).
##
## @var{n} is the minimum number of points to use.  When @var{n} is specified,
## the maximum stepsize will be @code{(@var{xhi} - @var{xlo}) / @var{n}}.  More
## than @var{n} points may still be used in order to meet the relative
## tolerance requirement.
##
## The @var{fmt} argument specifies the linestyle to be used by the plot
## command.
##
## Multiple property-value pairs may also be specified, but they must appear
## in pairs.  These arguments are applied to the line objects drawn by
## @code{plot}.
##
## The full list of line properties is documented at
## @ref{Line Properties}.
##
## If the first argument @var{hax} is an axes handle, then plot into this axes,
## rather than the current axes returned by @code{gca}.
##
## With no output arguments, the results are immediately plotted.  With two
## output arguments, the 2-D plot data is returned.  The data can subsequently
## be plotted manually with @code{plot (@var{x}, @var{y})}.
##
## Example:
##
## @example
## @group
## fplot (@@cos, [0, 2*pi])
## fplot ("[cos(x), sin(x)]", [0, 2*pi])
## @end group
## @end example
##
## Programming Notes:
##
## @code{fplot} works best with continuous functions.  Functions with
## discontinuities are unlikely to plot well.  This restriction may be removed
## in the future.
##
## @code{fplot} performance is better when the function accepts and returns a
## vector argument.  Consider this when writing user-defined functions and use
## element-by-element operators such as @code{.*}, @code{./}, etc.
##
## @seealso{ezplot, plot}
## @end deftypefn

function [X, Y] = fplot (varargin)

  [hax, varargin, nargin] = __plt_get_axis_arg__ ("fplot", varargin{:});

  if (nargin < 1 || nargin > 5)
    print_usage ();
  endif

  fn = varargin{1};
  if (isa (fn, "inline"))
    fn = vectorize (inline (fn));
    nam = formula (fn);
  elseif (is_function_handle (fn))
    nam = func2str (fn);
  elseif (all (isalnum (fn)))
    nam = fn;
  elseif (ischar (fn))
    fn = vectorize (inline (fn));
    nam = formula (fn);
  else
    error ("fplot: FN must be a function handle, inline function, or string");
  endif

  if (nargin > 1 && isnumeric (varargin{2}))
    limits = varargin{2};
    if (iscomplex (limits) || (numel (limits) != 2 && numel (limits) != 4))
      error ("fplot: LIMITS must be a real vector with 2 or 4 elements");
    endif
    i = 3;
  else
    limits = [-5, 5];
    i = 2;
  endif

  n = 5;
  tol = 2e-3;
  fmt = {};
  while (i <= numel (varargin))
    arg = varargin{i};
    if (ischar (arg))
      [~, valid_fmt] = __pltopt__ ("fplot", arg, false);
      if (valid_fmt)
        fmt(end+1) = arg;
      else
        if (i == numel (varargin))
          error ("fplot: bad input in position %d", i);
        endif
        fmt(end+(1:2)) = varargin([i, i+1]);
        i++;  # Skip PROPERTY.
      endif
    elseif (isnumeric (arg) && isscalar (arg) && arg > 0)
      if (arg == fix (arg))
        n = arg;
      else
        tol = arg;
      endif
    else
      error ("fplot: bad input in position %d", i);
    endif
    i++;
  endwhile

  if (n != 5)
    ## n was specified
    x0 = linspace (limits(1), limits(2), n/2 + 1)';
  else
    x0 = linspace (limits(1), limits(2), 5)';
    n = 8;
  endif

  try
    y0 = feval (fn, x0);
    if (isscalar (y0))
      warning ("fplot: FN is not a vectorized function which reduces performance");
      fn = @(x) arrayfun (fn, x);  # Create a new fn that accepts vectors
      y0 = feval (fn, x0);
    endif
  catch
    ## feval failed, maybe it is because the function is not vectorized?
    fn = @(x) arrayfun (fn, x);  # Create a new fn that accepts vectors
    y0 = feval (fn, x0);
    warning ("fplot: FN is not a vectorized function which reduces performance");
  end_try_catch

  x = linspace (limits(1), limits(2), n)';
  y = feval (fn, x);

  if (rows (x0) == rows (y0))
    fcn_transpose = false;
  elseif (rows (x0) == columns (y0))
    fcn_transpose = true;
    y0 = y0.';
    y = y.';
  else
    error ("fplot: invalid function FN (# of outputs not equal to inputs)");
  endif

  err0 = Inf;

  ## FIXME: This algorithm should really use adaptive scaling as
  ##        the numerical quadrature algorithms do so that extra points are
  ##        used where they are needed and not spread evenly over the entire
  ##        x-range.  Try any function with a discontinuity, such as
  ##        fplot (@tan, [-2, 2]) or fplot ("1./x", [-3, 2]), to see the
  ##        problems with the current solution.

  while (n < 2^18)    # Something is wrong if we need more than 250K points
    yi = interp1 (x0, y0, x, "linear");
    ## relative error calculation using average of [yi,y] as reference
    ## since neither estimate is known a priori to be better than the other.
    err = 0.5 * max (abs ((yi - y) ./ (yi + y + eps))(:));
    if (err < tol || abs (err - err0) < tol/2)
      ## Either relative tolerance has been met OR
      ## algorithm has stopped making any reasonable progress per iteration.
      break;
    endif
    x0 = x;
    y0 = y;
    err0 = err;
    n = 2 * (n - 1) + 1;
    x = linspace (limits(1), limits(2), n)';
    y = feval (fn, x);
    if (fcn_transpose)
      y = y.';
    endif
  endwhile

  if (nargout == 2)
    X = x;
    Y = y;
  else
    if (isempty (hax))
      hax = gca ();
    endif
    plot (hax, x, y, fmt{:});
    axis (hax, limits);
    ## FIXME: If hold is on, then this erases the existing legend rather than
    ##        adding to it.
    if (isvector (y))
      legend (hax, nam);
    else
      for i = 1:columns (y)
        nams{i} = sprintf ("%s(:,%i)", nam, i);
      endfor
      legend (hax, nams{:});
    endif
  endif

endfunction


%!demo
%! clf;
%! fplot (@cos, [0, 2*pi]);
%! title ("fplot() single function");

%!demo
%! clf;
%! fplot ("[cos(x), sin(x)]", [0, 2*pi]);
%! title ("fplot() multiple functions");

%!demo
%! clf;
%! fh = @(x) sin (pi*x) ./ (pi*x);
%! fplot (fh, [-5, 5]);
%! title ("fplot() sinc function (possible division by 0, near 0)");

%!test
%! ## Multi-valued function
%! [x, y] = fplot ("[cos(x), sin(x)]", [0, 2*pi]);
%! assert (columns (y) == 2);
%! assert (rows (x) == rows (y));
%! assert (y, [cos(x), sin(x)], -2e-3);

%!test
%! ## Function requiring transpose
%! fn = @(x) 2 * x(:).';
%! [x, y] = fplot (fn, [-1, 1]);
%! assert (columns (y) == 1);
%! assert (rows (x) == rows (y));
%! assert (y, 2*x);

%!test
%! ## Constant value function
%! fn = @(x) 0;
%! [x, y] = fplot (fn, [-1, 1]);
%! assert (columns (y) == 1);
%! assert (rows (x) == rows (y));
%! assert (y, repmat ([0], size (x)));

## Test input validation
%!error fplot ()
%!error fplot (1,2,3,4,5,6)
%!error <FN must be a function handle> fplot (1, [0 1])
%!error <LIMITS must be a real vector> fplot (@cos, [i, 2*i])
%!error <LIMITS must be a real vector with 2 or 4> fplot (@cos, [1])
%!error <LIMITS must be a real vector with 2 or 4> fplot (@cos, [1 2 3])
%!error <bad input in position 2> fplot (@cos, "linewidth")
%!error <bad input in position 3> fplot (@cos, [-1,1], {1})
%!warning <FN is not a vectorized function>
%! fn = @(x) 0;
%! [x,y] = fplot (fn, [-1,1]);
%!error <invalid function FN>
%! fn = @(x) [x;x];
%! fplot (fn, [-1,1]);
