########################################################################
##
## Copyright (C) 1993-2023 The Octave Project Developers
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
## @deftypefn {} {@var{h} =} __plt__ (@var{caller}, @var{hp}, @var{varargin})
## Internal function with common code to implement several 2-D plot types.
## @seealso{plot, polar, loglog, semilogx, semilogy}
## @end deftypefn

function retval = __plt__ (caller, hp, varargin)

  persistent warned_callers = {};
  nargs = nargin - 2;

  if (nargs < 1)
    error ("__plt__: invalid number of arguments");
  endif

  k = 1;

  x_set = false;
  y_set = false;
  property_set = false;
  properties = {};

  ## Find any legend associated with this axes
  try
    hlegend = get (hp, "__legend_handle__");
  catch
    hlegend = [];
  end_try_catch

  setlgnd = false;
  if (isempty (hlegend))
    hlgnd = [];
    tlgnd = {};
  else
    [hlgnd, tlgnd] = __getlegenddata__ (hlegend);
  endif

  ## Gather arguments, decode format, gather plot strings, and plot lines.

  retval = [];

  while (nargs > 0 || x_set)

    if (nargs == 0)
      ## Force the last plot when input variables run out.
      next_cell = {};
      next_arg = {""};
    else
      next_cell = varargin(k);
      next_arg = varargin{k++};
    endif

    if (isnumeric (next_arg) && ndims (next_arg) > 2
        && any (size (next_arg) == 1))
      next_arg = squeeze (next_arg);
      if (! any (strcmp (caller, warned_callers)) && ndims (next_arg) < 3)
        warning (["%s: N-d inputs have been squeezed to less than " ...
                  "three dimensions"], caller);
        warned_callers(end+1) = caller;
      endif
    endif
    if (isnumeric (next_arg) && ndims (next_arg) > 2)
      error ("%s: plot arrays must have less than 2 dimensions", caller);
    endif

    nargs -= 1;

    if (ischar (next_arg) || iscellstr (next_arg))
      if (x_set)
        [options, valid] = __pltopt__ (caller, next_arg, false);
        if (! valid)
          if (nargs == 0)
            error ("%s: properties must appear followed by a value", caller);
          endif
          properties = [properties, [next_cell, varargin(k++)]];
          nargs -= 1;
          continue;
        else
          while (nargs > 0 && ischar (varargin{k}))
            if (nargs < 2)
              error ("%s: properties must appear followed by a value",
                     caller);
            endif
            properties = [properties, varargin(k:k+1)];
            k += 2;
            nargs -= 2;
          endwhile
        endif
        if (y_set)
          htmp = __plt2__ (hp, x, y, options, properties);
          [hlgnd, tlgnd, setlgnd] = ...
            __plt_key__ (htmp, options, hlgnd, tlgnd, setlgnd);
          properties = {};
          retval = [retval; htmp];
        else
          htmp = __plt1__ (hp, x, options, properties);
          [hlgnd, tlgnd, setlgnd] = ...
             __plt_key__ (htmp, options, hlgnd, tlgnd, setlgnd);
          properties = {};
          retval = [retval; htmp];
        endif
        x_set = false;
        y_set = false;
      else
        error ("plot: no data to plot");
      endif
    elseif (x_set)
      if (y_set)
        options = __pltopt__ (caller, {""});
        htmp = __plt2__ (hp, x, y, options, properties);
        [hlgnd, tlgnd, setlgnd] = ...
          __plt_key__ (htmp, options, hlgnd, tlgnd, setlgnd);
        retval = [retval; htmp];
        x = next_arg;
        y_set = false;
        properties = {};
      else
        y = next_arg;
        y_set = true;
      endif
    else
      x = next_arg;
      x_set = true;
    endif

  endwhile

  if (setlgnd)
    legend (gca (), hlgnd, tlgnd);
  endif

endfunction

function [hlgnd, tlgnd, setlgnd] = __plt_key__ (hp, options,
                                                hlgnd, tlgnd, setlgnd)

  n = numel (hp);
  if (numel (options) == 1)
    options = repmat (options(:), n, 1);
  endif

  for i = 1 : n
    key = options(i).key;
    if (! isempty (key))
      hlgnd = [hlgnd(:); hp(i)];
      tlgnd = {tlgnd{:}, key};
      setlgnd = true;
    endif
  endfor

