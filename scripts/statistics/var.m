########################################################################
##
## Copyright (C) 1995-2020 The Octave Project Developers
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
## @deftypefn  {} {} var (@var{x})
## @deftypefnx {} {} var (@var{x}, @var{opt})
## @deftypefnx {} {} var (@var{x}, @var{opt}, @var{dim})
## Compute the variance of the elements of the vector @var{x}.
##
## The variance is defined as
## @tex
## $$
## {\rm var} (x) = \sigma^2 = {\sum_{i=1}^N (x_i - \bar{x})^2 \over N - 1}
## $$
## where $\bar{x}$ is the mean value of @var{x} and $N$ is the number of
## elements of @var{x}.
##
## @end tex
## @ifnottex
##
## @example
## @group
## var (@var{x}) = 1/(N-1) SUM_i (@var{x}(i) - mean(@var{x}))^2
## @end group
## @end example
##
## @noindent
## where @math{N} is the length of the @var{x} vector.
##
## @end ifnottex
## If @var{x} is a matrix, compute the variance for each column and return
## them in a row vector.
##
## The argument @var{opt} determines the type of normalization to use.
## Valid values are
##
## @table @asis
## @item 0:
##   normalize with @math{N-1}, provides the best unbiased estimator of the
## variance [default]
##
## @item 1:
##   normalizes with @math{N}, this provides the second moment around the mean
## @end table
##
## If @math{N} is equal to 1 the value of @var{opt} is ignored and
## normalization by @math{N} is used.
##
## If the optional argument @var{dim} is given, operate along this dimension.
## @seealso{cov, std, skewness, kurtosis, moment}
## @end deftypefn

function retval = var (x, opt = 0, dim)

  if (nargin < 1)
    print_usage ();
  endif

  if (! (isnumeric (x) || islogical (x)))
    error ("var: X must be a numeric vector or matrix");
  endif

  if (isempty (opt))
    opt = 0;
  elseif (opt != 0 && opt != 1)
    error ("var: normalization OPT must be 0 or 1");
  endif

  nd = ndims (x);
  sz = size (x);
  if (nargin < 3)
    ## Find the first non-singleton dimension.
    (dim = find (sz > 1, 1)) || (dim = 1);
  else
    if (! (isscalar (dim) && dim == fix (dim) && dim > 0))
      error ("var: DIM must be an integer and a valid dimension");
    endif
  endif

  n = size (x, dim);
  if (n == 1)
    if (isa (x, "single"))
      retval = zeros (sz, "single");
    else
      retval = zeros (sz);
    endif
  elseif (numel (x) > 0)
    retval = sumsq (center (x, dim), dim) / (n - 1 + opt);
  else
    error ("var: X must not be empty");
  endif

endfunction


%!assert (var (13), 0)
%!assert (var (single (13)), single (0))
%!assert (var ([1,2,3]), 1)
%!assert (var ([1,2,3], 1), 2/3, eps)
%!assert (var ([1,2,3], [], 1), [0,0,0])
%!assert (var ([1,2,3], [], 3), [0,0,0])

## Test input validation
%!error var ()
%!error var (1,2,3,4)
%!error <X must be a numeric> var (['A'; 'B'])
%!error <OPT must be 0 or 1> var (1, -1)
%!error <FLAG must be 0 or 1> skewness (1, 2)
%!error <FLAG must be 0 or 1> skewness (1, [1 0])
%!error <DIM must be an integer> var (1, [], ones (2,2))
%!error <DIM must be an integer> var (1, [], 1.5)
%!error <DIM must be .* a valid dimension> var (1, [], 0)
%!error <X must not be empty> var ([], 1)
