## Copyright (C) 2010-2018 Martin Hepperle
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

## -*- texinfo -*-
## @deftypefn  {} {@var{h} =} msgbox (@var{msg})
## @deftypefnx {} {@var{h} =} msgbox (@var{msg}, @var{title})
## @deftypefnx {} {@var{h} =} msgbox (@var{msg}, @var{title}, @var{icon})
## @deftypefnx {} {@var{h} =} msgbox (@var{msg}, @var{title}, "custom", @var{cdata})
## @deftypefnx {} {@var{h} =} msgbox (@var{msg}, @var{title}, "custom", @var{cdata}, @var{colormap})
## @deftypefnx {} {@var{h} =} msgbox (@dots{}, @var{opt})
## Display @var{msg} using a message dialog box.
##
## The message may have multiple lines separated by newline characters ("\n"),
## or it may be a cellstr array with one element for each line.
##
## The optional input @var{title} (character string) can be used to decorate
## the dialog caption.
##
## The optional argument @var{icon} selects a dialog icon.
## It can be one of @qcode{"none"} (default), @qcode{"error"}, @qcode{"help"},
## @qcode{"warn"} or @qcode{"custom"}. The latter must be followed at
## least by an image array  @var{cdata} and eventually its associated
## colormap for an indexed image.
##
## Finally the optional argument @var{opt} controls the behavior of the dialog.
## If @var{opt} is a string, it may be one of 
##
## @table @asis
## @item "modal"
## The dialog is displayed "modal" which means it prevents users from
## interacting with any other GUI element.
## @item "non-modal"
## The dialog is normal.
## @item "replace"
## If dialogs already exists with the same title, the latest is reused
## and others are closed. The resulting dialog is set "non-modal".
## @end table
##
## If @var{opt} is a structure, it must contain fields "WindowStyle" and
## "Interpreter":
##
## @table @asis
## @item "WindowStyle"
## The value must be "modal", "non-modal" or "replace". See above.
## @item "Interpreter"
## Controls the @qcode{"interpreter"} property of the text object used
## for displaying the message. The value must be "none", "tex" or "latex".
## @end table
## 
## The return value @var{h} is a handle to the figure object used for
## building the dialog.
##
## Examples:
##
## @example
## @group
## msgbox ("Some message for the user.");
## msgbox ("Some message\nwith two lines.");
## msgbox (@{"Some message", "with two lines."@});
## msgbox ("Some message for the user.", "Fancy caption");
##
## % A message dialog box with error icon
## msgbox ("Some message for the user.", "Fancy caption", "error");
## @end group
## @end example
##
## @seealso{errordlg, helpdlg, inputdlg, listdlg, questdlg, warndlg}
## @end deftypefn

function retval = msgbox (msg, tit = "", icon = "none", varargin)

  ## Input checks
  nargs = numel (varargin);
  if (nargin < 1)
    print_usage ();
  elseif (! ischar (msg) && ! iscellstr (msg))
    error ("msgbox: MSG must be a string or a cell array of strings");
  elseif (! ischar (tit))
    error ("msgbox: TITLE must be a string or a cell array of strings");
  elseif (! any (strcmp (icon, {"help", "warn", "error", "none", "custom"})))
    error ("msgbox: unhandled value for ICON data");
  elseif (strcmp (icon, "custom"))
    if (nargs < 1)
      error ("msgbox: missing data for 'custom' icon");
    else
      icon = struct ("cdata", varargin{1}, "colormap", []);
      if (! (ismatrix (icon.cdata) || size (icon.cdata, 3) == 3))
        error ("msgbox: unhandled data for 'custom' icon");
      elseif (size (icon.cdata, 3) == 1 && nargs > 1)
        icon.colormap = varargin{2};
        varargin(2) = [];
        nargs--;
      endif
      varargin(1) = [];
      nargs--;
    endif
  endif

  ## Window behavior and text interpreter
  windowstyle = "normal";
  interpreter = "tex";
  if (nargs > 0)
    if (isstruct (varargin{1}))
      if (isfield (varargin{1}, "WindowStyle"))
        windowstyle = varargin{1}.WindowStyle;
      endif
      if (isfield (varargin{1}, "Interpreter"))
        interpreter = varargin{1}.Interpreter;
      endif
    else
      windowstyle = varargin{1};
    endif

    if (! any (strcmp (windowstyle, {"non-modal", "modal", "replace"})))
      error ("msgbox: unhandled value %s for OPT", windowstyle);
    elseif (strcmp (windowstyle, "non-modal"))
      windowstyle = "normal";
    endif
  endif
  
  ## Make a GUI element or print to console
  if (__octave_link_enabled__ ())
    retval = __msgbox__ (msg, tit, icon, windowstyle, interpreter);
  else
    if (iscellstr (msg))
      msg = strjoin (msg, "\n");
    endif
    if (isstruct (icon))
      icon = "custom";
    endif
    disp (sprintf ("\n%s:\t%s\n\t%s\n", upper (icon), tit,
      strrep (msg, "\n", "\n\t")));
    retval = 1;
  endif

endfunction

