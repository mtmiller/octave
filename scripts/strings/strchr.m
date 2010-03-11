## Copyright (C) 2008, 2009 Jaroslav Hajek
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

## -*- texinfo -*-
## @deftypefn {Function File} {@var{idx} =} strchr (@var{str}, @var{chars})
## @deftypefnx {Function File} {@var{idx} =} strchr (@var{str}, @var{chars}, @var{n})
## @deftypefnx {Function File} {@var{idx} =} strchr (@var{str}, @var{chars}, @var{n}, @var{direction})
## Search for the string @var{str} for occurrences of characters from the set @var{chars}.
## The return value, as well as the @var{n} and @var{direction} arguments behave
## identically as in @code{find}.
##
## This will be faster than using regexp in most cases.
##
## @seealso{find}
## @end deftypefn

function varargout = strchr (str, chars, varargin)
  if (nargin < 2 || ! ischar (str) || ! ischar (chars))
    print_usage ();
  endif
  if (isempty (chars))
    mask = false (size (str));
  elseif (length (chars) <= 6)
    ## With a few characters, it pays off to build the mask incrementally.
    ## We do it via a for loop to save memory.
    mask = str == chars(1);
    for i = 2:length (chars)
      mask |= str == chars(i);
    endfor
  else
    ## Index the str into a mask of valid values. This is slower than it could be
    ## because of the +1 issue.
    f = false (1, 256);
    f(chars + 1) = true;
    si = uint32 (str); # default goes via double - unnecessarily long.
    ++si; # in-place
    mask = reshape (f(si), size (str));
  endif
  varargout = cell (1, nargout);
  varargout{1} = [];
  [varargout{:}] = find (mask, varargin{:});
endfunction 

%!assert(strchr("Octave is the best software","best"),[3, 6, 9, 11, 13, 15, 16, 17, 18, 20, 23, 27])
