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

## -*- texinfo -*-
## @deftypefn  {} {@var{h} =} struct2hdl (@var{s})
## @deftypefnx {} {@var{h} =} struct2hdl (@var{s}, @var{p})
## @deftypefnx {} {@var{h} =} struct2hdl (@var{s}, @var{p}, @var{hilev})
## Construct a graphics handle object @var{h} from the structure @var{s}.
##
## The structure must contain the fields @qcode{"handle"}, @qcode{"type"},
## @qcode{"children"}, @qcode{"properties"}, and @qcode{"special"}.
##
## If the handle of an existing figure or axes is specified, @var{p}, the new
## object will be created as a child of that object.  If no parent handle is
## provided then a new figure and the necessary children will be constructed
## using the default values from the root object.
##
## A third boolean argument @var{hilev} can be passed to specify whether the
## function should preserve listeners/callbacks, e.g., for legends or
## hggroups.  The default is false.
## @seealso{hdl2struct, hgload, findobj}
## @end deftypefn

function [h, pout] = struct2hdl (s, p=[], hilev = false)

  if (nargin < 1)
    print_usage ();
  endif

  fields = {"handle", "type", "children", "properties", "special"};
  partypes = {"root", "figure", "axes", "hggroup"};
  othertypes = {"line", "patch", "scatter", "surface", "image", "text"};
  alltypes = [partypes othertypes];

  if (! isstruct (s))
    print_usage ();
  elseif (! all (isfield (s, fields)))
    print_usage ();
  elseif (isscalar (p))
    if (! ishghandle (p))
      error ("struct2hdl: P is not a handle to a graphic object");
    endif
    if (any (strcmp (get (p).type, partypes)))
      paridx = find (strcmp (get (p).type, alltypes));
      kididx = find (strcmp (s.type, alltypes));
      if (kididx <= paridx)
        error ("struct2hdl: incompatible input handles");
      endif
    else
      error ("struct2hdl: %s object can't be parent object", get (p).type);
    endif
    hpar = p;
    p = [NaN; hpar];
    ## create appropriate parent if needed
    if (any (strcmp (s.type, othertypes)))
      for ii = (paridx+1) : (numel (partypes)-1)
        eval (["hpar = " partypes{ii} '("parent", hpar);']);
        p = [p [NaN; hpar]];
      endfor
    elseif (any (strcmp (s.type, {"hggroup", "axes"})))
      for ii = (paridx+1) : (kididx-1)
        eval (["hpar = " partypes{ii} '("parent", hpar);']);
        p = [p [NaN; hpar]];
      endfor
    else
      par = NaN;
    endif
  elseif (isempty (p))
    if (any (strcmp (s.type, othertypes)))
      par = axes ();
    elseif (any (strcmp (s.type, {"hggroup", "axes"})))
      par = figure ();
    else
      par = NaN;
    endif
    p = [NaN; par];
  endif
  ## read parent (last column) in p and remove it if duplicate
  par = p(2,end);
  tst = find (p(2,:) == par);
  if (numel (tst) > 1)
    p = p(1:2, 1:(tst(end)-1));
  endif

  ## Use lowercase for all properties
  s.properties = cell2struct (struct2cell (s.properties), ...
                              tolower (fieldnames (s.properties)));

  ## Place the "*mode" properties at the end to avoid having the updaters
  ## change the mode to "manual" when the value is "auto".
  names = fieldnames (s.properties);
  n = strncmp (cellfun (@fliplr, names, "uniformoutput", false), "edom", 4);
  n = (n | strcmp (names, "positionconstraint"));
  names = [names(! n); names(n)];
  n_pos = find (strcmp (names, "position") | strcmp (names, "outerposition"));
  if (strcmp (s.type, "axes") && numel (n_pos) == 2)
    if (isfield (s.properties, "positionconstraint"))
      positionconstraint = s.properties.positionconstraint;
    else
      ## loading old figure file before "positionconstraint" property was added
      positionconstraint = s.properties.activepositionproperty;
    endif
    if (strcmp (positionconstraint, "outerposition"))
      names{n_pos(1)} = "position";
      names{n_pos(2)} = "outerposition";
    else
      names{n_pos(1)} = "outerposition";
      names{n_pos(2)} = "position";
    endif
  endif
  ## Reorder the properties with the mode properties coming last
  s.properties = orderfields (s.properties, names);

  ## Silence deprecation warnings
  warning ("off", "Octave:deprecated-property", "local");

  ## Translate field names for Matlab .fig files
  ## FIXME: Is it ok to do this unconditionally?
  if isfield (s.properties, "applicationdata")
    s.properties.__appdata__ = s.properties.applicationdata;
    s.properties = rmfield (s.properties, "applicationdata");
  endif

  ## Create object
  if (strcmp (s.type, "root"))
    h = 0;
    s.properties = rmfield (s.properties, ...
                              {"callbackobject", "commandwindowsize", ...
                               "monitorpositions", "pointerwindow", ...
                               "screendepth", "screenpixelsperinch", ...
                               "screensize"});
  elseif (strcmp (s.type, "figure"))
    [h, s] = createfigure (s);
  elseif (strcmp (s.type, "axes"))
    ## legends and colorbars are "transformed" in to normal axes
    ## if hilev is not requested.
    if (! hilev && isfield (s.properties, "tag"))
      if (strcmp (s.properties.tag, "legend"))
        s.properties.tag = "";
        s.properties.userdata = [];
        par = gcf;
      elseif (strcmp (s.properties.tag, "colorbar"))
        s.properties.tag = "";
        s.properties.userdata = [];
        par = gcf;
      endif
    endif
    ## remove read only properties
    ## FIXME: Remove "interactions", "layout", "legend", "toolbar", "xaxis",
    ## "yaxis", and "zaxis" from this list once they are implemented.
    ro_props = {"interactions", "layout", "legend", "nextseriesindex", ...
                "tightinset", "toolbar", "xaxis", "yaxis", "zaxis"};
    has_ro_props = cellfun (@(x) isfield (s.properties, x), ro_props);
    if (any (has_ro_props))
      s.properties = rmfield (s.properties, ro_props(has_ro_props));
    endif
    [h, s] = createaxes (s, p, par);
  elseif (strcmp (s.type, "line"))
    h = createline (s, par);
  elseif (strcmp (s.type, "patch"))
    [h, s] = createpatch (s, par);
  elseif (strcmp (s.type, "scatter"))
    [h, s] = createscatter (s, par);
  elseif (strcmp (s.type, "text"))
    if (isfield (s.properties, "extent"))
      s.properties = rmfield (s.properties, "extent");
    endif
    h = createtext (s, par);
  elseif (strcmp (s.type, "image"))
    h = createimage (s, par);
  elseif (strcmp (s.type, "surface"))
    h = createsurface (s, par);
  elseif (strcmp (s.type, "light"))
    h = createlight (s, par);
  elseif (strcmp (s.type, "hggroup"))
    [h, s, p] = createhg (s, p, par, hilev);
  elseif (any (strcmp (s.type, {"uimenu", "uicontextmenu",...
                                "uicontrol", "uipanel", "uibuttongroup",...
                                "uitoolbar", "uipushtool", "uitoggletool"...
                                "uitable"})))
    if (isfield (s.properties, "extent"))
      s.properties = rmfield (s.properties, "extent");
    endif
    [h, s] = createui (s, par);
    if (strcmp (s.type, "uicontextmenu"))
      set (p(2, ismember (p(1,:), s.special)), 'UIContextMenu', h);
    endif
  else
    error ("struct2hdl: %s objects are not implemented yet", s.type);
  endif

  ## children
  p = [p [s.handle; h]];  # [original; new]
  kids = s.children;
  nkids = length (kids);
  ii = 0;
  while (nkids)
    ii += 1;
    if (! any (ii == s.special))
      [h2, p] = struct2hdl (s.children(ii), [p [s.handle; h]], hilev);
    endif
    nkids -= 1;
  endwhile

  ## paste properties
  setprops (s, h, p, hilev);

  pout = p;

endfunction

function [h, sout] = createfigure (s)
  ## Create figure initially invisible to speed up loading.
  opts = {"visible", "off"};
  if (isfield (s.properties, "integerhandle"))  # see also bug #53342.
    opts = [opts {"integerhandle", s.properties.integerhandle}];
    s.properties = rmfield (s.properties, "integerhandle");
  endif
  h = figure (opts{:});
  rmprops = {"currentaxes", "currentcharacter", "currentobject", ...
             "currentpoint", "number"};
  rmprops (! isfield (s.properties, rmprops)) = [];
  s.properties = rmfield (s.properties, rmprops);
  if (! isfield (s.properties, "visible"))
    s.properties.visible = "on";
  endif
  addmissingprops (h, s.properties);
  sout = s;
endfunction

function [h, sout] = createaxes (s, p, par)

  if (! isfield (s.properties, "tag")
      || ! any (strcmpi (s.properties.tag, {"colorbar", "legend"})))
    ## regular axes
    propval = {"position", s.properties.position};
    hid = {"__autopos_tag__", "looseinset"};
    for ii = 1:numel (hid)
      prop = hid{ii};
      if (isfield (s.properties, prop))
        val = s.properties.(prop);
        propval = [propval, prop, val];
      endif
    endfor
    h = axes (propval{:}, "parent", par);

    if (isfield (s.properties, "__plotyy_axes__"))
      plty = s.properties.__plotyy_axes__;
      addproperty ("__plotyy_axes__", h, "data");
      tmp = [p [s.handle; h]];
      tst = ismember (tmp(1:2:end), plty);
      if (sum (tst) == numel (plty))
        for ii = 1:numel (plty)
          plty(ii) = tmp(find (tmp == plty(ii)) + 1);
        endfor
        for ii = 1:numel (plty)
          set (plty(ii), "__plotyy_axes__", plty);
        endfor
      endif
      s.properties = rmfield (s.properties, "__plotyy_axes__");
    endif

    ## delete non-default and already set properties
    fields = fieldnames (s.properties);
    tst = cellfun (@(x) isprop (h, x), fields);
    s.properties = rmfield (s.properties, fields(find (tst == 0)));

  elseif (strcmp (s.properties.tag, "legend"))
    ## legends
    oldax = s.properties.userdata.handle;
    idx = find (p == oldax);
    newax = p(idx+1);
    strings = {};
    kids = s.children;
    kids(s.special) = [];
    oldh = unique (arrayfun (@(x) x.properties.userdata(end), kids));
    for ii = 1:length (oldh)
      idx = find (p(1:2:end) == oldh(ii)) * 2;
      if (! isempty (idx))
        newh(ii) = p(idx);
        if (! strcmp (get (newh(ii), "type"), "hggroup"))
          str = get (newh(ii), "displayname");
          strings = [strings str];
        else
          str = get (get (newh(ii), "children")(1), "displayname");
          strings = [strings str];
        endif
      else
        error ("struct2hdl: didn't find a legend item");
      endif
    endfor
    location = s.properties.location;
    orientation = s.properties.orientation;
    textpos = s.properties.textposition;
    box = s.properties.box;

    h = legend (newax, newh, strings, "location", location, ...
                "orientation", orientation);
    set (h, "textposition", textpos); # bug makes "textposition"
                                      # redefine the legend
    h = legend (newax, newh, strings, "location", location, ...
                "orientation", orientation);
    ## box
    if (strcmp (box, "on"))
      legend ("boxon");
    endif

    ## visibility
    tst = arrayfun (@(x) strcmp (x.properties.visible, "on"), kids);
    if (! any (tst))
      legend ("hide");
    endif

    ## remove all properties such as "textposition" that redefine
    ## the entire legend.  Also remove chidren.
    s.properties = rmfield (s.properties, ...
                              {"__appdata__", "xlabel",...
                               "ylabel", "zlabel", "location", ...
                               "title", "string","orientation", ...
                               "visible", "textposition"});

    s.children = [];

  elseif (strcmp (s.properties.tag, "colorbar"))
    ## colorbar
    oldax = s.properties.axes;
    if (! isempty (idx = find (oldax == p)))
      ax = p(idx+1);
      location = s.properties.location;
      h = colorbar ("peer", ax, location);
      s.properties = rmfield (s.properties, ...
                                {"__appdata__", "xlabel" ...
                                 "ylabel", "zlabel", ...
                                 "title", "axes"});
      s.children= [];
    else
      error ("hdl2struct: didn't find an object");
    endif
  endif

  sout = s;

endfunction

function h = createline (s, par)
  h = line ("parent", par);
  addmissingprops (h, s.properties);
endfunction

function [h, sout] = createpatch (s, par)

  prp.faces = s.properties.faces;
  prp.vertices = s.properties.vertices;
  prp.facevertexcdata = s.properties.facevertexcdata;
  h = patch (prp);
  set (h, "parent", par);
  s.properties = rmfield (s.properties,
                          {"faces", "vertices", "facevertexcdata"});
  ## Also remove derived properties.  Otherwise there is a possibility for
  ## a segfault when 'set (h, properties)' is used to restore properties
  ## which do not match in size the ones created with from the call to patch().
  s.properties = rmfield (s.properties, {"xdata", "ydata", "zdata", "cdata"});
  addmissingprops (h, s.properties);
  sout = s;

endfunction

function [h, sout] = createscatter (s, par)

  if (isempty (s.properties.zdata))
    ## 2D scatter
    h = scatter (s.properties.xdata, s.properties.ydata);
  else
    ## 3D scatter
    h = scatter3 (s.properties.xdata, s.properties.ydata, s.properties.zdata);
  endif

  set (h, "parent", par);
  s.properties = rmfield (s.properties,
                          {"xdata", "ydata", "zdata"});
  addmissingprops (h, s.properties);
  sout = s;

