c Copyright (C) 2009-2023 The Octave Project Developers
c
c See the file COPYRIGHT.md in the top-level directory of this
c distribution or <https://octave.org/copyright/>.
c
c Octave is free software: you can redistribute it and/or modify it
c under the terms of the GNU General Public License as published by
c the Free Software Foundation, either version 3 of the License, or
c (at your option) any later version.
c
c Octave is distributed in the hope that it will be useful, but
c WITHOUT ANY WARRANTY; without even the implied warranty of
c MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
c GNU General Public License for more details.
c
c You should have received a copy of the GNU General Public License
c along with Octave; see the file COPYING.  If not, see
c <https://www.gnu.org/licenses/>.
c
      subroutine cmatm3(m,n,k,np,a,b,c)
c purpose:      a 3-dimensional matrix product.
c               given a (m,k,np) array a and (k,n,np) array b,
c               calculates a (m,n,np) array c such that
c                 for i = 1:np
c                 c(:,:,i) = a(:,:,i) * b(:,:,i)
c
c arguments:
c m,n,k (in)    the dimensions
c np (in)       number of multiplications
c a (in)        a complex input array, size (m,k,np)
c b (in)        a complex input array, size (k,n,np)
c c (out)       a complex output array, size (m,n,np)
      integer m,n,k,np
      complex a(m*k,np),b(k*n,np)
      complex c(m*n,np)

      complex cdotu,one,zero
      parameter (one = 1e0, zero = 0e0)
      external cdotu,cgemv,cgemm
      integer i

c quick return if possible.
      if (np <= 0) return

      if (m == 1) then
        if (n == 1) then
          do i = 1,np
            c(1,i) = cdotu(k,a(1,i),1,b(1,i),1)
          end do
        else
          do i = 1,np
            call cgemv("T",k,n,one,b(1,i),k,a(1,i),1,zero,c(1,i),1)
          end do
        end if
      else
        if (n == 1) then
          do i = 1,np
            call cgemv("N",m,k,one,a(1,i),m,b(1,i),1,zero,c(1,i),1)
          end do
        else
          do i = 1,np
            call cgemm("N","N",m,n,k,
     +                 one,a(1,i),m,b(1,i),k,zero,c(1,i),m)
          end do
        end if
      end if

      end subroutine
