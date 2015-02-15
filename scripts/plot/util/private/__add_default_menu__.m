## Copyright (C) 2010-2015 Kai Habel
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} __add_default_menu__ (@var{fig})
## Add default menu to figure.  All uimenu handles have
## their @qcode{"HandleVisibility"} property set to @qcode{"off"}.
## @end deftypefn

## Author: Kai Habel

function __add_default_menu__ (fig)

  ## Only FLTK toolkit currently provides menubar
  if (! strcmp (get (fig, "__graphics_toolkit__"), "fltk"))
    return;
  endif

  obj = findall (fig, "-depth", 1, "tag", "__default_menu__", "label", "&File");
  if (isempty (obj))
    __f = uimenu (fig, "label", "&File", "handlevisibility", "off",
                       "tag", "__default_menu__");
      uimenu (__f, "label", "&Save", "callback", @save_cb);
      uimenu (__f, "label", "Save &As", "callback", @save_cb);
      uimenu (__f, "label", "&Close", "callback", "close (gcf)");

    __e = uimenu (fig, "label", "&Edit", "handlevisibility", "off",
                       "tag", "__default_menu__");
      uimenu (__e, "label", "Toggle &grid on all axes", "tag", "toggle", "callback", @grid_cb);
      uimenu (__e, "label", "Show grid on all axes", "tag", "on", "callback", @grid_cb);
      uimenu (__e, "label", "Hide grid on all axes", "tag", "off", "callback", @grid_cb);
      uimenu (__e, "label", "Auto&scale all axes", "callback", @autoscale_cb);
      gm = uimenu (__e, "label", "GUI &Mode (on all axes)");
        uimenu (gm, "label", "Pan x and y", "tag", "pan_on", "callback", @guimode_cb);
        uimenu (gm, "label", "Pan x only", "tag", "pan_xon", "callback", @guimode_cb);
        uimenu (gm, "label", "Pan y only", "tag", "pan_yon", "callback", @guimode_cb);
        uimenu (gm, "label", "Disable pan and rotate", "tag", "no_pan_rotate", "callback", @guimode_cb);
        uimenu (gm, "label", "Rotate on", "tag", "rotate3d", "callback", @guimode_cb);
        uimenu (gm, "label", "Enable mousezoom", "tag", "zoom_on", "callback", @guimode_cb);
        uimenu (gm, "label", "Disable mousezoom", "tag", "zoom_off", "callback", @guimode_cb);

  endif

endfunction

function save_cb (h, e)
  lbl = get (gcbo, "label");
  if (strcmp (lbl, "&Save"))
    fname = get (gcbo, "userdata");
    if (isempty (fname))
      __save_as__ (gcbo);
    else
      saveas (gcbo, fname);
    endif
  elseif (strcmp (lbl, "Save &As"))
    __save_as__ (gcbo);
  endif
endfunction

function __save_as__ (caller)
  [filename, filedir] = uiputfile ({"*.pdf;*.ps;*.gif;*.png;*.jpg",
                                    "Supported Graphic Formats"},
                                   "Save Figure",
                                   pwd);
  if (filename != 0)
    fname = [filedir filesep() filename];
    obj = findall (gcbf, "label", "&Save");
    if (! isempty (obj))
      set (obj(1), "userdata", fname);
    endif
    saveas (caller, fname);
  endif
endfunction


function hax = __get_axes__ (h)
  ## Get parent figure
  fig = ancestor (h, "figure");

  ## Find all axes which aren't legends
  hax = findobj (fig, "type", "axes", "-not", "tag", "legend");
endfunction

function grid_cb (h, e)
  hax = __get_axes__ (h);
  id = get (h, "tag");
  switch (id)
    case "toggle"
      arrayfun (@grid, hax);
    otherwise
      arrayfun (@(h) grid(h, id), hax);
  endswitch
  drawnow ();
endfunction

function autoscale_cb (h, e)
  hax = __get_axes__ (h);
  arrayfun (@(h) axis (h, "auto"), hax)
  drawnow ();
endfunction

function guimode_cb (h, e)
  hax = __get_axes__ (h);
  id = get (h, "tag");
  switch (id)
    case "pan_on"
      arrayfun (@(h) pan (h, "on"), hax)
    case "pan_xon"
      arrayfun (@(h) pan (h, "xon"), hax)
    case "pan_yon"
      arrayfun (@(h) pan (h, "yon"), hax)
    case "rotate3d"
      arrayfun (@(h) rotate3d (h, "on"), hax)
    case "no_pan_rotate"
      arrayfun (@(h) pan (h, "off"), hax)
      arrayfun (@(h) rotate3d (h, "off"), hax)
    case "zoom_on"
      arrayfun (@(h) set (h, "mouse_wheel_zoom", 0.05), hax);
    case "zoom_off"
      arrayfun (@(h) set (h, "mouse_wheel_zoom", 0.0), hax);
  endswitch
endfunction
