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
## @deftypefn  {} {@var{cmap} =} contrast (@var{x})
## @deftypefnx {} {@var{cmap} =} contrast (@var{x}, @var{n})
## Return a gray colormap that maximizes the contrast in an image.
##
## The returned colormap will have @var{n} rows.  If @var{n} is not defined
## then the size of the current colormap is used.
## @seealso{colormap, brighten}
## @end deftypefn

function cmap = contrast (x, n)

  if (nargin == 1)
    hf = get (0, "currentfigure");
    if (! isempty (hf))
      n = rows (get (hf, "colormap"));
    else
      n = 64;
    endif
  else
    if (! isscalar (n))
      error ("contrast: N must be a scalar");
    endif
    n = double (n);
  endif

  x = x(:);
  minx = min (x);
  cmap = find (diff (sort ([round(n * ((x - minx) ./ (max(x) - minx))); [0:n]'])));
  minm = min (cmap);
  cmap = (cmap - minm) ./ (max (cmap) - minm);
  cmap = [cmap, cmap, cmap];

endfunction


%!demo
%! clf;
%! img = reshape (1:100, 10, 10);
%! imagesc (img);
%! colormap (gray (64));
%! title ("Image with default 64 gray levels");
%! pos = get (gcf, "position");
%! pos(1) += pos(3) + 25;
%! figure ("position", pos);
%! colormap (contrast (img, 10));
%! imagesc (img);
%! title ("Image with contrast enhanced");

%!assert (contrast (1:100,10), [([0:9]/9)',([0:9]/9)',([0:9]/9)'], 1e-10)
