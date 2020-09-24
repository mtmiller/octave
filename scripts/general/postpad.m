########################################################################
##
## Copyright (C) 1994-2020 The Octave Project Developers
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
## @deftypefn  {} {} postpad (@var{x}, @var{l})
## @deftypefnx {} {} postpad (@var{x}, @var{l}, @var{c})
## @deftypefnx {} {} postpad (@var{x}, @var{l}, @var{c}, @var{dim})
## Append the scalar value @var{c} to the vector @var{x} until it is of length
## @var{l}.  If @var{c} is not given, a value of 0 is used.
##
## If @code{length (@var{x}) > @var{l}}, elements from the end of @var{x} are
## removed until a vector of length @var{l} is obtained.
##
## If @var{x} is a matrix, elements are appended or removed from each row.
##
## If the optional argument @var{dim} is given, operate along this dimension.
##
## If @var{dim} is larger than the dimensions of @var{x}, the result will have
## @var{dim} dimensions.
## @seealso{prepad, cat, resize}
## @end deftypefn

function y = postpad (x, l, c, dim)

  if (nargin < 2)
    print_usage ();
  endif

  if (nargin < 3 || isempty (c))
    c = 0;
  else
    if (! isscalar (c))
      error ("postpad: third argument must be empty or a scalar");
    endif
  endif

  nd = ndims (x);
  sz = size (x);
  if (nargin < 4)
    ## Find the first non-singleton dimension.
    (dim = find (sz > 1, 1)) || (dim = 1);
  else
    if (!(isscalar (dim) && dim == fix (dim) && dim >= 1))
      error ("postpad: DIM must be an integer and a valid dimension");
    endif
  endif

  if (! isscalar (l) || l < 0)
    error ("postpad: second argument must be a positive scaler");
  endif

  if (dim > nd)
    sz(nd+1:dim) = 1;
  endif

  d = sz(dim);

  if (d == l)
    ## This optimization makes sense because the function is used to match
    ## the length between two vectors not knowing a priori is larger, and
    ## allow for:
    ##    ml = max (numel (v1), numel (v2));
    ##    v1 = postpad (v1, ml);
    ##    v2 = postpad (v2, ml);
    y = x;
  elseif (d >= l)
    idx = repmat ({':'}, nd, 1);
    idx{dim} = 1:l;
    y = x(idx{:});
  else
    sz(dim) = l - d;
    y = cat (dim, x, c(ones (sz)));
  endif

endfunction


%!assert (postpad ([1,2], 4), [1,2,0,0])
%!assert (postpad ([1;2], 4), [1;2;0;0])
%!assert (postpad ([1,2], 4, 2), [1,2,2,2])
%!assert (postpad ([1;2], 4, 2), [1;2;2;2])
%!assert (postpad ([1,2], 2, 2, 1), [1,2;2,2])
%!assert (postpad ([1;2], 2, 2, 3), reshape ([1;2;2;2], 2, 1, 2))
%!assert (postpad ([1,2], 2, 2, 3), reshape ([1,2,2,2], 1, 2, 2))

%!assert (postpad ([1 2], 2), [1 2])
%!assert (postpad ([1; 2], 2), [1; 2])
%!assert (postpad ([1; 2], 2, 3, 2), [1 3; 2 3])

%! ## Test with string concatenation
%!assert <*44162> (postpad ("Octave", 16, "x"), "Octavexxxxxxxxxx")
%!assert (postpad ("Octave", 4), "Octa")

%!error postpad ()
%!error postpad (1)
%!error postpad (1,2,3,4,5)
