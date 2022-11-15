########################################################################
##
## Copyright (C) 1994-2022 The Octave Project Developers
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
## @deftypefn {} {@var{y} =} acot (@var{x})
## Compute the inverse cotangent in radians for each element of @var{x}.
## @seealso{cot, acotd}
## @end deftypefn

function y = acot (x)

  if (nargin < 1)
    print_usage ();
  endif

  y = atan (1 ./ x);

endfunction


%!test
%! rt2 = sqrt (2);
%! rt3 = sqrt (3);
%! x = [rt3, 1, rt3/3, 0, -rt3/3, -1, -rt3];
%! v = [pi/6, pi/4, pi/3, pi/2, -pi/3, -pi/4, -pi/6];
%! assert (acot (x), v, sqrt (eps));

%!error <Invalid call> acot ()
