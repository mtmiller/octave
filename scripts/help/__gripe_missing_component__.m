########################################################################
##
## Copyright (C) 2013-2023 The Octave Project Developers
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
## @deftypefn {} {} __gripe_missing_component__ (@var{caller}, @var{component})
## Undocumented internal function.
## @end deftypefn

function __gripe_missing_component__ (caller, component)

  if (nargin != 2)
    print_usage ();
  endif

  msg = "";
  fcn = missing_component_hook ();

  ftype = exist (fcn);
  if (ftype == 2 || ftype == 3 || ftype == 5 || ftype == 103)
    msg = feval (fcn, component);
  endif

  if (isempty (msg))
    switch (component)
      case "info-file"
        msg = "unable to find the Octave info manual, Octave installation is incomplete";
      case "mkoctfile"
        msg = "unable to find the mkoctfile command, Octave installation is incomplete";
      case "octave"
        msg = "unable to find the octave executable, Octave installation is incomplete";
      case "octave-config"
        msg = "unable to find the octave-config command, Octave installation is incomplete";
      otherwise
        msg = ['unable to find required Octave component "' component '"'];
    endswitch
  endif

  error ("%s: %s\n", caller, msg);

endfunction


## NOTE: Tests cannot rely on the exact error strings shown above because we
##       specifically allow these messages to be overridden by
##       missing_component_hook.  The prefix is all we can be sure of.
%!error <abc: .*> __gripe_missing_component__ ("abc", "info-file")
%!error <abc: .*> __gripe_missing_component__ ("abc", "octave")
%!error <abc: .*> __gripe_missing_component__ ("abc", "octave-config")
%!error <abc: .*> __gripe_missing_component__ ("abc", "xyz")

%!error <Invalid call> __gripe_missing_component__ ()
%!error <Invalid call> __gripe_missing_component__ ("fcn")
