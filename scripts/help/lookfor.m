## Copyright (C) 2009 S�ren Hauberg
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Command} lookfor @var{str}
## @deftypefnx {Command} lookfor -all @var{str}
## @deftypefnx {Function} {[@var{fun}, @var{helpstring}] = } lookfor (@var{str})
## @deftypefnx {Function} {[@var{fun}, @var{helpstring}] = } lookfor ('-all', @var{str})
## Search for the string @var{str} in all of the functions found in the
## function search path.  By default @code{lookfor} searches for @var{str}
## in the first sentence of the help string of each function found. The entire
## help string of each function found in the path can be searched if
## the '-all' argument is supplied. All searches are case insensitive.
## 
## Called with no output arguments, @code{lookfor} prints the list of matching
## functions to the terminal. Otherwise the output arguments @var{fun} and
## @var{helpstring} define the matching functions and the first sentence of
## each of their help strings.
## 
## Note that the ability of @code{lookfor} to correctly identify the first
## sentence of the help of the functions is dependent on the format of the
## functions help. All of the functions in Octave itself will correctly
## find the first sentence, but the same can not be guaranteed for other
## functions. Therefore the use of the '-all' argument might be necessary
## to find related functions that are not part of Octave.
## @seealso{help, which}
## @end deftypefn

## PKG_ADD: mark_as_command lookfor

function [out_fun, out_help_text] = lookfor (str, extra)
  if (strcmpi (str, "-all"))
    ## The difference between using '-all' and not, is which part of the caches
    ## we search. The cache is organised such that its first column contains
    ## the function name, its second column contains the full help text, and its
    ## third column contains the first sentence of the help text.
    str = extra;
    search_type = 2; # when using caches, search its second column
  else
    search_type = 3; # when using caches, search its third column
  endif
  str = lower (str);

  ## Search functions, operators, and keywords that come with Octave
  cache_file = fullfile (octave_config_info.datadir, "etc", "DOC");
  if (exist (cache_file, "file"))
    [fun, help_text] = search_cache (str, cache_file, search_type);
    had_core_cache = true;
  else
    fun = help_text = {};
    had_core_cache = false;
  endif
  
  ## Search functions in path
  pathorig = __pathorig__ ();
  p = path ();
  idx = find (p == pathsep ());
  prev_idx = 1;
  for n = 1:length (idx)
    f = p (prev_idx:idx (n)-1);
    
    ## Should we search the directory or has it been covered by the cache?
    if (!had_core_cache || isempty (findstr (f, pathorig)))
      cache_file = fullfile (f, "DOC");
      if (exist (cache_file, "file"))
        ## We have a cache in the directory, then read it and search it!
        [funs, hts] = search_cache (str, cache_file, search_type);
        fun (end+1:end+length (funs)) = funs;
        help_text (end+1:end+length (hts)) = hts;
      else
      ## We don't have a cache. Search files
        funs_in_f = __list_functions__ (f);
        for m = 1:length (funs_in_f)
          fn = funs_in_f {m};
        
          ## Skip files that start with __
          if (length (fn) > 2 && strcmp (fn (1:2), "__"))
            continue;
          endif
        
          ## Extract first sentence
          try
            first_sentence = get_first_help_sentence (fn);
            status = 0;
          catch
            status = 1;
          end_try_catch

          if (search_type == 2) # search entire help text
            [text, format] = get_help_text (fn);
    
            ## Take action depending on help text format
            switch (lower (format))
              case "plain text"
                status = 0;
              case "texinfo"
                [text, status] = makeinfo (text, "plain text");
              case "html"
                [text, status] = strip_html_tags (text);
              otherwise
                status = 1;
            endswitch

          elseif (status == 0) # only search the first sentence of the help text
            text = first_sentence;
          endif
        
          ## Search the help text, if we can
          if (status == 0 && !isempty (strfind (text, str)))
            fun (end+1) = fn;
            help_text (end+1) = first_sentence;
          endif
        endfor
      endif
    endif
    prev_idx = idx (n) + 1;
  endfor
  
  if (nargout == 0)
    ## Print the results (FIXME: improve this to make it look better.
    indent = 20;
    term_width = terminal_size() (2);
    desc_width = term_width - indent - 2;
    indent_space = repmat (" ", 1, indent);
    for k = 1:length (fun)
      f = fun {k};
      f (end+1:indent) = " ";
      printf (f);
      desc = strtrim (strrep (help_text {k}, "\n", " "));
      ldesc = length (desc);
      printf ("%s\n", desc (1:min (desc_width, ldesc)));
      for start = desc_width+1:desc_width:ldesc
        stop = min (start + desc_width, ldesc);
        printf ("%s%s\n", indent_space, strtrim (desc (start:stop)));
      endfor
    endfor

  else
    ## Return the results instead of displaying them
    out_fun = fun;
    out_help_text = help_text;
  endif
endfunction

function [funs, help_texts] = search_cache (str, cache_file, search_type)
  load (cache_file);
  if (! isempty(cache))
    tmp = strfind (cache (search_type, :), str);
    cache_idx = find (!cellfun ("isempty", tmp));
    funs = cache (1, cache_idx);
    help_texts = cache (3, cache_idx);
  else
    funs = help_texts = {};
  endif
endfunction

