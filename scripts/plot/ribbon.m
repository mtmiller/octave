## Copyright (C) 2007-2012 Kai Habel
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
## @deftypefn  {Function File} {} ribbon (@var{x}, @var{y}, @var{width})
## @deftypefnx {Function File} {} ribbon (@var{y})
## @deftypefnx {Function File} {@var{h} =} ribbon (@dots{})
## Plot a ribbon plot for the columns of @var{y} vs.  @var{x}.  The
## optional parameter @var{width} specifies the width of a single ribbon
## (default is 0.75).  If @var{x} is omitted, a vector containing the
## row numbers is assumed (1:rows(Y)).
##
## The optional return value @var{h} is a vector of graphics handles to
## the surface objects representing each ribbon.
## @end deftypefn

## Author: Kai Habel <kai.habel at gmx.de>

function h = ribbon (x, y, width = 0.75)

  if (nargin < 1 || nargin > 3)
    print_usage ();
  endif

  if (nargin == 1)
    y = x;
    if (isvector (y))
      y = y(:);
    endif
    [nr, nc] = size (y);
    x = repmat ((1:nr)', 1, nc);
  endif

  if (isvector (x) && isvector (y))
    if (length (x) != length (y))
      error ("ribbon: vectors X and Y must have the same length");
    else
      [x, y] = meshgrid (x, y);
    endif
  else
    if (! size_equal (x, y))
      error ("ribbon: matrices X and Y must have the same size");
    endif
  endif

  newplot ();

  [nr, nc] = size (y);
  htmp = zeros (nc, 1);

  for c = nc:-1:1
    zz = [y(:,c), y(:,c)];
    yy = x(:,c);
    xx = [c - width / 2, c + width / 2];
    [xx, yy] = meshgrid (xx, yy);
    cc = repmat (c, size (zz));
    htmp(c) = surface (xx, yy, zz, cc);
  endfor

  if (! ishold ())
    ax = get (htmp(1), "parent");
    set (ax, "view", [-37.5, 30], "box", "off", 
             "xgrid", "on", "ygrid", "on", "zgrid", "on");
  endif

  if (nargout > 0)
    h = htmp;
  endif

endfunction


%!demo
%! clf;
%! [x, y, z] = sombrero ();
%! [x, y] = meshgrid (x, y);
%! ribbon (y, z);

%!FIXME: Could have some input validation tests here
