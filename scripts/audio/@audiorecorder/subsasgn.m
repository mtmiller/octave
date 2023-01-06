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
## @deftypefn {} {@var{value} =} subsasgn (@var{recorder}, @var{idx}, @var{rhs})
## Perform subscripted assignment on the audio recorder object @var{recorder}.
##
## Assign the value of @var{rhs} to the recorder property named by @var{idx}.
## @seealso{@audiorecorder/audiorecorder}
## @end deftypefn

function recorder = subsasgn (recorder, idx, rhs)

  if (nargin != 3)
    print_usage ();
  endif

  if (isempty (idx))
    error ("@audiorecorder/subsasgn: missing index");
  endif

  if (strcmp (idx(1).type, "."))
    field = idx.subs;
    set (recorder, field, rhs);
  else
    error ("@audiorecorder/subsasgn: invalid subscript type");
  endif

endfunction


%!testif HAVE_PORTAUDIO; audiodevinfo (1) > 0
%! recorder = audiorecorder (44100, 16, 2);
%! recorder.Tag = "mytag";
%! assert (get (recorder, "Tag"), "mytag");

## Test input validation
%!testif HAVE_PORTAUDIO; audiodevinfo (1) > 0
%! recorder = audiorecorder (44100, 16, 2);
%! fail ("recorder(1).Tag = 5", "invalid subscript type");
%! fail ("recorder{1}.Tag = 5", "invalid subscript type");
