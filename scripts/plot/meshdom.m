## Copyright (C) 1996, 1997 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} meshdom (@var{x}, @var{y})
## Given vectors of @var{x} and @var{y} coordinates, return two matrices
## corresponding to the @var{x} and @var{y} coordinates of the mesh.
##
## Note: this function is provided for compatibility with older versions
## of @sc{Matlab}.  You should use @code{meshgrid} instead.
## @end deftypefn

## Author: jwe

function [xx, yy] = meshdom (x, y)

  if (nargin == 2)
    if (is_vector (x) && is_vector (y))
      xlen = length (x);
      ylen = length (y);
      xx = zeros (ylen, xlen);
      yy = zeros (ylen, xlen);
      y = y (ylen:-1:1);
      if (columns (x) == 1)
        x = x';
      endif
      if (rows (y) == 1)
        y = y';
      endif
      for i = 1:ylen
        xx(i,:) = x;
      endfor
      for i = 1:xlen
        yy(:,i) = y;
      endfor
    else
      error ("meshdom: arguments must be vectors");
    endif
  else
    usage ("[xx, yy] = meshdom (x, y)");
  endif

endfunction
