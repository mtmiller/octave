########################################################################
##
## Copyright (C) 2014-2021 The Octave Project Developers
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
## @deftypefn  {} {@var{filename} =} fullfile (@var{dir1}, @var{dir2}, @dots{}, @var{file})
## @deftypefnx {} {@var{filenames} =} fullfile (@dots{}, @var{files})
## Build complete filename from separate parts.
##
## Joins any number of path components intelligently.  The return value is
## the concatenation of each component with exactly one file separator
## between each non empty part and at most one leading and/or trailing file
## separator.
##
## If the last component part is a cell array, returns a cell array of
## filepaths, one for each element in the last component, e.g.:
##
## @example
## @group
## fullfile ("/home/username", "data", @{"f1.csv", "f2.csv", "f3.csv"@})
##   @result{}
##       @{
##         [1,1] = /home/username/data/f1.csv
##         [1,2] = /home/username/data/f2.csv
##         [1,3] = /home/username/data/f3.csv
##       @}
## @end group
## @end example
##
## On Windows systems, while forward slash file separators do work, they are
## replaced by backslashes; in addition drive letters are stripped of leading
## file separators to obtain a valid file path.
##
## Note: @code{fullfile} does not perform any validation of the resulting full
## filename.
## @seealso{fileparts, filesep}
## @end deftypefn

function filename = fullfile (varargin)

  if (nargin && iscell (varargin{end}))
    filename = cellfun (@(x) fullfile (varargin{1:end-1}, x), varargin{end},
                                       "UniformOutput", false);
  else
    non_empty = cellfun ("isempty", varargin);
    unc = 0;
    if (ispc && ! isempty (varargin))
      varargin = strrep (varargin, '/', filesep);
      unc = strncmp (varargin{1}, '\\', 2);
      varargin(1) = regexprep (varargin{1}, '[\\/]*([a-zA-Z]:[\\/]*)', "$1");
    endif
    filename = strjoin (varargin(! non_empty), filesep);
    filename(unc + strfind (filename(1+unc : end), [filesep filesep])) = "";
  endif

endfunction


%!shared fs, fsx, xfs, fsxfs, xfsy, xfsyfs
%! fs = filesep ();
%! fsx = [fs "x"];
%! xfs = ["x" fs];
%! fsxfs = [fs "x" fs];
%! xfsy = ["x" fs "y"];
%! xfsyfs = ["x" fs "y" fs];

%!assert (fullfile (""), "")
%!assert (fullfile (fs), fs)
%!assert (fullfile ("", fs), fs)
%!assert (fullfile (fs, ""), fs)
%!assert (fullfile ("", fs), fs)
%!assert (fullfile ("x"), "x")
%!assert (fullfile ("", "x"), "x")
%!assert (fullfile ("x", ""), "x")
%!assert (fullfile ("", "x", ""), "x")
%!assert (fullfile ("x", "y"), xfsy)
%!assert (fullfile ("x", "", "y"), xfsy)
%!assert (fullfile ("x", "", "y", ""), xfsy)
%!assert (fullfile ("", "x", "", "y", ""), xfsy)
%!assert (fullfile (fs), fs)
%!assert (fullfile (fs, "x"), fsx)
%!assert (fullfile (fs, xfs), fsxfs)
%!assert (fullfile (fsx, fs), fsxfs)
%!assert (fullfile (fs, "x", fs), fsxfs)

%!assert (fullfile ("x/", "/", "/", "y", "/", "/"), xfsyfs)
%!assert (fullfile ("/", "x/", "/", "/", "y", "/", "/"), [fs xfsyfs])
%!assert (fullfile ("/x/", "/", "/", "y", "/", "/"), [fs xfsyfs])

## different on purpose so that "fullfile (c{:})" works for empty c
%!assert (fullfile (), "")

%!assert (fullfile ("x", "y", {"c", "d"}), {[xfsyfs "c"], [xfsyfs "d"]})

## Windows specific - drive letters and file sep type
%!test
%! if (ispc)
%!   assert (fullfile ('\/\/\//A:/\/\', "x/", "/", "/", "y", "/", "/"), ...
%!           ['A:\' xfsyfs]);
%! endif

## *nix specific - double backslash
%!test
%! if (isunix || ismac)
%!   assert (fullfile (fs, fs), fs);
%! endif

## Windows specific - drive letters and file sep type, cell array
%!test
%! if (ispc)
%!  tmp = fullfile ({"\\\/B:\//", "A://c", "\\\C:/g/h/i/j\/"});
%!  assert (tmp{1}, 'B:\');
%!  assert (tmp{2}, 'A:\c');
%!  assert (tmp{3}, 'C:\g\h\i\j\');
%! endif
