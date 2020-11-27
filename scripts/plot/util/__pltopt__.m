########################################################################
##
## Copyright (C) 1994-2020 The Octave Project Developers
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
## @deftypefn {} {} __pltopt__ (@var{caller}, @var{opt}, @var{err_on_invalid})
##
## Decode plot option strings.
##
## @var{opt} can currently be some combination of the following:
##
## @table @asis
## @item @qcode{"-"}
## For solid linestyle (default).
##
## @item @qcode{"--"}
## For dashed line style.
##
## @item @qcode{"-."}
## For linespoints plot style.
##
## @item @qcode{":"}
## For dots plot style.
##
## @item @qcode{"r"}
## Red line color.
##
## @item @qcode{"g"}
## Green line color.
##
## @item @qcode{"b"}
## Blue line color.
##
## @item @qcode{"c"}
## Cyan line color.
##
## @item @qcode{"m"}
## Magenta line color.
##
## @item @qcode{"y"}
## Yellow line color.
##
## @item @qcode{"k"}
## Black line color.
##
## @item @qcode{"w"}
## White line color.
##
## @item @qcode{";title;"}
## Here @code{"title"} is the label for the key.
##
## @item  @qcode{"+"}
## @itemx @qcode{"o"}
## @itemx @qcode{"*"}
## @itemx @qcode{"."}
## @itemx @qcode{"x"}
## @itemx @qcode{"s"}
## @itemx @qcode{"d"}
## @itemx @qcode{"^"}
## @itemx @qcode{"v"}
## @itemx @qcode{">"}
## @itemx @qcode{"<"}
## @itemx @qcode{"p"}
## @itemx @qcode{"h"}
## Used in combination with the points or linespoints styles, set the point
## style.
## @end table
##
## The legend may be fixed to include the name of the variable
## plotted in some future version of Octave.
## @end deftypefn

function [options, valid] = __pltopt__ (caller, opt, err_on_invalid = true)

  if (ischar (opt))
    opt = cellstr (opt);
  elseif (! iscellstr (opt))
    ## FIXME: This is an internal function.  Can't we rely on valid input?
    error ("__pltopt__: argument must be a character string or cell array of character strings");
  endif

  nel = numel (opt);

  if (nel) 
    for i = nel:-1:1
      [options(i), valid] = decode_linespec (caller, opt{i}, err_on_invalid);
      if (! err_on_invalid && ! valid)
        return;
      endif
    endfor
  else
    options = __default_plot_options__ ();
    valid = true;
  endif

endfunction

