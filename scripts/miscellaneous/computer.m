########################################################################
##
## Copyright (C) 2004-2021 The Octave Project Developers
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
## @deftypefn  {} {} computer ()
## @deftypefnx {} {@var{c} =} computer ()
## @deftypefnx {} {[@var{c}, @var{maxsize}] =} computer ()
## @deftypefnx {} {[@var{c}, @var{maxsize}, @var{endian}] =} computer ()
## @deftypefnx {} {@var{arch} =} computer ("arch")
## Print or return a string of the form @var{cpu}-@var{vendor}-@var{os} that
## identifies the type of computer that Octave is running on.
##
## If invoked with an output argument, the value is returned instead of
## printed.  For example:
##
## @example
## @group
## computer ()
##    @print{} x86_64-pc-linux-gnu
##
## mycomp = computer ()
##    @result{} mycomp = x86_64-pc-linux-gnu
## @end group
## @end example
##
## If two output arguments are requested, also return the maximum number of
## elements for an array.  This will depend on whether Octave has been
## compiled with 32-bit or 64-bit index vectors.
##
## If three output arguments are requested, also return the byte order of the
## current system as a character (@qcode{"B"} for big-endian or @qcode{"L"}
## for little-endian).
##
## If the argument @qcode{"arch"} is specified, return a string indicating the
## architecture of the computer on which Octave is running.
## @seealso{isunix, ismac, ispc}
## @end deftypefn

function [c, maxsize, endian] = computer (a)

  if (nargin > 1)
    print_usage ();
  elseif (nargin == 1 && ! strcmpi (a, "arch"))
    error ('computer: "arch" is only valid argument');
  endif

  if (nargin == 0)
    msg = __octave_config_info__ ("canonical_host_type");

    if (strcmp (msg, "unknown"))
      msg = "Hi Dave, I'm a HAL-9000";
    endif

    if (nargout == 0)
      disp (msg);
    else
      c = msg;
      if (isargout (2))
        if (__octave_config_info__ ("ENABLE_64"))
          maxsize = 2^63-1;
        else
          maxsize = 2^31-1;
        endif
      endif
      if (isargout (3))
        if (__octave_config_info__ ("words_big_endian"))
          endian = "B";
        elseif (__octave_config_info__ ("words_little_endian"))
          endian = "L";
        else
          endian = "?";
        endif
      endif
    endif
  else
    ## "arch" argument asked for
    tmp = ostrsplit (__octave_config_info__ ("canonical_host_type"), "-");
    if (numel (tmp) == 4)
      c = sprintf ("%s-%s-%s", tmp{4}, tmp{3}, tmp{1});
    else
      c = sprintf ("%s-%s", tmp{3}, tmp{1});
    endif

  endif

endfunction


%!assert (ischar (computer ()))
%!assert (computer (), __octave_config_info__ ("canonical_host_type"))
%!assert (ischar (computer ("arch")))

%!error computer (1,2)
%!error <"arch" is only valid argument> computer ("xyz")
