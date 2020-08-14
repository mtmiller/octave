########################################################################
##
## Copyright (C) 2006-2020 The Octave Project Developers
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

function sparseimages (d, nm, typ)

  if (strcmp (typ, "txt"))
    txtimages (d, nm, 15, typ);
    return;
  endif

  set_graphics_toolkit ();
  set_print_size ();
  hide_output ();
  outfile = fullfile (d, [nm "." typ]);
  if (strcmp (typ, "png"))
    set (groot, "defaulttextfontname", "*");
  endif
  if (strcmp (typ, "eps"))
    d_typ = "-depsc2";
  else
    d_typ = ["-d" typ];
  endif

  if (! (__have_feature__ ("COLAMD")
         && __have_feature__ ("CHOLMOD")
         && __have_feature__ ("UMFPACK")))
    ## There is no sparse matrix implementation available because
    ## of missing libraries, plot sombreros instead.
    sombreroimage (outfile, typ, d_typ);
  elseif (strcmp (nm, "gplot"))
    A = sparse ([2,6,1,3,2,4,3,5,4,6,1,5],
                [1,1,2,2,3,3,4,4,5,5,6,6], 1, 6, 6);
    xy = [0,4,8,6,4,2;5,0,5,7,5,7]';
    gplot (A, xy);
    print (outfile, d_typ);
  elseif (strcmp (nm, "grid"))
    femimages (outfile, d_typ);
  elseif (strcmp (nm, "spmatrix"))
    n = 200;
    a = 10*speye (n) + sparse (1:n,ceil ([1:n]/2),1,n,n) + ...
        sparse (ceil ([1:n]/2),1:n,1,n,n);
    spy (a);
    axis ("ij");
    print (outfile, d_typ);
  elseif (strcmp (nm, "spchol"))
    n = 200;
    a = 10*speye (n) + sparse (1:n,ceil ([1:n]/2),1,n,n) + ...
        sparse (ceil ([1:n]/2),1:n,1,n,n);
    r1 = chol (a);
    spy (r1);
    axis ("ij");
    print (outfile, d_typ);
  elseif (strcmp (nm, "spcholperm"))
    n = 200;
    a = 10*speye (n) + sparse (1:n,ceil ([1:n]/2),1,n,n) + ...
        sparse (ceil ([1:n]/2),1:n,1,n,n);
    [r2,p2,q2] = chol (a);
    spy (r2);
    axis ("ij");
    print (outfile, d_typ);
  else
    error ("unrecognized plot requested");
  endif
  hide_output ();
endfunction

function txtimages (d, nm, n, typ)
  outfile = fullfile (d, [nm "." typ]);
  a = 10*speye (n) + sparse (1:n,ceil([1:n]/2),1,n,n) + ...
      sparse (ceil ([1:n]/2),1:n,1,n,n);
  if (strcmp (nm, "gplot") || strcmp (nm, "grid"))
    fid = fopen (outfile, "wt");
    fputs (fid, "\n");
    fputs (fid, "+---------------------------------+\n");
    fputs (fid, "| Image unavailable in text mode. |\n");
    fputs (fid, "+---------------------------------+\n");
    fclose (fid);
  elseif (strcmp (nm, "spmatrix"))
    printsparse (a, outfile);
  elseif (strcmp (nm, "spchol"))
    r1 = chol (a);
    printsparse (r1, outfile);
  elseif (strcmp (nm, "spcholperm"))
    [r2,p2,q2] = chol (a);
    printsparse (r2, outfile);
  else
    error ("unrecognized plot requested");
  endif
endfunction

function printsparse (a, nm)
  fid = fopen (nm, "wt");
  fputs (fid, "\n");
  for i = 1:rows (a)
    if (rem (i,5) == 0)
      fprintf (fid, "         %2d - ", i);
    else
      fprintf (fid, "            | ");
    endif
    for j = 1:columns (a)
      if (a(i,j) == 0)
        fprintf (fid, "  ");
      else
        fprintf (fid, " *");
      endif
    endfor
    fprintf (fid, "\n");
  endfor
  fprintf (fid, "            |-");
  for j = 1:columns (a)
    if (rem (j,5) == 0)
      fprintf (fid, "-|");
    else
      fprintf (fid, "--");
    endif
  endfor
  fprintf (fid, "\n");
  fprintf (fid, "              ");
  for j = 1:columns (a)
    if (rem (j,5) == 0)
      fprintf (fid, "%2d", j);
    else
      fprintf (fid, "  ");
    endif
  endfor
  fclose (fid);
endfunction

