########################################################################
##
## Copyright (C) 2004-2023 The Octave Project Developers
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
## @deftypefn  {} {} delete @var{file}
## @deftypefnx {} {} delete @var{file1} @var{file2} @dots{}
## @deftypefnx {} {} delete (@var{file})
## @deftypefnx {} {} delete (@var{file1}, @var{file2}, @dots{})
## @deftypefnx {} {} delete (@var{handle})
## Delete the named file or graphics handle.
##
## @var{file} may contain globbing patterns such as @samp{*}.  Multiple files
## to be deleted may be specified in the same function call.
##
## @var{handle} may be a scalar or vector of graphic handles to delete.
##
## Programming Note: Deleting graphics objects is the proper way to remove
## features from a plot without clearing the entire figure.
## @seealso{clf, cla, unlink, rmdir}
## @end deftypefn

function delete (varargin)

  if (nargin == 0)
    print_usage ();
  endif

  if (iscellstr (varargin))
    for arg = varargin
      if (ispc ())
        files = __wglob__ (arg{1});
      else
        files = glob (arg{1});
      endif
      if (isempty (files))
        warning ("Octave:delete:no-such-file", ...
                 "delete: no such file: %s", arg{1});
      endif
      for i = 1:length (files)
        file = files{i};
        [err, msg] = unlink (file);
        if (err)
          warning ("Octave:delete:unlink-error", ...
                   "delete: %s: %s", file, msg);
        endif
      endfor
    endfor

  elseif (isscalar (varargin) && all (ishghandle (varargin{1}(:))))
    ## Delete a graphics object.
    __go_delete__ (varargin{1});

  else
    error ("Octave:delete:unsupported-object", ...
           "delete: first argument must be a filename or graphics handle");
  endif

endfunction


%!test
%! unwind_protect
%!   file = tempname ();
%!   tmp_var = pi;
%!   save (file, "tmp_var");
%!   assert (exist (file, "file"));
%!   delete (file);
%!   assert (! exist (file, "file"));
%! unwind_protect_cleanup
%!   sts = unlink (file);
%! end_unwind_protect

%!test
%! unwind_protect
%!   hf = figure ("visible", "off");
%!   hl = plot (1:10);
%!   assert (get (gca, "children"), hl);
%!   delete (hl);
%!   assert (get (gca, "children"), zeros (0,1));
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

## Test input validation
%!error <Invalid call> delete ()
%!error <first argument must be a filename> delete (struct ())