endfunction

function h = createtext (s, par)
  h = text ("parent", par);
  addmissingprops (h, s.properties);
endfunction

function h = createimage (s, par)
  h = image (1, "parent", par);
  addmissingprops (h, s.properties);
endfunction

function h = createsurface (s, par)
  h = surface ("parent", par);
  addmissingprops (h, s.properties);
endfunction

function h = createlight (s, par)
  h = light ("parent", par);
  addmissingprops (h, s.properties);
endfunction

function [h, s] = createui (s, par)
  if (isfield (s.properties, "style") && strcmp (s.properties.style, "frame"))
    s.type = "uipanel";  # frame is deprecated: use uipanel instead
  endif
  h = feval (s.type, "parent", par);
  addmissingprops (h, s.properties);
endfunction

function [h, sout, pout] = createhg (s, p, par, hilev)
  ## Here we infer from properties the type of hggroup we should build
  ## an call corresponding high level functions
  ## We manually set "hold on" to avoid next hggroup be deleted
  ## the proper value of axes "nextplot" will finally be recovered

  hold on;
  if (hilev)
    [h, s, p] = createhg_hilev (s, p, par);
    if (numel (s.children) != numel (get (h).children))
      warning (["struct2hdl: could not infer the hggroup type. ", ...
                "Will build objects but listener/callback functions ", ...
                "will be lost"]);
      if (isfield (h, "bargroup"))
        delete (get (h).bargroup);
      else
        delete (h);
      endif
      h = hggroup ("parent", par);
      addmissingprops (h, s.properties);
      s.special = [];
    else
      oldkids = s.children;
      newkids = get (h).children;
      nkids = numel (oldkids);
      ii = 1;
      while (nkids)
        p = [p [oldkids(ii++).handle; newkids(nkids--)]];
      endwhile
    endif
  else
    h = hggroup ("parent", par);
    addmissingprops (h, s.properties);
    s.special = [];
  endif

  sout = s;
  pout = p;

endfunction

