########################################################################
##
## Copyright (C) 2005-2024 The Octave Project Developers
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
## @deftypefn {} {} __printf_assert__ (@dots{})
## Undocumented internal function.
## @end deftypefn

function __printf_assert__ (varargin)
  global __assert_printf__ = "";

  __assert_printf__ = cat (2, __assert_printf__, sprintf (varargin{:}));

endfunction


## No test coverage for internal function.  It is tested through calling fcn.
%!assert (1)
