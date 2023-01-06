########################################################################
##
## Copyright (C) 2021-2023 The Octave Project Developers
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
## @deftypefn  {} {@var{val} =} disable_range ()
## @deftypefnx {} {@var{old_val} =} disable_range (@var{new_val})
## @deftypefnx {} {@var{old_val} =} disable_range (@var{new_val}, "local")
##
## @code{disable_range} is deprecated and will be removed in Octave version 9.
## Use @code{optimize_range} instead.
##
## Query or set whether storing ranges in a special space-efficient format is
## disabled.
##
## The default value is false.  If this option is set to true, Octave will
## store ranges as full matrices.
##
## When called from inside a function with the @qcode{"local"} option, the
## setting is changed locally for the function and any subroutines it calls.
## The original setting is restored when exiting the function.
## @seealso{disable_diagonal_matrix, disable_permutation_matrix}
## @end deftypefn

## FIXME: DEPRECATED: Remove in version 9.

function retval = disable_range (varargin)

  persistent warned = false;
  if (! warned)
    warned = true;
    warning ("Octave:deprecated-function",
             "disable_range is obsolete and will be removed from a future version of Octave, please use optimize_range instead\n");
  endif

  if (nargin == 0)
    retval = ! optimize_range ();
  elseif (nargout == 0)
    optimize_range (! varargin{1}, varargin{2:end});
  else
    retval = ! optimize_range (! varargin{1}, varargin{2:end});
  endif

endfunction