function [h, sout, pout] = createhg_hilev (s, p, par)

  fields = s.properties;
  if (isfield (fields, "contourmatrix"))
    ## contours
    xdata = s.properties.xdata;
    ydata = s.properties.ydata;
    zdata = s.properties.zdata;
    levellist = s.properties.levellist;
    textlist = s.properties.textlist;

    ## contour creation
    if (isempty (s.children(1).properties.zdata))
      if (strcmpi (s.properties.fill, "on"))
        [cm2, h] = contourf (xdata, ydata, zdata, levellist);
      else
        [cm2, h] = contour (xdata, ydata, zdata, levellist);
      endif

      ## labels
      if (strcmpi (s.properties.showtext, "on"))
        clabel (cm2, h, textlist);
      endif
    else
      [cm2, h] = contour3 (xdata, ydata, zdata, levellist);
    endif

    ## delete already set properties and children
    s.properties = rmfield (s.properties, ...
                              {"xdata", "ydata", "zdata", ...
                               "contourmatrix", "levellist", ...
                               "fill", "labelspacing", ...
                               "levellistmode", "levelstep", ...
                               "levelstepmode", "textlist"...
                               "textlistmode" , "textstep", ...
                               "textstepmode", "zlevel", ...
                               "zlevelmode"});

  elseif (isfield (fields, "udata") && isfield (fields, "vdata"))
    ## quiver
    xdata = s.properties.xdata;
    ydata = s.properties.ydata;

    udata = s.properties.udata;
    vdata = s.properties.vdata;

    h = quiver (xdata, ydata, udata, vdata);

    ## delete already set properties and children
    s.properties = rmfield (s.properties, ...
                              {"xdata", "ydata", "zdata", ...
                               "xdatasource", "ydatasource", "zdatasource", ...
                               "udata", "vdata", "wdata", ...
                               "udatasource", "vdatasource", "wdatasource"});

  elseif (isfield (fields, "format"))
    ##errorbar
    form = s.properties.format;
    xdata = s.properties.xdata;
    ydata = s.properties.ydata;
    xldata = s.properties.xldata;
    ldata = s.properties.ldata;
    xudata = s.properties.xudata;
    udata = s.properties.udata;

    switch (form)
      case "xerr"
        h = errorbar (xdata, ydata, xldata, xudata, ">");
      case "yerr"
        h = errorbar (xdata, ydata, ldata, udata, "~");
      case "xyerr"
        h = errorbar (xdata, ydata, xldata, xudata, ldata, udata, "~>");
      case "box"
        h = errorbar (xdata, ydata, xldata, xudata, "#");
      case "boxy"
        h = errorbar (xdata, ydata, ldata, udata, "#~");
      case "boxxy"
        h = errorbar (xdata, ydata, xldata, xudata, ldata, udata, "#~>");
      otherwise
        error ("struct2hdl: couldn't guess the errorbar format");
    endswitch
    ## delete already set properties
    s.properties = rmfield (s.properties, ...
                              {"xdata", "ydata", ...
                               "xldata", "ldata", ...
                               "xudata", "udata", ...
                               "xldatasource", "ldatasource", ...
                               "xudatasource", "udatasource", ...
                               "format"});

  elseif (isfield (fields, "bargroup"))
    ## bar plot
    ## FIXME: Here we don't have access to brothers so we first create all
    ## the barseries of the bargroup (but the last), then retrieve information,
    ## and rebuild the whole bargroup.
    ## The duplicate are deleted after calling "setprops"

    bargroup = s.properties.bargroup;
    oldh = s.handle;

    temp = ismember ([p(1:2:end) oldh], bargroup);

    tst = sum (temp) == length (bargroup);

    if (isscalar (bargroup) || ! tst)
      xdata = s.properties.xdata;
      ydata = s.properties.ydata;

      h = bar (xdata, ydata);

      ## delete already set properties,
      s.properties = rmfield (s.properties, ...
                                {"xdata", "ydata", ...
                                 "xdatasource", "ydatasource", ...
                                 "bargroup", ...
                                 "barwidth", "baseline"});
    else
      xdata = [];
      ydata = [];

      ##build x/y matrix
      nbar = length (bargroup);
      tmp = struct ("handle", NaN, "type", "", "children", [], "special", []);
      for ii = 1:(nbar - 1)
        idx = find (p(1:2:end) == bargroup(ii)) * 2;
        hdl = p(idx);
        xdata = [xdata get(hdl).xdata];
        ydata = [ydata get(hdl).ydata];
        tmp.children(ii) = hdl2struct (hdl);
      endfor

      xdata = [xdata s.properties.xdata];
      ydata = [ydata s.properties.ydata];
      width = s.properties.barwidth;
      h = bar (ydata, width);

      ## replace previous handles in "match", copy props and delete redundant
      for ii = 1:(nbar - 1)
        props = tmp.children(ii).properties;
        bl = props.baseline;
        tmp.children(ii).properties = rmfield (props, {"baseline", "bargroup"});
        setprops (tmp.children(ii), h(ii), p, 1);
        delete (tmp.children(ii).handle);
        delete (bl);
        idxpar = find (p == tmp.children(ii).handle);
        p(idxpar) = h(ii);
        idxkid = idxpar - 2;
        p(idxkid) = get (h(ii), "children");
      endfor
      p(2,((end-nbar+2):end)) = h(1:(end-1));
      h = h(end);

      ## delete already set properties ,
      s.properties = rmfield (s.properties, ...
                                {"xdata", "ydata", "bargroup"...
                                 "barwidth", "baseline"});
    endif
  elseif (isfield (fields, "baseline"))
    ## stem plot
    xdata = s.properties.xdata;
    ydata = s.properties.ydata;

    h = stem (xdata, ydata);

    ## delete already set properties,
    s.properties = rmfield (s.properties, ...
                              {"xdata", "ydata", ...
                               "xdatasource", "ydatasource", ...
                               "baseline"});
  elseif (isfield (fields, "basevalue"))
    ## area plot
    xdata = s.properties.xdata;
    ydata = s.properties.ydata;
    level = s.properties.basevalue;

    h = area (xdata, ydata, level);

    ## delete already set properties,
    s.properties = rmfield (s.properties, ...
                              {"xdata", "ydata", ...
                               "xdatasource", "ydatasource"});
  else
    warning ("struct2hdl: could not infer the hggroup type.  Will build objects but listener/callback functions will be lost");
    h = hggroup ("parent", par);
    addmissingprops (h, s.properties);
    s.special = [];           # children will be treated as normal children
  endif

  sout = s;
  pout = p;

