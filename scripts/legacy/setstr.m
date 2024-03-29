########################################################################
##
## Copyright (C) 2003-2024 The Octave Project Developers
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
## @deftypefn {} {@var{s} =} setstr (@var{x})
## This function is obsolete.  Use @code{char} instead.
## @seealso{char}
## @end deftypefn

## At one time, Matlab docs stated that this function is obsolete and would be
## removed in some future version.  Now users are told that it should be
## avoided, but there is no mention of possible future removal.

function retval = setstr (varargin)

  persistent warned = false;
  if (! warned)
    warned = true;
    warning ("Octave:legacy-function",
             "setstr is obsolete; please use char instead");
  endif

  retval = char (varargin{:});

endfunction
