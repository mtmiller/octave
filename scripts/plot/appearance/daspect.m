########################################################################
##
## Copyright (C) 2010-2023 The Octave Project Developers
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
## @deftypefn  {} {@var{data_aspect_ratio} =} daspect ()
## @deftypefnx {} {} daspect (@var{data_aspect_ratio})
## @deftypefnx {} {} daspect (@var{mode})
## @deftypefnx {} {@var{data_aspect_ratio_mode} =} daspect ("mode")
## @deftypefnx {} {} daspect (@var{hax}, @dots{})
## Query or set the data aspect ratio of the current axes.
##
## The aspect ratio is a normalized 3-element vector representing the span of
## the x, y, and z-axis limits.
##
## @code{daspect (@var{mode})}
##
## Set the data aspect ratio mode of the current axes.  @var{mode} is
## either @qcode{"auto"} or @qcode{"manual"}.
##
## @code{daspect (@qcode{"mode"})}
##
## Return the data aspect ratio mode of the current axes.
##
## @code{daspect (@var{hax}, @dots{})}
##
## Operate on the axes in handle @var{hax} instead of the current axes.
##
## @seealso{axis, pbaspect, xlim, ylim, zlim}
## @end deftypefn

function daratio = daspect (varargin)

  ## Grab axes handle if present
  if (nargin > 0)
    if (isscalar (varargin{1}) && isaxes (varargin{1}))
      hax = varargin{1};
      varargin = varargin(2:end);
    else
      hax = gca ();
    endif
  else
    hax = gca ();
  endif

  nargin = numel (varargin);
  if (nargin > 1)
    print_usage ();
  endif

  if (nargin == 0)
    daratio = get (hax, "dataaspectratio");
  else
    arg = varargin{1};
    if (isnumeric (arg))
      if (numel (arg) == 2)
        set (hax, "dataaspectratio", [arg, 1]);
      elseif (numel (arg) == 3)
        set (hax, "dataaspectratio", arg);
      else
        error ("daspect: DATA_ASPECT_RATIO must be a 2 or 3 element vector");
      endif
    elseif (ischar (arg))
      arg = tolower (arg);
      switch (arg)
        case "auto"
          set (hax, "dataaspectratiomode", "auto");
        case "manual"
          set (hax, "dataaspectratiomode", "manual");
        case "mode"
          daratio = get (hax, "dataaspectratiomode");
        otherwise
          error ("daspect: Invalid MODE <%s>", arg);
      endswitch
    else
      print_usage ();
    endif
  endif

endfunction


%!demo
%! clf;
%! x = 0:0.01:4;
%! plot (x,cos (x), x,sin (x));
%! axis square;
%! daspect ([1 1 1]);
%! title ("square plot box with axis limits [0, 4, -2, 2]");

%!demo
%! clf;
%! x = 0:0.01:4;
%! plot (x,cos (x), x,sin (x));
%! axis ([0 4 -1 1]);
%! daspect ([2 1 1]);
%! title ("square plot box with axis limits [0, 4, -1, 1]");

%!demo
%! clf;
%! x = 0:0.01:4;
%! plot (x,cos (x), x,sin (x));
%! daspect ([1 2 1]);
%! pbaspect ([2 1 1]);
%! title ("2x1 plot box with axis limits [0, 4, -2, 2]");

%!demo
%! clf;
%! x = 0:0.01:4;
%! plot (x,cos (x), x, sin (x));
%! axis square;
%! set (gca, "positionconstraint", "innerposition");
%! daspect ([1 1 1]);
%! title ("square plot box with axis limits [0, 4, -2, 2]");

%!demo
%! clf;
%! x = 0:0.01:4;
%! plot (x,cos (x), x,sin (x));
%! axis ([0 4 -1 1]);
%! set (gca, "positionconstraint", "innerposition");
%! daspect ([2 1 1]);
%! title ("square plot box with axis limits [0, 4, -1, 1]");

## FIXME: need some input validation tests
