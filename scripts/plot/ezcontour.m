## Copyright (C) 2007-2012 David Bateman
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {Function File} {} ezcontour (@var{f})
## @deftypefnx {Function File} {} ezcontour (@dots{}, @var{dom})
## @deftypefnx {Function File} {} ezcontour (@dots{}, @var{n})
## @deftypefnx {Function File} {} ezcontour (@var{hax}, @dots{})
## @deftypefnx {Function File} {@var{h} =} ezcontour (@dots{})
##
## Plot the contour lines of a function.
## 
## @var{f} is a string, inline function, or function handle with two arguments
## defining the function.  By default the plot is over the meshed domain
## @code{-2*pi <= @var{x} | @var{y} <= 2*pi} with 60 points in each dimension.
##
## If @var{dom} is a two element vector, it represents the minimum and maximum
## values of both @var{x} and @var{y}.  If @var{dom} is a four element vector,
## then the minimum and maximum values are @code{[xmin xmax ymin ymax]}.
##
## @var{n} is a scalar defining the number of points to use in each dimension.
##
## If the first argument @var{hax} is an axes handle, then plot into this axis,
## rather than the current axes returned by @code{gca}.
##
## The optional return value @var{h} is a graphics handle to the created plot.
##
## Example:
##
## @example
## @group
## f = @@(x,y) sqrt (abs (x .* y)) ./ (1 + x.^2 + y.^2);
## ezcontour (f, [-3, 3]);
## @end group
## @end example
##
## @seealso{contour, ezcontourf, ezplot, ezmeshc, ezsurfc}
## @end deftypefn

function h = ezcontour (varargin)

  [htmp, needusage] = __ezplot__ ("contour", varargin{:});

  if (needusage)
    print_usage ();
  endif

  if (nargout > 0)
    h = htmp;
  endif

endfunction


%!demo
%! clf;
%! colormap ('default');
%! f = @(x,y) sqrt (abs (x .* y)) ./ (1 + x.^2 + y.^2);
%! ezcontour (f, [-3, 3]);