## Really decode plot option strings.
function [options, valid] = decode_linespec (caller, opt, err_on_invalid)

  persistent default_options = __default_plot_options__ ();

  options = default_options;
  valid = true;

  have_linestyle = false;
  have_marker = false;

  ## If called by __errplot__, extract the linestyle before proceeding.
  if (strcmp (caller, "__do_errplot__"))
    if (strncmp (opt, "#~>", 3))
      n = 3;
    elseif (strncmp (opt, "#~", 2) || strncmp (opt, "~>", 2))
      n = 2;
    elseif (strncmp (opt, "~", 1) || strncmp (opt, ">", 1)
            || strncmp (opt, "#", 1))
      n = 1;
    else
      n = 0;
    endif
    options.errorstyle = opt(1:n);
    opt(1:n) = [];
  else
    options.errorstyle = "~";
  endif

  while (! isempty (opt))
    topt = opt(1);
    n = 1;

    ## LineStyles
    if (strncmp (opt, "--", 2) || strncmp (opt, "-.", 2))
      options.linestyle = opt(1:2);
      have_linestyle = true;
      n = 2;
    elseif (topt == "-" || topt == ":")
      have_linestyle = true;
      options.linestyle = topt;
    ## Markers
    elseif (any (topt == "+o*.xsd^v><ph"))
      have_marker = true;
      ## Check for long form marker styles
      if (any (topt == "sdhp"))
        if (strncmp (opt, "square", 6))
          n = 6;
        elseif (strncmp (opt, "diamond", 7))
          n = 7;
        elseif (strncmp (opt, "hexagram", 8))
          n = 8;
        elseif (strncmp (opt, "pentagram", 9))
          n = 9;
        endif
      endif
      options.marker = topt;
    ## Color specs
    elseif (topt == "k")
      options.color = [0, 0, 0];
    elseif (topt == "r")
      if (strncmp (opt, "red", 3))
        n = 3;
      endif
      options.color = [1, 0, 0];
    elseif (topt == "g")
      if (strncmp (opt, "green", 5))
        n = 5;
      endif
      options.color = [0, 1, 0];
    elseif (topt == "b")
      if (strncmp (opt, "black", 5))
        options.color = [0, 0, 0];
        n = 5;
      elseif (strncmp (opt, "blue", 4))
        options.color = [0, 0, 1];
        n = 4;
      else
        options.color = [0, 0, 1];
      endif
    elseif (topt == "y")
      if (strncmp (opt, "yellow", 6))
        n = 6;
      endif
      options.color = [1, 1, 0];
    elseif (topt == "m")
      if (strncmp (opt, "magenta", 7))
        n = 7;
      endif
      options.color = [1, 0, 1];
    elseif (topt == "c")
      if (strncmp (opt, "cyan", 4))
        n = 4;
      endif
      options.color = [0, 1, 1];
    elseif (topt == "w")
      if (strncmp (opt, "white", 5))
        n = 5;
      endif
      options.color = [1, 1, 1];
    elseif (isspace (topt))
      ## Do nothing.
    elseif (topt == ";")
      t = index (opt(2:end), ";");
      if (t)
        options.key = opt(2:t);
        n = t+1;
      else
        if (err_on_invalid)
          error ("%s: unfinished key label", caller);
        else
          valid = false;
          options = default_options;
          return;
        endif
      endif
    else
      if (err_on_invalid)
        error ("%s: unrecognized format character: '%s'", caller, topt);
      else
        valid = false;
        options = default_options;
        return;
      endif
    endif

    opt(1:n) = [];  # Delete decoded portion
  endwhile

  if (! have_linestyle && have_marker)
    options.linestyle = "none";
  endif

  if (have_linestyle && ! have_marker)
    options.marker = "none";
  endif

endfunction


## Only cursory testing.  Real testing done by appearance of plots.
%!test
%! opts = __pltopt__ ("abc", "");
%! assert (opts.color, []);
%! assert (opts.linestyle, []);
%! assert (opts.marker, []);
%! assert (opts.key, "");
%!test
%! opts = __pltopt__ ("abc", "r:x");
%! assert (opts.color, [1 0 0]);
%! assert (opts.linestyle, ":");
%! assert (opts.marker, "x");
%!test
%! opts = __pltopt__ ("abc", "-.blackx");
%! assert (opts.color, [0 0 0]);
%! assert (opts.linestyle, "-.");
%! assert (opts.marker, "x");
%!test
%! opts = __pltopt__ ("abc", "gsquare");
%! assert (opts.color, [0 1 0]);
%! assert (opts.linestyle, "none");
%! assert (opts.marker, "s");
%!test
%! opts = __pltopt__ ("abc", ";Title;");
%! assert (opts.key, "Title");
%! assert (opts.color, []);
%! assert (opts.linestyle, []);
%! assert (opts.marker, []);
%!test
%! opts = __pltopt__ ("__do_errplot__", "~>r");
%! assert (opts.errorstyle, "~>");
%! assert (opts.color, [1 0 0 ]);
%! assert (opts.linestyle, []);
%! assert (opts.marker, []);

## Test input validation
%!error <argument must be a character string or cell array> __pltopt__ ("abc", 1)
%!error <unfinished key label> __pltopt__ ("abc", "rx;my_title", true)
%!error <unrecognized format character: 'u'> __pltopt__ ("abc", "u", true)