function hf = __msgbox__ (msg, tit, icon, windowstyle, interpreter)

  ## Prepare graphics objects
  hf = [];
  if (strcmp (windowstyle, "replace"))
    windowstyle = "normal";
    hf = findall (groot, "tag", "__dialog__", "-and", "name", tit);
  endif

  if (! isempty (hf))
    if (numel (hf) > 1)
      close (hf(2:end));
    endif
    hf = hf(1);
    set (hf, "visible", "off", "windowstyle", "normal");
  else
    hf = dialog ("visible", "off", "name", tit, "tag", "__dialog__", ...
                 "windowstyle", "normal");
    if (! strcmp (graphics_toolkit (), "qt"))
      graphics_toolkit (hf, "qt");
    endif
  endif
  
  hp = uipanel (hf);
  
  hax = axes ("parent", hp, "visible", "off", "units", "pixels", ...
              "ydir", "reverse");
  
  ht = text ("parent", hax, "string", msg, "units", "pixels", ...
             "fontsize", 14, "interpreter", interpreter);
  
  ## Hold default icons data in a persistent variable
  persistent cdata = struct ("help", [], "warn", [], "error", []);

  icon_name = "custom";
  if (ischar (icon))
    icon_name = icon;
    icon = struct ("cdata", [], "colormap", []);
    if (isfield (cdata, icon_name))
      if (! isempty (cdata.(icon_name)))
        icon.cdata = cdata.(icon_name);
      else
        if (strcmp (icon_name, "help"))
          icon_name = "information";
        elseif (strcmp (icon_name, "warn"))
          icon_name = "warning";
        endif
        tmp = __octave_link_named_icon__ (["dialog-" icon_name]);
        ## Fake transparency until the opengl renderer handles it:
        ## RGB data from Qt are premultiplied, we only need to add the
        ## background part
        alpha = tmp(:,:,4);
        tmp(:,:,4) = [];
        backgnd = get (hp, "backgroundcolor");
        tmp(:,:,1) += backgnd(1) * (255-alpha);
        tmp(:,:,2) += backgnd(2) * (255-alpha);
        tmp(:,:,3) += backgnd(3) * (255-alpha);
        icon.cdata = tmp;
        cdata.(icon_name) = tmp;
        
      endif
    endif
  endif

  ## Compute bbox in pixels
  ax_sz = [200 60];
  ax_margin = 12;
  txt_margin = 20;
  button_margin = 40;

  extent = get (ht, "extent");
  extent(3) += txt_margin;

  im_sz = size (icon.cdata);
  
  if (! isempty (icon.cdata))
    extent(3) += im_sz(2);
    extent(4) = max (extent(4), im_sz(1));
  endif

  ax_sz = max ([ax_sz; extent(3:4)+2*ax_margin]);
  
  ## Align text left when there is an icon
  text_offset = txt_margin;
  if (! isempty (icon.cdata))
    if (! isempty (icon.colormap))
      set (hax, "colormap", icon.colormap)
    endif
    image ("parent", hax, "cdata", icon.cdata, "cdatamapping", "direct", ...
           "xdata", [1 im_sz(2)], "ydata", [-(im_sz(1))/2+1 im_sz(1)/2]);
    text_offset = im_sz(2) + txt_margin;
  endif

  ## Set objects position
  wd = ax_sz(1);
  hg = ax_sz(2) + button_margin;
  center = get (0, "screensize")(3:4) / 2;
  
  set (hf, "position", [center(1)-wd/2 center(2)-hg/2 wd hg]);
  
  set (hax, "position", [0 button_margin ax_sz], ...
       "xlim", [1 ax_sz(1)]-ax_margin, "ylim", [-ax_sz(2)/2 ax_sz(2)/2], ...
       "units", "normalized");

  set (ht, "units", "data", "position", [text_offset 0 0]);

  hui = uicontrol ("string", "OK", "callback", @() close (gcbf ()), ...
                   "position", [ax_sz(1)/2-40 ax_margin 80 28], "parent", hp);
  
  set (hf, "windowstyle", windowstyle, "visible", "on");

endfunction

%!demo
%! msgbox ("A bare dialog");

%!demo
%! msgbox ("An informative string", "Documentation", "help");

%!demo
%! msgbox ("Something the user should hear about before continuing", ...
%!         "Take care!", "warn");

%!demo
%! msgbox ("A critical message for the user", "Error", "error");

%!demo
%! msgbox ({"The default interpreter is'tex':", ...
%!          "\\Delta_{1-2} = r_2 - r_1"}, "Documentation", "help");

%!demo
%! msgbox ({"Help dialog with uninterpreted string:", ...
%!          "\\Delta_{1-2} = r_2 - r_1"}, "Documentation", "help", ...
%!          struct ("Interpreter", "none", "WindowStyle", "non-modal"));

%!demo
%! msgbox ({"This dialog has replaced all the previously open dialogs", ...
%!          "that had the same title ('Documentation')."}, "Documentation", ...
%!         "warn", "replace");

%!demo
%! msgbox ("Custom dialog with a random 32-by-32-by-3 RGB icon.", ...
%!         "Dialog Title", "custom", rand (32, 32, 3));

%!demo
%! cdata = get (0, "factoryimagecdata"); 
%! msgbox ({"Custom dialog with the default Octave image.", ...
%!          "The image is indexed into the 'copper' colormap"}, ...
%!         "Dialog Title", "custom", cdata, copper (64));

## Test input validation
%!error <msgbox: MSG must be a string or a cell array of strings> msgbox (1)
%!error <msgbox: TITLE must be a string or a cell array of strings>
%! msgbox ("msg", 1)
%!error <msgbox: unhandled value for ICON data> msgbox ("msg", "title", 1)
%!error <msgbox: missing data for 'custom' icon>
%! msgbox ("msg", "title", "custom")
%!error <msgbox: unhandled value 1 for OPT>
%! msgbox ("msg", "title", "help", "1")

