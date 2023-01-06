########################################################################
##
## Copyright (C) 2000-2023 The Octave Project Developers
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
## @deftypefn  {} {@var{val} =} getfield (@var{s}, @var{field})
## @deftypefnx {} {@var{val} =} getfield (@var{s}, @var{sidx1}, @var{field1}, @var{fidx1}, @dots{})
## Get the value of the field named @var{field} from a structure or nested
## structure @var{s}.
##
## If @var{s} is a structure array then @var{sidx} selects an element of the
## structure array, @var{field} specifies the field name of the selected
## element, and @var{fidx} selects which element of the field (in the case of
## an array or cell array).  For a more complete description of the syntax,
## @pxref{XREFsetfield,,@code{setfield}}.
##
## @seealso{setfield, rmfield, orderfields, isfield, fieldnames, isstruct,
## struct}
## @end deftypefn

function val = getfield (s, varargin)

  if (nargin < 2)
    print_usage ();
  endif

  subs = varargin;
  flds = cellfun ("isclass", subs, "char");
  idxs = cellfun ("isclass", subs, "cell");
  if (! all (flds | idxs))
    error ("getfield: invalid index");
  endif

  typs = merge (flds, {"."}, {"()"});
  val = subsref (s, struct ("type", typs, "subs", subs));

endfunction


%!test
%! x.a = "hello";
%! assert (getfield (x, "a"), "hello");
%!test
%! ss(1,2).fd(3).b(1,4) = 5;
%! assert (getfield (ss,{1,2},"fd",{3},"b", {1,4}), 5);

## Test input validation
%!error <Invalid call> getfield ()
%!error <Invalid call> getfield (1)
%!error <invalid index> getfield (1,2)
