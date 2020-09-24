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
## @deftypefn {} {@var{h} =} gcf ()
## Return a handle to the current figure.
##
## The current figure is the default target for graphics output.  If multiple
## figures exist, @code{gcf} returns the last created figure or the last figure
## that was clicked on with the mouse.
##
## If a current figure does not exist, create one and return its handle.  The
## handle may then be used to examine or set properties of the figure.  For
## example,
##
## @example
## @group
## fplot (@@sin, [-10, 10]);
## fig = gcf ();
## set (fig, "numbertitle", "off", "name", "sin plot")
## @end group
## @end example
##
## @noindent
## plots a sine wave, finds the handle of the current figure, and then
## renames the figure window to describe the contents.
##
## Note: To find the current figure without creating a new one if it does not
## exist, query the @qcode{"CurrentFigure"} property of the root graphics
## object.
##
## @example
## get (groot, "currentfigure");
## @end example
##
## @seealso{gca, gco, gcbf, gcbo, get, set}
## @end deftypefn

function h = gcf ()

  h = get (0, "currentfigure");
  if (isempty (h) || h == 0)
    ## We only have a root object, so create a new figure object
    ## and make it the current figure.
    h = figure ();
  endif

endfunction


%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   assert (gcf, hf);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect
