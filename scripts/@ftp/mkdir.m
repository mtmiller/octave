########################################################################
##
## Copyright (C) 2009-2023 The Octave Project Developers
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
## @deftypefn {} {} mkdir (@var{f}, @var{path})
## Create the remote directory @var{path}, over the FTP connection @var{f}.
##
## @var{f} is an FTP object returned by the @code{ftp} function.
## @seealso{@ftp/rmdir, @ftp/ftp}
## @end deftypefn

function mkdir (f, path)
  __ftp_mkdir__ (f.curlhandle, path);
endfunction


## No test possible for interactive function.
%!assert (1)
