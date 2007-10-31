/*

Copyright (C) 2006, 2007 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either
version 3  of the License, or (at your option) any later 
version.

Octave is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public 
License along with Octave; see the file COPYING.  If not,
see <http://www.gnu.org/licenses/>.

*/

#include <octave/oct.h>
#include <octave/parse.h>

DEFUN_DLD (funcdemo, args, nargout, "Function Demo")
{
  int nargin = args.length();
  octave_value_list retval;

  if (nargin < 2)
    print_usage ();
  else
    {
      octave_value_list newargs;
      for (octave_idx_type i = nargin - 1; i > 0; i--)
        newargs (i - 1) = args(i);
      if (args(0).is_function_handle ()
          || args(0).is_inline_function ())
        {
          octave_function *fcn = args(0).function_value ();
          if (! error_state)
            retval = feval (fcn, newargs, nargout);
        }
      else if (args(0).is_string ())
        {
          std::string fcn = args (0).string_value ();
          if (! error_state)
            retval = feval (fcn, newargs, nargout);
        }
      else
        error ("funcdemo: expected string,",
	       " inline or function handle");
    }
  return retval;
}
