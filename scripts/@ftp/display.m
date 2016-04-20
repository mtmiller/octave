## Copyright (C) 2009-2015 David Bateman
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

function display (obj)
  fprintf ("FTP Object\n");
  fprintf (" host: %s\n", obj.host);
  fprintf (" user: %s\n", obj.username);
  fprintf ("  dir: %s\n", __ftp_pwd__ (obj.curlhandle));
  fprintf (" mode: %s\n", __ftp_mode__ (obj.curlhandle));
endfunction


## No test possible for interactive function.
%!assert (1)

