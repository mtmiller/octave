# Copyright (C) 1993, 1994, 1995 Auburn University.  All Rights Reserved.
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA.

function [n, m, p] = abcddim (a, b, c, d)

# Usage: [n, m, p] = abcddim (a, b, c, d)
#
# Check for compatibility of the dimensions of the matrices defining
# the linear system (a, b, c, d).
#
# Returns n = number of system states,
#         m = number of system inputs,
#         p = number of system outputs.
#
# Note: n = 0 (pure gain block) is returned without warning.
#
# Returns n = m = p = -1 if the system is not compatible.
#
# See also: is_abcd

# Written by A. S. Hodel (scotte@eng.auburn.edu) August 1993.
# a s hodel: modified to accept pure-gain systems aug 1996

  if (nargin != 4)
    error ("abcddim: four arguments required");
  endif

  n = m = p = -1;

  [a,an,am] = abcddims(a);
  [b,bn,bm] = abcddims(b);
  [c,cn,cm] = abcddims(c);
  [d,dn,dm] = abcddims(d);

  if ( (!is_square(a)) & (!isempty(a)) )
    warning (["abcddim: a is not square (",num2str(an),"x",num2str(am),")"]);
    return
  endif

  if( (bm == 0) & (dm == 0) )
    warning("abcddim: no inputs");
  elseif (bn != am)
    warning (["abcddim: a(",num2str(an),"x",num2str(am), ...
      " and b(",num2str(bn),"x",num2str(bm),") are not compatible"]);
    return
  endif

  if( (cn == 0) & (dn == 0 ) )
    warning("abcddim: no outputs");
  elseif (cm != an)
    warning (["abcddim: a(",num2str(an),"x",num2str(am), ...
	" and c(",num2str(cn),"x",num2str(cm),") are not compatible"]);
    return
  endif

  have_connections = (bn*cn != 0);

  if( (dn == 0) & have_connections)
    warning("abcddim: empty d matrix passed; setting compatibly with b, c");
    [d,dn,dm] = abcddims(zeros(cn,bm));
  endif

  if(an > 0)
    [dn, dm] = size(d);
    if ( (cn != dn) & have_connections )
      warning (["abcddim: c(",num2str(cn),"x",num2str(cm), ...
	" and d(",num2str(dn),"x",num2str(dm),") are not compatible"]);
      return
    endif

    if ( (bm != dm) & have_connections )
      warning (["abcddim: b(",num2str(bn),"x",num2str(bm), ...
	  " and d(",num2str(dn),"x",num2str(dm),") are not compatible"]);
      return
    endif

    m = bm;
    p = cn;
  else
    [p,m] = size(d);
  endif
  n = an;
endfunction
