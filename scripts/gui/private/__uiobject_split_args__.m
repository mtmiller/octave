########################################################################
##
## Copyright (C) 2012-2024 The Octave Project Developers
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
## @deftypefn {} {[@var{p}, @var{args}] =} __uiobject_split_args__ (@var{who}, @var{in_args}, @var{parent_type}, @var{use_gcf})
## Undocumented internal function.
## @end deftypefn

function [parent, args] = __uiobject_split_args__ (who, in_args, parent_type = {}, use_gcf = 1)

  parent = [];
  args = {};
  offset = 1;

  if (! isempty (in_args))
    if (ishghandle (in_args{1}))
      parent = in_args{1};
      offset = 2;
    elseif (! ischar (in_args{1}) && ! isstruct (in_args{1}))
      error ("%s: invalid parent handle", who);
    endif

    args = in_args(offset:end);
  endif

  if (! isempty (args))
    i = find (strcmpi (args(1:2:end), "parent"), 1, "first");
    if (! isempty (i) && numel (args) >= 2*i)
      parent = args{2*i};
      if (! ishghandle (parent))
        error ("%s: invalid parent handle", who);
      endif
      args((2*i-1):2*i) = [];
    endif
  endif

  if (! isempty (parent))
    if (! isempty (parent_type)
        && isempty (find (strcmpi (get (parent, "type"), parent_type))))
      error ("%s: invalid parent, the parent type must be: %s", ...
             who, sprintf ("%s, ", parent_type{:})(1:end-2));
    endif
  elseif (use_gcf)
    parent = gcf ();
  endif

endfunction
