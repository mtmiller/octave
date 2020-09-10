########################################################################
##
## Copyright (C) 2007-2020 The Octave Project Developers
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
## @deftypefn  {} {@var{T} =} delaunayn (@var{pts})
## @deftypefnx {} {@var{T} =} delaunayn (@var{pts}, @var{options})
## Compute the Delaunay triangulation for an N-dimensional set of points.
##
## The Delaunay triangulation is a tessellation of the convex hull of a set of
## points such that no N-sphere defined by the N-triangles contains any other
## points from the set.
##
## The input matrix @var{pts} of size [n, dim] contains n points in a space of
## dimension dim.  The return matrix @var{T} has size [m, dim+1].  Each row of
## @var{T} contains a set of indices back into the original set of points
## @var{pts} which describes a simplex of dimension dim.  For example, a 2-D
## simplex is a triangle and 3-D simplex is a tetrahedron.
##
## An optional second argument, which must be a string or cell array of
## strings, contains options passed to the underlying qhull command.  See the
## documentation for the Qhull library for details
## @url{http://www.qhull.org/html/qh-quick.htm#options}.
## The default options depend on the dimension of the input:
##
## @itemize
## @item 2-D and 3-D: @var{options} = @code{@{"Qt", "Qbb", "Qc"@}}
##
## @item 4-D and higher: @var{options} = @code{@{"Qt", "Qbb", "Qc", "Qx"@}}
## @end itemize
##
## If Qhull fails for 2-D input the triangulation is attempted again with
## the options @code{@{"Qt", "Qbb", "Qc", "Qz"@}} which may result in
## reduced accuracy.
##
## If @var{options} is not present or @code{[]} then the default arguments are
## used.  Otherwise, @var{options} replaces the default argument list.
## To append user options to the defaults it is necessary to repeat the
## default arguments in @var{options}.  Use a null string to pass no arguments.
##
## @seealso{delaunay, convhulln, voronoin, trimesh, tetramesh}
## @end deftypefn

function T = delaunayn (pts, varargin)

  if (nargin < 1)
    print_usage ();
  endif

  if (isempty (varargin) || isempty (varargin{1}))
    try
      T = __delaunayn__ (pts);
    catch
      if (columns (pts) <= 2)
        T = __delaunayn__ (pts, "Qt Qbb Qc Qz");
      endif
    end_try_catch
  else
    T = __delaunayn__ (pts, varargin{:});
  endif

  if (isa (pts, "single"))
    tol = 1e3 * eps ("single");
  else
    tol = 1e3 * eps;
  endif

  ## Try to remove the zero volume simplices.  The volume of the i-th simplex is
  ## given by abs(det(pts(T(i,1:end-1),:)-pts(T(i,2:end),:)))/factorial(ndim+1)
  ## (reference http://en.wikipedia.org/wiki/Simplex).  Any simplex with a
  ## relative volume less than some arbitrary criteria is rejected.  The
  ## criteria we use is the volume of the simplex corresponding to an
  ## orthogonal simplex is equal edge length all equal to the edge length of
  ## the original simplex.  If the relative volume is 1e3*eps then the simplex
  ## is rejected.  Note division of the two volumes means that the factor
  ## factorial(ndim+1) is dropped.
  [nt, nd] = size (T);
  if (nd == 3)
    ## 2-D case
    np = rows (pts);
    ptsz = [pts, zeros(np, 1)];
    p1 = ptsz(T(:,1), :);
    p2 = ptsz(T(:,2), :);
    p3 = ptsz(T(:,3), :);
    p12 = p1 - p2;
    p23 = p2 - p3;
    det = cross (p12, p23, 2);
    idx = abs (det(:,3) ./ sqrt (sumsq (p12, 2))) < tol & ...
          abs (det(:,3) ./ sqrt (sumsq (p23, 2))) < tol;
  else
    ## FIXME: Vectorize this for loop or convert delaunayn to .oct function
    idx = [];
    for i = 1:nt
      X = pts(T(i,1:end-1),:) - pts(T(i,2:end),:);
      if (abs (det (X)) / sqrt (sumsq (X, 2)) < tol)
        idx(end+1) = i;
      endif
    endfor
  endif
  T(idx,:) = [];

endfunction


%!testif HAVE_QHULL
%! x = [-1, 0; 0, 1; 1, 0; 0, -1; 0, 0];
%! assert (sortrows (sort (delaunayn (x), 2)), [1,2,5;1,4,5;2,3,5;3,4,5]);

## Test 3-D input
%!testif HAVE_QHULL
%! x = [-1, -1, 1, 0, -1]; y = [-1, 1, 1, 0, -1]; z = [0, 0, 0, 1, 1];
%! assert (sortrows (sort (delaunayn ([x(:) y(:) z(:)]), 2)), [1,2,3,4;1,2,4,5]);

## FIXME: Need tests for delaunayn

## Input validation tests
%!error delaunayn ()
