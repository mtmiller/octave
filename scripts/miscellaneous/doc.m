## Copyright (C) 2005 Soren Hauberg
## 
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Command} doc @var{function_name}
## Displays documentation for the function @var{function_name}.
## For example, if you want to see the documentation for the Octave
## random number generator @code{rand}, type
## @example
## @code{doc rand}
## @end example
## @seealso{help}
## @end deftypefn

## Author: Soren Hauberg <soren@hauberg.org>
## Adapted-by: jwe

## PKG_ADD: mark_as_command doc

function retval = doc (fname)

  if (nargin == 0 || nargin == 1)

    ftype = 0;

    if (nargin == 1)
      ## Get the directory where the function lives.
      ## FIXME -- maybe we should have a better way of doing this.

      if (ischar (fname))
	ftype = exist (fname);
      else
	error ("doc: expecting argument to be a character string");
      endif
    else
      fname = "";
    endif

    if (ftype == 2 || ftype == 3)
      ffile = file_in_loadpath (strcat (fname, "."));
    else
      ffile = "";
    endif

    if (isempty (ffile))
      info_dir = octave_config_info ("infodir");
    else
      info_dir = fileparts (ffile);
    endif

    ## Determine if a file called doc.info exist in the same 
    ## directory as the function.

    info_file_name = fullfile (info_dir, "doc.info");

    [stat_info, err] = stat (info_file_name);

    if (err < 0)
      info_file_name = info_file ();
    endif

    ## FIXME -- don't change the order of the arguments below because
    ## the info-emacs-info script currently expects --directory DIR as
    ## the third and fourth arguments.  Someone should fix that.

    cmd = sprintf ("\"%s\" --file \"%s\" --directory \"%s\"",
		   info_program (), info_file_name, info_dir);

    if (! isempty (fname))
      cmd = sprintf ("%s --index-search %s", cmd, fname);
    endif

    status = system (cmd);

    if (status == 127)
      warning ("unable to find info program `%s'", info_program ());
    endif

    if (nargout > 0)
      retval = status;
    endif

  else
    print_usage ();
  endif

endfunction
