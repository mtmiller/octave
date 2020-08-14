########################################################################
##
## Copyright (C) 2012-2020 The Octave Project Developers
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

function splineimages (d, nm, typ)

  if (strcmp (typ, "txt"))
    image_as_txt (d, nm);
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

  if (strcmp (nm, "splinefit1")) ## Breaks and Pieces
    rand ("state", 1);
    randn ("state", 1);
    x = 2 * pi * rand (1, 200);
    y = sin (x) + sin (2 * x) + 0.2 * randn (size (x));
    ## Uniform breaks
    breaks = linspace (0, 2 * pi, 41); ## 41 breaks, 40 pieces
    pp1 = splinefit (x, y, breaks);
    ## Breaks interpolated from data
    pp2 = splinefit (x, y, 10);  ## 11 breaks, 10 pieces
    ## Plot
    xx = linspace (0, 2 * pi, 400);
    y1 = ppval (pp1, xx);
    y2 = ppval (pp2, xx);
    plot (x, y, ".", xx, [y1; y2]);
    axis tight;
    ylim ([-2.5 2.5]);
    legend ("data", "41 breaks, 40 pieces", "11 breaks, 10 pieces");
    print (outfile, d_typ);
  elseif (strcmp (nm, "splinefit2")) ## Spline orders
    ## Data (200 points)
    rand ("state", 1);
    randn ("state", 1);
    x = 2 * pi * rand (1, 200);
    y = sin (x) + sin (2 * x) + 0.1 * randn (size (x));
    ## Splines
    pp1 = splinefit (x, y, 8, "order", 0);  ## Piecewise constant
    pp2 = splinefit (x, y, 8, "order", 1);  ## Piecewise linear
    pp3 = splinefit (x, y, 8, "order", 2);  ## Piecewise quadratic
    pp4 = splinefit (x, y, 8, "order", 3);  ## Piecewise cubic
    pp5 = splinefit (x, y, 8, "order", 4);  ## Etc.
    ## Plot
    xx = linspace (0, 2 * pi, 400);
    y1 = ppval (pp1, xx);
    y2 = ppval (pp2, xx);
    y3 = ppval (pp3, xx);
    y4 = ppval (pp4, xx);
    y5 = ppval (pp5, xx);
    plot (x, y, ".", xx, [y1; y2; y3; y4; y5]);
    axis tight;
    ylim ([-2.5 2.5]);
    legend ({"data", "order 0", "order 1", "order 2", "order 3", "order 4"});
    print (outfile, d_typ);
  elseif (strcmp (nm, "splinefit3"))
    ## Data (100 points)
    rand ("state", 1);
    randn ("state", 1);
    x = 2 * pi * [0, (rand (1, 98)), 1];
    y = sin (x) - cos (2 * x) + 0.2 * randn (size (x));
    ## No constraints
    pp1 = splinefit (x, y, 10, "order", 5);
    ## Periodic boundaries
    pp2 = splinefit (x, y, 10, "order", 5, "periodic", true);
    ## Plot
    xx = linspace (0, 2 * pi, 400);
    y1 = ppval (pp1, xx);
    y2 = ppval (pp2, xx);
    plot (x, y, ".", xx, [y1; y2]);
    axis tight;
    ylim ([-2 3]);
    legend ({"data", "no constraints", "periodic"});
    print (outfile, d_typ);
  elseif (strcmp (nm, "splinefit4"))
    ## Data (200 points)
    rand ("state", 1);
    randn ("state", 1);
    x = 2 * pi * rand (1, 200);
    y = sin (2 * x) + 0.1 * randn (size (x));
    ## Breaks
    breaks = linspace (0, 2 * pi, 10);
    ## Clamped endpoints, y = y" = 0
    xc = [0, 0, 2*pi, 2*pi];
    cc = [(eye (2)), (eye (2))];
    con = struct ("xc", xc, "cc", cc);
    pp1 = splinefit (x, y, breaks, "constraints", con);
    ## Hinged periodic endpoints, y = 0
    con = struct ("xc", 0);
    pp2 = splinefit (x, y, breaks, "constraints", con, "periodic", true);
    ## Plot
    xx = linspace (0, 2 * pi, 400);
    y1 = ppval (pp1, xx);
    y2 = ppval (pp2, xx);
    plot (x, y, ".", xx, [y1; y2]);
    axis tight;
    ylim ([-1.5 1.5]);
    legend ({"data", "clamped", "hinged periodic"});
    print (outfile, d_typ);
  elseif (strcmp (nm, "splinefit5"))
    ## Truncated data
    x = [0,  1,  2,  4,  8, 16, 24, 40, 56, 72, 80] / 80;
    y = [0, 28, 39, 53, 70, 86, 90, 79, 55, 22,  2] / 1000;
    xy = [x; y];
    ## Curve length parameter
    ds = sqrt (diff (x).^2 + diff (y).^2);
    s = [0, cumsum(ds)];
    ## Constraints at s = 0: (x,y) = (0,0), (dx/ds,dy/ds) = (0,1)
    con = struct ("xc", [0 0], "yc", [0 0; 0 1], "cc", eye (2));
    ## Fit a spline with 4 pieces
    pp = splinefit (s, xy, 4, "constraints", con);
    ## Plot
    ss = linspace (0, s(end), 400);
    xyfit = ppval (pp, ss);
    xyb = ppval (pp, pp.breaks);
    plot (x, y, ".", xyfit(1,:), xyfit(2,:), "r", xyb(1,:), xyb(2,:), "ro");
    legend ({"data", "spline", "breaks"});
    axis tight;
    ylim ([0 0.1]);
    print (outfile, d_typ);
  elseif (strcmp (nm, "splinefit6"))
    ## Data
    randn ("state", 1);
    x = linspace (0, 2*pi, 200);
    y = sin (x) + sin (2 * x) + 0.05 * randn (size (x));
    ## Add outliers
    x = [x, linspace(0,2*pi,60)];
    y = [y, -ones(1,60)];
    ## Fit splines with hinged conditions
    con = struct ("xc", [0, 2*pi]);
    pp1 = splinefit (x, y, 8, "constraints", con, "beta", 0.25); ## Robust fitting
    pp2 = splinefit (x, y, 8, "constraints", con, "beta", 0.75); ## Robust fitting
    pp3 = splinefit (x, y, 8, "constraints", con); ## No robust fitting
    ## Plot
    xx = linspace (0, 2*pi, 400);
    y1 = ppval (pp1, xx);
    y2 = ppval (pp2, xx);
    y3 = ppval (pp3, xx);
    plot (x, y, ".", xx, [y1; y2; y3]);
    legend ({"data with outliers","robust, beta = 0.25", ...
             "robust, beta = 0.75", "no robust fitting"});
    axis tight;
    ylim ([-2 2]);
    print (outfile, d_typ);
  endif
  hide_output ();
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
  ## FIXME: Required until listener for legend exists (bug #39697)
  set (groot, "defaultfigureposition", [ 72*[border, border, image_size] ]);
endfunction

## Use this function before plotting commands and after every call to print
## since print() resets output to stdout (unfortunately, gnuplot can't pop
## output as it can the terminal type).
function hide_output ()
  hf = figure (1, "visible", "off");
endfunction

## generate something for the texinfo @image command to process
function image_as_txt (d, nm)
  fid = fopen (fullfile (d, [nm ".txt"]), "wt");
  fputs (fid, "\n");
  fputs (fid, "+---------------------------------+\n");
  fputs (fid, "| Image unavailable in text mode. |\n");
  fputs (fid, "+---------------------------------+\n");
  fclose (fid);
endfunction


%!demo
%! for s = 1:6
%!   splineimages (sprintf ("splinefit##d", s), "pdf");
%! endfor