endfunction

function retval = __plt1__ (hp, x1, options, properties = {})

  if (nargin < 3 || isempty (options))
    options = __default_plot_options__ ();
  endif

  if (! isstruct (options))
    error ("__plt1__: options must be a struct array");
  endif

  [nr, nc] = size (x1);
  if (nr == 1)
    x1 = x1.';
    [nr, nc] = deal (nc, nr);   # Swap rows and columns
  endif
  if (iscomplex (x1))
    x1_i = imag (x1);
    if (any ((x1_i(:))))
      x2 = x1_i;
      x1 = real (x1);
    else
      x2 = x1;
      x1 = (1:nr)';
    endif
  else
    x2 = x1;
    x1 = (1:nr)';
  endif

  retval = __plt2__ (hp, x1, x2, options, properties);

endfunction

function retval = __plt2__ (hp, x1, x2, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  if (! isstruct (options))
    error ("__plt2__: options must be a struct array");
  endif

  if (islogical (x1))
    x1 = int8 (x1);
  elseif (iscomplex ((x1)))
    x1 = real (x1);
  endif

  if (islogical (x2))
    x2 = int8 (x2);
  elseif (iscomplex (x2))
    x2 = real (x2);
  endif

  h_set = false;
  if (isempty (x1) && isempty (x2))
    retval = zeros (0, 1);
  elseif (isscalar (x1))
    if (isscalar (x2))
      retval = __plt2ss__ (hp, x1, x2, options, properties);
    elseif (isvector (x2))
      retval = __plt2sv__ (hp, x1, x2, options, properties);
    else
      error ("__plt2__: invalid data for plotting");
    endif
  elseif (isvector (x1))
    if (isscalar (x2))
      retval = __plt2vs__ (hp, x1, x2, options, properties);
    elseif (isvector (x2))
      retval = __plt2vv__ (hp, x1, x2, options, properties);
    elseif (ismatrix (x2))
      retval = __plt2vm__ (hp, x1, x2, options, properties);
    else
      error ("__plt2__: invalid data for plotting");
    endif
  elseif (ismatrix (x1))
    if (isvector (x2))
      retval = __plt2mv__ (hp, x1, x2, options, properties);
    elseif (ismatrix (x2))
      retval = __plt2mm__ (hp, x1, x2, options, properties);
    else
      error ("__plt2__: invalid data for plotting");
    endif
  else
    error ("__plt2__: invalid data for plotting");
  endif

endfunction

function retval = __plt2mm__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  [x_nr, x_nc] = size (x);
  [y_nr, y_nc] = size (y);

  if (x_nr != y_nr && x_nc != y_nc)
    error ("__plt2mm__: matrix dimensions must match");
  endif

  if (numel (options) == 1)
    options = repmat (options(:), x_nc, 1);
  endif
  retval = zeros (x_nc, 1);
  for i = 1:x_nc
    linestyle = options(i).linestyle;
    marker = options(i).marker;
    if (isempty (marker) && isempty (linestyle))
      [linestyle, marker] = __next_line_style__ ();
    endif
    color = options(i).color;
    if (isempty (color))
      color = __next_line_color__ ();
    endif

    retval(i) = __go_line__ (hp, "xdata", x(:,i), "ydata", y(:,i),
                             "color", color, "linestyle", linestyle,
                             "marker", marker, properties{:});
  endfor

endfunction

function retval = __plt2mv__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  y = y(:);
  [y_nr, y_nc] = size (y);
  [x_nr, x_nc] = size (x);

  if (x_nr == y_nr)
    ## Correctly oriented.  Do nothing.
  elseif (x_nc == y_nr)
    x = x.';
    [x_nr, x_nc] = deal (x_nc, x_nr);
  else
    error ("__plt2mv__: matrix dimensions must match");
  endif

  if (numel (options) == 1)
    options = repmat (options(:), x_nc, 1);
  endif
  retval = zeros (x_nc, 1);
  for i = 1:x_nc
    linestyle = options(i).linestyle;
    marker = options(i).marker;
    if (isempty (marker) && isempty (linestyle))
      [linestyle, marker] = __next_line_style__ ();
    endif
    color = options(i).color;
    if (isempty (color))
      color = __next_line_color__ ();
    endif

    retval(i) = __go_line__ (hp, "xdata", x(:,i), "ydata", y,
                             "color", color, "linestyle", linestyle,
                             "marker", marker, properties{:});
  endfor

endfunction

function retval = __plt2ss__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  if (numel (options) > 1)
    options = options(1);
  endif

  linestyle = options.linestyle;
  marker = options.marker;
  if (isempty (marker) && isempty (linestyle))
    ## If unspecified, marker for a single point is always "."
    linestyle = "-";
    marker = ".";
  endif
  color = options.color;
  if (isempty (color))
    color = __next_line_color__ ();
  endif

  retval = __go_line__ (hp, "xdata", x, "ydata", y,
                        "color", color, "linestyle", linestyle,
                        "marker", marker, properties{:});

endfunction

function retval = __plt2sv__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  len = numel (y);
  if (numel (options) == 1)
    options = repmat (options(:), len, 1);
  endif
  retval = zeros (len, 1);
  for i = 1:len
    linestyle = options(i).linestyle;
    marker = options(i).marker;
    if (isempty (marker) && isempty (linestyle))
      ## If unspecified, marker for a point is always "."
      linestyle = "-";
      marker = ".";
    endif
    color = options(i).color;
    if (isempty (color))
      color = __next_line_color__ ();
    endif

    retval(i) = __go_line__ (hp, "xdata", x, "ydata", y(i),
                             "color", color, "linestyle", linestyle,
                             "marker", marker, properties{:});
  endfor

endfunction

function retval = __plt2vm__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  x = x(:);
  [x_nr, x_nc] = size (x);
  [y_nr, y_nc] = size (y);

  if (x_nr == y_nr)
    ## Correctly oriented.  Do nothing.
  elseif (x_nr == y_nc)
    y = y.';
    [y_nr, y_nc] = deal (y_nc, y_nr);
  else
    error ("__plt2vm__: matrix dimensions must match");
  endif

  if (numel (options) == 1)
    options = repmat (options(:), y_nc, 1);
  endif
  retval = zeros (y_nc, 1);
  for i = 1:y_nc
    linestyle = options(i).linestyle;
    marker = options(i).marker;
    if (isempty (marker) && isempty (linestyle))
      [linestyle, marker] = __next_line_style__ ();
    endif
    color = options(i).color;
    if (isempty (color))
      color = __next_line_color__ ();
    endif

    retval(i) = __go_line__ (hp, "xdata", x, "ydata", y(:,i),
                             "color", color, "linestyle", linestyle,
                             "marker", marker, properties{:});
  endfor

endfunction

function retval = __plt2vs__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  len = numel (x);
  if (numel (options) == 1)
    options = repmat (options(:), len, 1);
  endif
  retval = zeros (len, 1);
  for i = 1:len
    linestyle = options(i).linestyle;
    marker = options(i).marker;
    if (isempty (marker) && isempty (linestyle))
      ## If unspecified, marker for a point is always "."
      linestyle = "-";
      marker = ".";
    endif
    color = options(i).color;
    if (isempty (color))
      color = __next_line_color__ ();
    endif

    retval(i) = __go_line__ (hp, "xdata", x(i), "ydata", y,
                             "color", color, "linestyle", linestyle,
                             "marker", marker, properties{:});
  endfor

endfunction

function retval = __plt2vv__ (hp, x, y, options, properties = {})

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  if (numel (options) > 1)
    options = options(1);
  endif

  x = x(:);
  y = y(:);

  if (length (x) != length (y))
    error ("__plt2vv__: vector lengths must match");
  endif

  linestyle = options.linestyle;
  marker = options.marker;
  if (isempty (marker) && isempty (linestyle))
    [linestyle, marker] = __next_line_style__ ();
  endif
  color = options.color;
  if (isempty (color))
    color = __next_line_color__ ();
  endif

  retval = __go_line__ (hp, "xdata", x, "ydata", y,
                        "color", color, "linestyle", linestyle,
                        "marker", marker, properties{:});

endfunction
