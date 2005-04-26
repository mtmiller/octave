## Copyright (C) 1995, 1996, 1997  Kurt Hornik
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
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} studentize (@var{x}, @var{dim})
## If @var{x} is a vector, subtract its mean and divide by its standard
## deviation.
##
## If @var{x} is a matrix, do the above along the first non-singleton
## dimension. If the optional argument @var{dim} is given then operate
## along this dimension.
## @end deftypefn

## Author: KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description: Subtract mean and divide by standard deviation

function t = studentize (x, dim)

  if (nargin != 1 && nargin != 2)
    usage ("studentize (x, dim)");
  endif

  nd = ndims (x);
  sz = size (x);
  if (nargin != 2)
    ## Find the first non-singleton dimension.
    dim  = 1;
    while (dim < nd + 1 && sz(dim) == 1)
      dim = dim + 1;
    endwhile
    if (dim > nd)
      dim = 1;
    endif
  else
    if (! (isscalar (dim) && dim == round (dim))
	&& dim > 0
	&& dim < (nd + 1))
      error ("studentize: dim must be an integer and valid dimension");
    endif
  endif

  if (! ismatrix (x))
    error ("studentize: x must be a vector or a matrix");
  endif

  c = sz(dim);
  idx = ones (1, nd);
  idx(dim) = c;
  t = x - repmat (mean (x, dim), idx);
  t = t ./ repmat (max (cat (dim, std(t, [], dim), ! any (t, dim)), [], dim), idx);

endfunction
