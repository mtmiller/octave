# Copyright (C) 1996 A. Scottedward Hodel 
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
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 
function [y,x] = lsim(sys,u,t,x0)
# lsim: Produce output for a linear simulation of a system
#
# lsim(sys,u,t,[x0])
# Produces a plot for the output of the system, sys.
#
# U is an array that contains the system's inputs.  Each column in u 
# corresponds to a different time step.  Each row in u corresponds to a 
# different input.  T is an array that contains the time index of the 
# system.  T should be regularly spaced.  If initial conditions are required
# on the system, the x0 vector should be added to the argument list.
#
# When the lsim function is invoked with output parameters:
# [y,x] = lsim(sys,u,t,[x0])
# a plot is not displayed, however, the data is returned in y = system output
# and x = system states.

# Written by David Clem, A. S. Hodel July 1995
# modified by John Ingram for system format August 1996
# $Revision: 1.1.1.1 $
# $Log: lsim.m,v $
# Revision 1.1.1.1  1998/05/19 20:24:07  jwe
#
# Revision 1.3  1997/12/01 16:51:50  scotte
# updated by Mueller 27 Nov 97
#
# Revision 1.2  1997/11/24  16:13:39  mueller
# old style c2d call fixed
#

  if((nargin < 3)||(nargin > 4))
    usage("[y,x] = lsim(sys,u,t[,x0])");
  endif

  if(!is_struct(sys))
    error("sys must be in system data structure");
  endif

  sys = sysupdate(sys,"ss");

  nout = rows(sys.c);
  nin = columns(sys.b);
  ncstates = sys.n;
  ndstates = sys.nz;
  
  a = sys.a;
  b = sys.b;
  c = sys.c;
  d = sys.d;
  
  if (nargin == 3)
    x0 = zeros(columns(a),1);
  endif

  if(rows(u) ~= length(t))
    error("lsim: There should be an input value (row) for each time instant");
  endif
  if(columns(u) ~= columns(d))
    error("lsim: U and d should have the same number of inputs");
  endif
  if(columns(x0) > 1)
    error("lsim: Initial condition vector should have only one column");
  endif
  if(rows(x0) > rows(a))
    error("lsim: Initial condition vector is too large");
  endif

  Ts = 0;
  t(2)-t(1);
  u=u';
  n = max(size(t));
  for ii = 1:(n-1)

    # check if step size changed
    if (t(ii+1) - t(ii) != Ts)
      Ts = t(ii+1) - t(ii);
      # [F,G] = c2d(a,b,Ts);
      dsys = c2d(sys, Ts);
      F = dsys.a;
      G = dsys.b;
    endif

    x(:,ii) = x0;
    x0 = F*x0 + G*u(:,ii);
  endfor

  # pick up last point
  x(:,n) = x0;

  y = c*x + d*u;
  if(nargout == 0)
   plot(t,y);
   y=[];
   x=[];
  endif
endfunction
