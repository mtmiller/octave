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
## @deftypefn {} {@var{p} =} polyreduce (@var{c})
## Reduce a polynomial coefficient vector to a minimum number of terms by
## stripping off any leading zeros.
## @seealso{polyout}
## @end deftypefn

function p = polyreduce (c)

  if (nargin < 1)
    print_usage ();
  elseif (! isvector (c) || isempty (c))
    error ("polyreduce: C must be a non-empty vector");
  endif

  idx = find (c != 0, 1);

  if (isempty (idx))
    p = 0;
  else
    p = c(idx:end);
  endif

endfunction


%!assert (polyreduce ([0, 0, 1, 2, 3]), [1, 2, 3])
%!assert (polyreduce ([1, 2, 3, 0, 0]), [1, 2, 3, 0, 0])
%!assert (polyreduce ([1, 0, 3]), [1, 0, 3])
%!assert (polyreduce ([0, 0, 0]), 0)

%!error <Invalid call> polyreduce ()
%!error <C must be a non-empty vector> polyreduce ([1, 2; 3, 4])
%!error <C must be a non-empty vector> polyreduce ([])
