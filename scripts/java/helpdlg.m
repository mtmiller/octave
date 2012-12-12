## Copyright (C) 2010 Martin Hepperle
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
## @deftypefn  {Function File} {@var{h} =} helpdlg (@var{msg})
## @deftypefnx {Function File} {@var{h} =} helpdlg (@var{msg}, @var{title})
## Display @var{msg} in a help dialog box.
##
## The message may have multiple lines separated by newline characters
## ("\n"), or it may be a cellstr array with one element for each
## line.  The optional input @var{title} (character string) can be used to
## set the dialog caption.  The default title is "Help Dialog".
##
## The return value is always 1.
## @seealso{errordlg, inputdlg, listdlg, msgbox, questdlg, warndlg}
## @end deftypefn

function h = helpdlg (msg, title = "Help Dialog")

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  if (! ischar (msg))
    if (iscell (msg))
      msg = sprintf ("%s\n", msg{:});
      msg(end) = "";
    else
      error ("helpdlg: MSG must be a string or cellstr array");
    endif
  endif

  if (! ischar (title))
    error ("helpdlg: TITLE must be a character string");
  endif

  h = java_invoke ("org.octave.JDialogBox", "helpdlg", msg, title);

endfunction

