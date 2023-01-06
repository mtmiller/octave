########################################################################
##
## Copyright (C) 2000-2023 The Octave Project Developers
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
## @deftypefn  {} {} loglogerr (@var{y}, @var{ey})
## @deftypefnx {} {} loglogerr (@var{y}, @dots{}, @var{fmt})
## @deftypefnx {} {} loglogerr (@var{x}, @var{y}, @var{ey})
## @deftypefnx {} {} loglogerr (@var{x}, @var{y}, @var{err}, @var{fmt})
## @deftypefnx {} {} loglogerr (@var{x}, @var{y}, @var{lerr}, @var{uerr}, @var{fmt})
## @deftypefnx {} {} loglogerr (@var{x}, @var{y}, @var{ex}, @var{ey}, @var{fmt})
## @deftypefnx {} {} loglogerr (@var{x}, @var{y}, @var{lx}, @var{ux}, @var{ly}, @var{uy}, @var{fmt})
## @deftypefnx {} {} loglogerr (@var{x1}, @var{y1}, @dots{}, @var{fmt}, @var{xn}, @var{yn}, @dots{})
## @deftypefnx {} {} loglogerr (@var{hax}, @dots{})
## @deftypefnx {} {@var{h} =} loglogerr (@dots{})
## Produce 2-D plots on a double logarithm axis with errorbars.
##
## Many different combinations of arguments are possible.  The most common
## form is
##
## @example
## loglogerr (@var{x}, @var{y}, @var{ey}, @var{fmt})
## @end example
##
## @noindent
## which produces a double logarithm plot of @var{y} versus @var{x}
## with errors in the @var{y}-scale defined by @var{ey} and the plot
## format defined by @var{fmt}.  @xref{XREFerrorbar,,@code{errorbar}}, for
## available formats and additional information.
##
## If the first argument @var{hax} is an axes handle, then plot into this axes,
## rather than the current axes returned by @code{gca}.
## @seealso{errorbar, semilogxerr, semilogyerr}
## @end deftypefn

function h = loglogerr (varargin)

  [hax, varargin] = __plt_get_axis_arg__ ("loglogerr", varargin{:});

  oldfig = [];
  if (! isempty (hax))
    oldfig = get (0, "currentfigure");
  endif
  unwind_protect
    hax = newplot (hax);

    set (hax, "xscale", "log", "yscale", "log");
    if (! ishold ())
      set (hax, "xminortick", "on", "yminortick", "on", "box", "on");
    endif
    htmp = __errplot__ ("loglogerr", hax, varargin{:});

  unwind_protect_cleanup
    if (! isempty (oldfig))
      set (0, "currentfigure", oldfig);
    endif
  end_unwind_protect

  if (nargout > 0)
    h = htmp;
  endif

endfunction


%!demo
%! clf;
%! x = exp (log (0.01):0.2:log (10));
%! wblpdf = @(x, scl, shp) shp*(scl^-shp) .* x.^(shp-1) .* exp (-(x/scl).^shp);
%! y = wblpdf (x, 3, 2);
%! eyu = 2*rand (size (y)) .* y;
%! eyl = 0.5*rand (size (y)) .* y;
%! loglogerr (x, y, eyl, eyu, "#~x-");
%! xlim (x([1, end]));
%! title ({"loglogerr(): loglog() plot with errorbars", ...
%!         "Both axes are logarithmic"});
