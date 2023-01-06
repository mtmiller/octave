########################################################################
##
## Copyright (C) 2006-2023 The Octave Project Developers
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

%!function y = f (x)
%!  if (x == 1)
%!    y = x;
%!    return;
%!  else
%!    y = x * f (x-1);
%!  endif
%!endfunction
%!
%!assert (f (5), 120)

%!function y = f (x)
%!  if (x == 1)
%!    y = x;
%!    return;
%!  else
%!    y = f (x-1) * x;
%!  endif
%!endfunction
%!
%!assert (f (5), 120)

%!function r = f (x)
%!  persistent p = 1;
%!  if (x == 1)
%!    f (x + 1);
%!    r = p;
%!  else
%!    clear p
%!    p = 13;
%!  endif
%!endfunction
%!
%!error <'p' undefined> f (1)

%!function r = f (x)
%!  persistent p = 1;
%!  if (x == 1)
%!    f (x + 1);
%!    r = p;
%!  else
%!    p = 13;
%!  endif
%!endfunction
%!
%!assert (f (1), 13)


%%FIXME: Need test for maximum recursion depth
