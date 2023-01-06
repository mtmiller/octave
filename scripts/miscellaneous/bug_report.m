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
## @deftypefn {} {} bug_report ()
## Display information about how to submit bug reports for Octave.
## @end deftypefn

function bug_report ()

  disp (" ");
  disp ("  Bug reports play an essential role in making Octave");
  disp ("  reliable.  Please use the Octave bug tracker at");
  disp (" ");
  disp ("    https://bugs.octave.org");
  disp (" ");
  disp ("  to report problems.");
  disp (" ");
  disp ("  Please also read the bug reporting guidelines at");
  disp (" ");
  disp ("    https://www.octave.org/bugs.html");
  disp (" ");
  disp ("  to learn how to submit useful bug reports that will");
  disp ("  help the Octave community diagnose and fix the problem");
  disp ("  quickly and efficiently.");
  disp (" ");

endfunction


## Mark file as being tested.  No real test needed for this function.
%!assert (1)
