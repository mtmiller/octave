## Copyright (C) 2013-2018 Vytautas Jančauskas
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

## -*- texinfo -*-
## @deftypefn {} {@var{player} =} getplayer (@var{recorder})
## Return an audioplayer object with data recorded by the audiorecorder object
## @var{recorder}.
## @end deftypefn

function player = getplayer (varargin)

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  recorder = varargin{1};
  data = getaudiodata (recorder);
  player = audioplayer (data, get (recorder, "SampleRate"),
                        get (recorder, "BitsPerSample"));

endfunction
