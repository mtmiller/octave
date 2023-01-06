########################################################################
##
## Copyright (C) 2020-2023 The Octave Project Developers
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
## @deftypefn  {} {@var{retval} =} startsWith (@var{str}, @var{pattern})
## @deftypefnx {} {@var{retval} =} startsWith (@var{str}, @var{pattern}, "IgnoreCase", @var{ignore_case})
## Check whether string(s) start with pattern(s).
##
## Return an array of logical values that indicates which string(s) in the
## input @var{str} (a single string or cell array of strings) begin with
## the input @var{pattern} (a single string or cell array of strings).
##
## If the value of the parameter @qcode{"IgnoreCase"} is true, then the
## function will ignore the letter case of @var{str} and @var{pattern}.  By
## default, the comparison is case sensitive.
##
## Examples:
##
## @example
## @group
## ## one string and one pattern while considering case
## startsWith ("hello", "he")
##       @result{}  1
## @end group
##
## @group
## ## one string and one pattern while ignoring case
## startsWith ("hello", "HE", "IgnoreCase", true)
##       @result{}  1
## @end group
##
## @group
## ## multiple strings and multiple patterns while considering case
## startsWith (@{"lab work.pptx", "data.txt", "foundations.ppt"@},
##             @{"lab", "data"@})
##       @result{}  1  1  0
## @end group
##
## @group
## ## multiple strings and one pattern while considering case
## startsWith (@{"DATASHEET.ods", "data.txt", "foundations.ppt"@},
##             "data", "IgnoreCase", false)
##       @result{}  0  1  0
## @end group
##
## @group
## ## multiple strings and one pattern while ignoring case
## startsWith (@{"DATASHEET.ods", "data.txt", "foundations.ppt"@},
##             "data", "IgnoreCase", true)
##       @result{}  1  1  0
## @end group
## @end example
##
## @seealso{endsWith, regexp, strncmp, strncmpi}
## @end deftypefn

function retval = startsWith (str, pattern, IgnoreCase, ignore_case)

  if (nargin != 2 && nargin != 4)
    print_usage ();
  endif

  ## Validate input str and pattern
  if (! (iscellstr (str) || ischar (str)))
    error ("startsWith: STR must be a string or cell array of strings");
  endif
  if (! (iscellstr (pattern) || ischar (pattern)))
    error ("startsWith: PATTERN must be a string or cell array of strings");
  endif

  str = cellstr (str);
  pattern = cellstr (pattern);

  if (nargin == 2)
    ignore_case = false;
  else
    ## For Matlab compatibility accept any abbreviation of 3rd argument
    if (! ischar (IgnoreCase) || isempty (IgnoreCase)
        || ! strncmpi (IgnoreCase, "IgnoreCase", length (IgnoreCase)))
      error ('startsWith: third input must be "IgnoreCase"');
    endif

    if (! isscalar (ignore_case) || ! isreal (ignore_case))
      error ('startsWith: "IgnoreCase" value must be a logical scalar');
    endif
    ignore_case = logical (ignore_case);
  endif

  retval = false (size (str));
  if (ignore_case)
    for j = 1:numel (pattern)
      retval |= strncmpi (str, pattern{j}, length (pattern{j}));
    endfor
  else
    for j = 1:numel (pattern)
      retval |= strncmp (str, pattern{j}, length (pattern{j}));
    endfor
  endif

endfunction


## Test simple use with one string and one pattern
%!assert (startsWith ("hello", "he"))
%!assert (! startsWith ("hello", "HE"))
%!assert (startsWith ("hello", "HE", "i", 5))
%!assert (! startsWith ("hello", "no"))

## Test multiple strings with a single pattern
%!test
%! str = {"data science", "dataSheet.ods", "myFunc.m"; "foundations.ppt", ...
%!        "results.txt", "myFile.odt"};
%! pattern = "data";
%! expected = [true, true, false; false, false, false];
%! assert (startsWith (str, pattern), expected);

## Test multiple strings with multiple patterns
%!test
%! str = {"lab work.pptx", "myFile.odt", "data.txt", "foundations.ppt"};
%! pattern = {"lab", "data"};
%! expected = [true, false, true, false];
%! assert (startsWith (str, pattern), expected);

## Test IgnoreCase
%!test
%! str = {"DATASHEET.ods", "myFile.odt", "data.txt", "foundations.ppt"};
%! pattern = "data";
%! expected_ignore = [true, false, true, false];
%! expected_wo_ignore = [false, false, true, false];
%! assert (startsWith (str, pattern, "IgnoreCase", true), expected_ignore);
%! assert (startsWith (str, pattern, "IgnoreCase", false), expected_wo_ignore);
%! assert (startsWith (str, pattern, "I", 500), expected_ignore);
%! assert (startsWith (str, pattern, "iG", 0), expected_wo_ignore);

## Test input validation
%!error <Invalid call> startsWith ()
%!error startsWith ("A")
%!error startsWith ("A", "B", "C")
%!error startsWith ("A", "B", "C", "D", "E")
%!error <STR must be a string> startsWith (152, "hi")
%!error <STR must be a .* cell array of strings> startsWith ({152}, "hi")
%!error <PATTERN must be a string> startsWith ("hi", 152)
%!error <PATTERN must be a .* cell array of strings> startsWith ("hi", {152})
%!error <third input must be "IgnoreCase"> startsWith ("hello", "he", 1, 1)
%!error <third input must be "IgnoreCase"> startsWith ("hello", "he", "", 1)
%!error <third input must be "IgnoreCase"> startsWith ("hello", "he", "foo", 1)
%!error <"IgnoreCase" value must be a logical scalar>
%! startsWith ("hello", "hi", "i", "true");
%!error <"IgnoreCase" value must be a logical scalar>
%! startsWith ("hello", "hi", "i", {true});