function femimages (outfile, d_typ)

  ## build a rectangle
  node_y = [1;1.2;1.5;1.8;2] * ones (1,11);
  node_x = ones (5,1) * [1,1.05,1.1,1.2,1.3,1.5,1.7,1.8,1.9,1.95,2];
  nodes = [node_x(:), node_y(:)];

  [h,w] = size (node_x);
  elems = [];
  for idx = 1 : w-1
    widx = (idx-1)*h;
    elems = [elems; widx+[(1:h-1);(2:h);h+(1:h-1)]'];
    elems = [elems; widx+[(2:h);h+(2:h);h+(1:h-1)]'];
  endfor

  E = size (elems,1);  # No. of elements
  N = size (nodes,1);  # No. of elements
  D = size (elems,2);  # dimensions+1

  ## Plot FEM Geometry
  elemx = elems(:,[1,2,3,1])';
  xelems = reshape (nodes(elemx, 1), 4, E);
  yelems = reshape (nodes(elemx, 2), 4, E);

  ## Set element conductivity
  conductivity = [1*ones(1,16), 2*ones(1,48), 1*ones(1,16)];

  ## Dirichlet boundary conditions
  D_nodes = [1:5, 51:55];
  D_value = [10*ones(1,5), 20*ones(1,5)];

  ## Neumann boundary conditions
  ## Note that N_value must be normalized by the boundary
  ##   length and element conductivity
  N_nodes = [];
  N_value = [];

  ## Calculate connectivity matrix
  C = sparse ((1:D*E), reshape (elems',D*E,1),1, D*E, N);

  ## Calculate stiffness matrix
  Siidx = floor ([0:D*E-1]'/D)*D*ones(1,D) + ones(D*E,1)*(1:D);
  Sjidx = [1:D*E]'*ones (1,D);
  Sdata = zeros (D*E,D);
  dfact = prod (2:(D-1));
  for j = 1:E
    a = inv ([ ones(D,1), nodes( elems(j,:), : ) ]);
    const = conductivity(j)*2/dfact/abs (det (a));
    Sdata(D*(j-1)+(1:D),:) = const * a(2:D,:)'*a(2:D,:);
  endfor

  ## Element-wise system matrix
  SE = sparse (Siidx,Sjidx,Sdata);
  ## Global system matrix
  S = C'* SE *C;

  ## Set Dirichlet boundary
  V = zeros (N,1);
  V(D_nodes) = D_value;
  idx = 1:N;
  idx(D_nodes) = [];

  ## Set Neumann boundary
  Q = zeros (N,1);
  Q(N_nodes) = N_value; # FIXME

  V(idx) = S(idx,idx) \ ( Q(idx) - S(idx,D_nodes)*V(D_nodes) );

  velems = reshape (V(elemx), 4, E);

  plot3 (xelems, yelems, velems);
  view (80, 10);
  print (outfile, d_typ);
endfunction

## There is no sparse matrix implementation available because of missing
## libraries, plot sombreros instead.  Also plot a nice title that we are
## sorry about that.
function sombreroimage (outfile, typ, d_typ)
  if (strcmp (typ, "txt"))
    fid = fopen (outfile, "wt");
    fputs (fid, "+--------------------------------+\n");
    fputs (fid, "| Image unavailable because of a |\n");
    fputs (fid, "| missing SuiteSparse library.   |\n");
    fputs (fid, "+--------------------------------+\n");
    fclose (fid);
    return;
  else
    [x, y, z] = sombrero ();
    unwind_protect
      mesh (x, y, z);
      title ({"Sorry, graphics are unavailable because Octave was",
              "compiled without the SuiteSparse library."});
    unwind_protect_cleanup
      print (outfile, d_typ);
      hide_output ();
    end_unwind_protect
  endif
endfunction

## This function no longer sets the graphics toolkit; That is now done
## automatically by C++ code which will ordinarily choose 'qt', but might
## choose gnuplot on older systems.  Only a complete lack of plotting is a
## problem.
function set_graphics_toolkit ()
  if (isempty (available_graphics_toolkits ()))
    error ("no graphics toolkit available for plotting");
  elseif (strcmp ("qt", graphics_toolkit ())
          && __have_feature__ ("QT_OFFSCREEN"))
    ## Use qt with QOffscreenSurface for plot
  elseif (! strcmp ("gnuplot", graphics_toolkit ()))
    if (! any (strcmp ("gnuplot", available_graphics_toolkits ())))
      error ("no graphics toolkit available for offscreen plotting");
    else
      graphics_toolkit ("gnuplot");
    endif
  endif
endfunction

function set_print_size ()
  image_size = [5.0, 3.5]; # in inches, 16:9 format
  border = 0;              # For postscript use 50/72
  set (groot, "defaultfigurepapertype", "<custom>");
  set (groot, "defaultfigurepaperorientation", "landscape");
  set (groot, "defaultfigurepapersize", image_size + 2*border);
  set (groot, "defaultfigurepaperposition", [border, border, image_size]);
endfunction

## Use this function before plotting commands and after every call to print
## since print() resets output to stdout (unfortunately, gnuplot can't pop
## output as it can the terminal type).
function hide_output ()
  hf = figure (1, "visible", "off");
endfunction
