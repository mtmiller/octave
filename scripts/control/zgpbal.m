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
 
function [retsys] = zgpbal(Asys)
  # function [retsys] = zgpbal(Asys)
  #
  # used internally in tzero; minimal argument checking performed
  #
  # implementation of zero computation generalized eigenvalue problem 
  # balancing method (Hodel and Tiller, Allerton Conference, 1991)
  # Based on Ward's balancing algorithm (SIAM J. Sci Stat. Comput., 1981)
  #
  # zgpbal computes a state/input/output weighting that attempts to 
  # reduced the range of the magnitudes of the nonzero elements of [a,b,c,d]
  # The weighting uses scalar multiplication by powers of 2, so no roundoff
  # will occur.  
  #
  # zgpbal should be followed by zgpred
  # References:
  # ZGEP: Hodel, "Computation of Zeros with Balancing," 1992, Linear Algebra
  # and its Applications
  # Generalized CG: Golub and Van Loan, "Matrix Computations, 2nd ed" 1989
  
  # A. S. Hodel July 24 1992
  # Conversion to Octave by R. Bruce Tenison July 3, 1994
  # $Revision: 1.1 $
  # $Log: zgpbal.m,v $
# Revision 1.1  1998/11/04  14:35:42  hodel
# Initial revision
#
  # Revision 1.2  1998/08/24 15:50:31  hodelas
  # updated documentation
  #
  # Revision 1.1.1.1  1998/05/19 20:24:10  jwe
  #
  # Revision 1.2  1997/02/13 11:54:59  hodel
  # added debugging code (commented out).
  #

  if( (nargin != 1) | (!is_struct(Asys)))
    usage("retsys = zgpbal(Asys)");
  endif

  Asys = sysupdate(Asys,"ss");
  [a,b,c,d] = sys2ss(Asys);

  [nn,mm,pp] = abcddim(a,b,c,d);
  
  np1 = nn+1;
  nmp = nn+mm+pp;

  # set up log vector zz, incidence matrix ff
  zz = zginit(a,b,c,d);

  #disp("zgpbal: zginit returns")
  #zz
  #disp("/zgpbal")

  if (norm(zz))
    # generalized conjugate gradient approach
    xx = zgscal(a,b,c,d,zz,nn,mm,pp);
    
    for i=1:nmp
      xx(i) = floor(xx(i)+0.5);
      xx(i) = 2.0^xx(i);
    endfor
    
    # now scale a
    # block 1: a = sigma a inv(sigma)
    for i=1:nn
      a(i,1:nn) = a(i,1:nn)*xx(i);
      a(1:nn,i) = a(1:nn,i)/xx(i);
    endfor
    # block 2: b= sigma a phi
    for j=1:mm
      j1 = j+nn;
      b(1:nn,j) = b(1:nn,j)*xx(j1);
    endfor
    for i=1:nn
      b(i,1:mm) = b(i,1:mm)*xx(i);
    endfor
    for i=1:pp
      i1 = i+nn+mm;
      #   block 3: c = psi C inv(sigma)
      c(i,1:nn) = c(i,1:nn)*xx(i1);
    endfor
    for j=1:nn
      c(1:pp,j) = c(1:pp,j)/xx(j);
    endfor
    #   block 4: d = psi D phi
    for j=1:mm
      j1 = j+nn;
      d(1:pp,j) = d(1:pp,j)*xx(j1);
    endfor
    for i=1:pp
      i1 = i + nn + mm;
      d(i,1:mm) = d(i,1:mm)*xx(i1);
    endfor
  endif
  
  retsys = ss2sys(a,b,c,d);
endfunction

