########################################################################
##
## Copyright (C) 1994-2023 The Octave Project Developers
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
## @deftypefn {} {[x, map] =} ind2x (@var{caller}, @var{x}, @var{map})
##
## Private function for the ind2XXX functions which have a lot of code in
## common.
## @end deftypefn

function [x, map] = ind2x (caller, x, map)

  ## Check if X is an indexed image.
  ## An indexed image is defined has having only 2 dimensions, and that's how
  ## Matlab behaves.  But we want to support N-D images, so we will allow up to
  ## 4-D and check that the 3rd dimension is a singleton.
  if (all (ndims (x) != [2 4]) || size (x, 3) != 1
      || iscomplex (x) || issparse (x)
      || ! (isfloat (x) && all (x(:) == fix (x(:)))
            || (isinteger (x) && intmin (x) == 0)))
    error ("%s: X must be an indexed image", caller);
  endif

  ## Check if map is a valid colormap.
  if (! iscolormap (map))
    error ("%s: MAP must be a valid colormap", caller);
  endif

  ## Any color indices below the lower bound of the color map are modified
  ## to point to the first color in the map (see bug #41851).
  if (isfloat (x))
    invalid_idx = x < 1;
    if (any (invalid_idx(:)))
      warning (["Octave:" caller ":invalid-idx-img"],
               [caller ": indexed image contains colors outside of colormap"]);
      x(invalid_idx) = 1;
    endif
  endif

  ## Switch to using 1-based indexing.
  ## It is possible that an integer storage class may not have enough room
  ## to make the switch, in which case we convert the data to single.
  maxidx = max (x(:));
  is_int = isinteger (x);
  if (is_int)
    if (maxidx == intmax (x))
      x = single (x);
    endif
    x += 1;
  endif

  ## When there are more colors in the image, than there are in the map,
  ## pad the colormap with the last color in the map for Matlab compatibility.
  num_colors = rows (map);
  if (num_colors - is_int < maxidx)
    warning (["Octave:" caller ":invalid-idx-img"],
             [caller ": indexed image contains colors outside of colormap"]);
    if (numel (x) > maxidx - num_colors + is_int)
      ## The image is large. So extend the map.
      pad = repmat (map(end,:), maxidx - num_colors + is_int, 1);
      map(end+(1:rows (pad)), :) = pad;
    else
      ## The map extension would be large. So clip the image.
      x(x > rows (map)) = rows (map);
    endif
  endif

endfunction