endfunction

function setprops (s, h, p, hilev)

  isspecial = (isfield (s.properties, "tag")
               && any (strcmpi (s.properties.tag, {"colorbar", "legend"})));
  if (! isspecial)
    try
      specs = s.children(s.special);
    catch
      specs = [];
    end_try_catch
    if (isempty (specs))
      hdls = [];
    else
      hdls = [specs.handle];
    endif
    nh = length (hdls);
    msg = "";
    if (! nh)
      set (h, s.properties);
    else
      ## Specials are objects that where automatically constructed with
      ## current object.  Among them are "x(yz)labels", "title", and
      ## high level hggroup children
      fields = fieldnames (s.properties);
      vals = struct2cell (s.properties);
      idx = find (cellfun (@(x) valcomp(x, hdls) , vals));
      s.properties = rmfield (s.properties, fields(idx));

      ## set all properties but special handles
      set (h, s.properties);

      ## find props with val == (one of special handles)
      nf = length (idx);
      fields = fields(idx);
      vals = vals(idx);
      while (nf)
        field = fields{nf};
        idx = find (hdls == vals{nf});
        spec = specs(idx);
        ## FIXME: Wouldn't it be better to call struct2hdl recursively
        ##        for this handle?  That way the function could determine
        ##        based on type what special actions to take.
        try
          spec.properties = rmfield (spec.properties, "extent");
        end_try_catch
        if (isprop (h, field))
          h2 = get (h, field);
          addmissingprops (h2, spec.properties);
          set (h2, spec.properties);
        endif
        nf -= 1;
      endwhile

      ## If hggroup children were created by high level functions,
      ## copy only useful properties.
      if (hilev)
        if (strcmp (s.type, "hggroup"))
          nold = numel (s.children);
          nnew = numel (get (h).children);

          if (nold == nnew)
            hnew = get (h).children;
            ii = 1;
            while (ii <= nnew)
              try
                set (hnew (ii), "displayname", ...
                     s.children(ii).properties.displayname);
              catch
                sprintf ("struct2hdl: couldn't set hggroup children #%d props.", ii);
              end_try_catch
              ii += 1;
            endwhile

          else
            error ("struct2hdl: non-conformant number of children in hgggroup");
          endif
        endif
      endif
    endif

  else
    set (h, s.properties);
  endif

endfunction

function out = valcomp (x, hdls)

  if (isfloat (x) && isscalar (x))
    out = any (x == hdls);
  else
    out = 0;
  endif

endfunction

function addmissingprops (h, props)

  hid = {"__autopos_tag__", "looseinset"};
  oldfields = fieldnames (props);
  for ii = 1:numel (oldfields)
    prop = oldfields{ii};
    if (! isprop (h, prop) && ! any (strcmp (prop, hid)))
      addproperty (prop, h, "any");
    endif
  endfor

endfunction


## FIXME: Need validation tests
