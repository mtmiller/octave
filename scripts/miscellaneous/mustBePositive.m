########################################################################
##
## Copyright (C) 2019-2020 The Octave Project Developers
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
## @deftypefn {} {} mustBePositive (@var{x})
##
## Require that input @var{x} is positive.
##
## Raise an error if any element of the input @var{x} is not positive, as
## determined by @code{@var{x} > 0}.
##
## @seealso{mustBeNonnegative, mustBeNonzero}
## @end deftypefn

function mustBePositive (x)

  if (nargin != 1)
    print_usage ();
  endif

  tf = (x(:) > 0);
  if (! all (tf))
    label = inputname (1);
    if (isempty (label))
      label = "input";
    endif
    bad_idx = find (! tf);
    try
      bad_val = x(bad_idx);
      errmsg = sprintf ("%s must be positive; found %d elements that were not: values %s", ...
                        label, numel (bad_idx), mat2str (bad_val));
    catch
      errmsg = sprintf ("%s must be positive; found %d elements that were not: indexes %s", ...
                        label, numel (bad_idx), mat2str (bad_idx));
    end_try_catch
    error (errmsg);
  endif

endfunction


%!test
%! mustBePositive (1);
%! mustBePositive (123.456);
%! mustBePositive (Inf);
%! mustBePositive (eps);

%!error <Invalid call> mustBePositive ()
%!error <found 1 elements> mustBePositive (0)
%!error <found 1 elements> mustBePositive (0:10)
%!error <found 2 elements> mustBePositive ([-1 -2])
%!error <found 2 elements> mustBePositive ([0 1 2 3 -4])
%!error <found 1 elements> mustBePositive (-Inf)
%!error <found 1 elements> mustBePositive (NaN)
