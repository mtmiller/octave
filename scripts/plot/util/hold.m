########################################################################
##
## Copyright (C) 2005-2023 The Octave Project Developers
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
## @deftypefn  {} {} hold
## @deftypefnx {} {} hold on
## @deftypefnx {} {} hold off
## @deftypefnx {} {} hold (@var{hax}, @dots{})
## Toggle or set the @qcode{"hold"} state of the plotting engine which
## determines whether new graphic objects are added to the plot or replace
## the existing objects.
##
## @table @code
## @item hold on
## Retain plot data and settings so that subsequent plot commands are displayed
## on a single graph.  Line color and line style are advanced for each new plot
## added.
##
## @item hold all (deprecated)
## Equivalent to @code{hold on}.
##
## @item hold off
## Restore default graphics settings which clear the graph and reset axes
## properties before each new plot command.  (default).
##
## @item hold
## Toggle the current hold state.
## @end table
##
## When given the additional argument @var{hax}, the hold state is modified
## for this axes rather than the current axes returned by @code{gca}.
##
## To query the current hold state use the @code{ishold} function.
## @seealso{ishold, cla, clf, newplot}
## @end deftypefn

function hold (varargin)

  if (nargin > 0 && isscalar (varargin{1}) && isaxes (varargin{1}))
    hax = varargin{1};
    varargin(1) = [];
    nargs = numel (varargin);
    hfig = ancestor (hax, "figure");
  elseif (nargin > 0 && numel (varargin{1}) > 1 && ishghandle (varargin{1}))
    print_usage ();
  else
    hax = gca ();
    hfig = gcf ();
    nargs = numel (varargin);
  endif

  hold_all = false;
  if (nargs == 0)
    turn_hold_off = ishold (hax);
  elseif (nargs == 1)
    state = tolower (varargin{1});
    switch (state)
      case "off"
        turn_hold_off = true;
      case "all"
        turn_hold_off = false;
        hold_all = true;
      case "on"
        turn_hold_off = false;
      otherwise
        error ("hold: invalid hold STATE");
    endswitch
  else
    print_usage ();
  endif

  if (turn_hold_off)
    set (hax, "nextplot", "replace");
  else
    set (hax, "nextplot", "add");
    set (hfig, "nextplot", "add");
  endif

endfunction


%!demo
%! clf;
%! t = linspace (0, 2*pi, 100);
%! plot (t, sin (t));
%! hold on;
%! plot (t, cos (t));
%! title ({"hold on", "2 plots shown on same graph"});
%! hold off;

%!demo
%! clf;
%! A = rand (100);
%! [X, Y] = find (A > 0.95);
%! imshow (A);
%! hold on;
%! plot (X, Y, "o");
%! hold off;
%! title ("hold with image and plot");

%!demo
%! clf;
%! colormap ("default");
%! hold on;
%! imagesc (1 ./ hilb (4));
%! plot (1:4, "-s");
%! title ("hold with image and plot");
%! hold off;

%!demo
%! clf;
%! colormap ("default");
%! hold on;
%! imagesc (1 ./ hilb (2));
%! imagesc (1 ./ hilb (4));
%! title ({"hold with two images", "only second image is visible"});
%! hold off;

%!demo
%! clf;
%! colormap ("default");
%! hold on;
%! plot (1:4, "-s");
%! imagesc (1 ./ hilb (4));
%! title ({"hold with plot and image", "only image is visible"});
%! hold off;

%!demo
%! clf;
%! colormap ("default");
%! t = linspace (-3, 3, 50);
%! [x, y] = meshgrid (t, t);
%! z = peaks (x, y);
%! contourf (x, y, z, 10);
%! hold on;
%! plot (x(:), y(:), "^");
%! patch ([-1.0 1.0 1.0 -1.0 -1.0], [-1.0 -1.0 1.0 1.0 -1.0], "red");
%! xlim ([-2.0 2.0]);
%! ylim ([-2.0 2.0]);
%! colorbar ("SouthOutside");
%! title ("hold with line, surface, and patch objects");

## hold on test
%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   p = plot ([0 1]);
%!   assert (! ishold);
%!   hold on;
%!   assert (ishold);
%!   p1 = fill ([0 1 1], [0 0 1], "black");
%!   p2 = fill ([0 1 0], [0 1 1], "red");
%!   assert (length (get (hf, "children")), 1);
%!   assert (length (get (gca, "children")), 3);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

## hold off test
%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   p = plot ([0 1]);
%!   assert (! ishold);
%!   hold on;
%!   assert (ishold);
%!   p1 = fill ([0 1 1], [0 0 1], "black");
%!   hold off;
%!   p2 = fill ([0 1 0], [0 1 1], "red");
%!   assert (length (get (hf, "children")), 1);
%!   assert (length (get (gca, "children")), 1);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect
