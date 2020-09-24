########################################################################
##
## Copyright (C) 2005-2020 The Octave Project Developers
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
## @deftypefn  {} {} unzip (@var{zipfile})
## @deftypefnx {} {} unzip (@var{zipfile}, @var{dir})
## @deftypefnx {} {@var{filelist} =} unzip (@dots{})
## Unpack the ZIP archive @var{zipfile}.
##
## If @var{dir} is specified the files are unpacked in this directory rather
## than the current directory.
##
## The optional output @var{filelist} is a list of the uncompressed files.
## @seealso{zip, unpack, bunzip2, gunzip, untar}
## @end deftypefn

function filelist = unzip (zipfile, dir = [])

  if (nargin < 1)
    print_usage ();
  endif

  if (nargout > 0)
    filelist = unpack (zipfile, dir, "zip");
  else
    unpack (zipfile, dir, "zip");
  endif

endfunction


## Tests for this m-file are located in zip.m
## Remove from test statistics
%!assert (1)
