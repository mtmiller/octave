########################################################################
##
## Copyright (C) 2003-2023 The Octave Project Developers
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
## @deftypefn  {} {@var{c} =} contourc (@var{z})
## @deftypefnx {} {@var{c} =} contourc (@var{z}, @var{vn})
## @deftypefnx {} {@var{c} =} contourc (@var{x}, @var{y}, @var{z})
## @deftypefnx {} {@var{c} =} contourc (@var{x}, @var{y}, @var{z}, @var{vn})
## @deftypefnx {} {[@var{c}, @var{lev}] =} contourc (@dots{})
## Compute contour lines (isolines of constant Z value).
##
## The matrix @var{z} contains height values above the rectangular grid
## determined by @var{x} and @var{y}.  If only a single input @var{z} is
## provided then @var{x} is taken to be @code{1:columns (@var{z})} and @var{y}
## is taken to be @code{1:rows (@var{z})}.  The minimum data size is 2x2.
##
## The optional input @var{vn} is either a scalar denoting the number of
## contour lines to compute or a vector containing the Z values where lines
## will be computed.  When @var{vn} is a vector the number of contour lines
## is @code{numel (@var{vn})}.  However, to compute a single contour line
## at a given value use @code{@var{vn} = [val, val]}.  If @var{vn} is omitted
## it defaults to 10.
##
## The return value @var{c} is a 2x@var{n} matrix containing the contour lines
## in the following format
##
## @example
## @group
## @var{c} = [lev1, x1, x2, @dots{}, levn, x1, x2, ...
##      len1, y1, y2, @dots{}, lenn, y1, y2, @dots{}]
## @end group
## @end example
##
## @noindent
## in which contour line @var{n} has a level (height) of @var{levn} and length
## of @var{lenn}.
##
## The optional return value @var{lev} is a vector with the Z values of the
## contour levels.
##
## Example:
##
## @example
## @group
## x = 0:2;
## y = x;
## z = x' * y;
## c = contourc (x, y, z, 2:3)
##   @result{} c =
##         2.0000   1.0000   1.0000   2.0000   2.0000   3.0000   1.5000   2.0000
##         4.0000   2.0000   2.0000   1.0000   1.0000   2.0000   2.0000   1.5000
## @end group
## @end example
## @seealso{contour, contourf, contour3, clabel}
## @end deftypefn

function [c, lev] = contourc (varargin)

  if (nargin < 1 || nargin > 4)
    print_usage ();
  endif

  if (nargin == 1)
    z = varargin{1};
    x = 1:columns (z);
    y = 1:rows (z);
    vn = 10;
  elseif (nargin == 2)
    z = varargin{1};
    x = 1:columns (z);
    y = 1:rows (z);
    vn = varargin{2};
  elseif (nargin == 3)
    x = varargin{1};
    y = varargin{2};
    z = varargin{3};
    vn = 10;
  elseif (nargin == 4)
    x = varargin{1};
    y = varargin{2};
    z = varargin{3};
    vn = varargin{4};
  endif

  if (! (isnumeric (z) && isnumeric (x) && isnumeric (y))
      || ! (ismatrix (z) && ismatrix (x) && ismatrix (y))
      || ! (isreal (z) && isreal (x) && isreal (y)))
    error ("contourc: X, Y, and Z must be real numeric matrices");
  endif

  if (rows (z) < 2 || columns (z) < 2)
    error ("contourc: Z data must have at least 2 rows and 2 columns");
  endif

  if (isscalar (vn))
    lev = linspace (min (z(:)), max (z(:)), vn+2)(2:end-1);
  else
    lev = unique (sort (vn));
  endif

  if (isvector (x) && isvector (y))
    c = __contourc__ (x(:)', y(:)', z, lev);
  elseif (! any (bsxfun (@minus, x, x(1,:))(:))
          && ! any (bsxfun (@minus, y, y(:,1))(:)))
    ## x,y are uniform grid (such as from meshgrid)
    c = __contourc__ (x(1,:), y(:,1)', z, lev);
  else
    ## Data is sampled over non-uniform mesh.
    ## Algorithm calculates contours for uniform grid
    ## and then interpolates values back to the non-uniform mesh.

    ## Uniform grid for __contourc__.
    [nr, nc] = size (z);
    ii = 1:nc;
    jj = 1:nr;

    c = __contourc__ (ii, jj, z, lev);

    ## Map the contour lines from index space (i,j)
    ## back to the original grid (x,y)
    i = 1;

    while (i < columns (c))
      clen = c(2, i);
      idx = i + (1:clen);

      ci = c(1, idx);
      cj = c(2, idx);

      ## Due to rounding errors, some elements of ci and cj can fall out of the
      ## range of ii and jj and interp2 would return NA for those values.
      ## The permitted range is enforced here:

      ci = max (ci, 1); ci = min (ci, nc);
      cj = max (cj, 1); cj = min (cj, nr);

      c(1, idx) = interp2 (ii, jj, x, ci, cj);
      c(2, idx) = interp2 (ii, jj, y, ci, cj);

      i += (clen + 1);
    endwhile
  endif

endfunction


%!test
%! x = 0:2;
%! y = x;
%! z = x' * y;
%! c_exp = [2, 1, 1, 2, 2, 3, 1.5, 2; 4, 2, 2, 1, 1, 2, 2, 1.5];
%! lev_exp = [2 3];
%! [c_obs, lev_obs] = contourc (x, y, z, 2:3);
%! assert (c_obs, c_exp, eps);
%! assert (lev_obs, lev_exp, eps);

## Test input validation
%!error <Invalid call> contourc ()
%!error <Invalid call> contourc (1,2,3,4,5)
%!error <X, Y, and Z must be .* numeric> contourc ({3})
%!error <X, Y, and Z must be .* numeric> contourc ({1}, 2, 3)
%!error <X, Y, and Z must be .* numeric> contourc (1, {2}, 3)
%!error <X, Y, and Z must be .* matrices> contourc (ones (3,3,3))
%!error <X, Y, and Z must be .* matrices> contourc (ones (3,3,3), 2, 3)
%!error <X, Y, and Z must be .* matrices> contourc (1, ones (3,3,3), 3)
%!error <X, Y, and Z must be real> contourc (3i)
%!error <X, Y, and Z must be real> contourc (1i, 2, 3)
%!error <X, Y, and Z must be real> contourc (1, 2i, 3)
%!error <Z data must have at least 2 rows> contourc ([1, 2])
%!error <Z data must have at least .* 2 columns> contourc ([1; 2])
