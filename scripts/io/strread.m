## Copyright (C) 2009-2011 Eric Chassande-Mottin, CNRS (France)
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
## @deftypefn  {Function File} {[@var{a}, @dots{}] =} strread (@var{str})
## @deftypefnx {Function File} {[@var{a}, @dots{}] =} strread (@var{str}, @var{format})
## @deftypefnx {Function File} {[@var{a}, @dots{}] =} strread (@var{str}, @var{format}, @var{format_repeat})
## @deftypefnx {Function File} {[@var{a}, @dots{}] =} strread (@var{str}, @var{format}, @var{prop1}, @var{value1}, @dots{})
## @deftypefnx {Function File} {[@var{a}, @dots{}] =} strread (@var{str}, @var{format}, @var{format_repeat}, @var{prop1}, @var{value1}, @dots{})
## Read data from a string.
##
## The string @var{str} is split into words that are repeatedly matched to the
## specifiers in @var{format}.  The first word is matched to the first
## specifier,
## the second to the second specifier and so forth.  If there are more words
## than
## specifiers, the process is repeated until all words have been processed.
##
## The string @var{format} describes how the words in @var{str} should be
## parsed.
## It may contain any combination of the following specifiers:
## @table @code
## @item %s
## The word is parsed as a string.
##
## @item %d
## @itemx %f
## @itemx %u
## @itemx %n
## The word is parsed as a number (and converted to double).
##
## @item %*', '%*f', '%*s
## The word is skipped.
##
## For %s and %d, %f, %n, %u and the associated %*s @dots{} specifiers an
## optional width can be specified as %Ns, etc. where N is an integer > 1.
## For %f, formats like %N.Mf are allowed.
##
## @item literals
## In addition the format may contain literal character strings; these will be
## skipped during reading.
## @end table
##
## Parsed word corresponding to the first specifier are returned in the first
## output argument and likewise for the rest of the specifiers.
##
## By default, @var{format} is @t{"%f"}, meaning that numbers are read from
## @var{str}.  This will do if @var{str} contains only numeric fields.
##
## For example, the string
##
## @example
## @group
## @var{str} = "\
## Bunny Bugs   5.5\n\
## Duck Daffy  -7.5e-5\n\
## Penguin Tux   6"
## @end group
## @end example
##
## @noindent
## can be read using
##
## @example
## [@var{a}, @var{b}, @var{c}] = strread (@var{str}, "%s %s %f");
## @end example
##
## Optional numeric argument @var{format_repeat} can be used for
## limiting the number of items read:
## @table @asis
## @item -1
## (default) read all of the string until the end.
##
## @item N
## Read N times @var{nargout} items.  0 (zero) is an acceptable
## value for @var{format_repeat}.
##
## @end table
##
## The behavior of @code{strread} can be changed via property-value
## pairs.  The following properties are recognized:
##
## @table @asis
## @item "commentstyle"
## Parts of @var{str} are considered comments and will be skipped.
## @var{value} is the comment style and can be any of the following.
## @itemize
## @item "shell"
## Everything from @code{#} characters to the nearest end-line is skipped.
##
## @item "c"
## Everything between @code{/*} and @code{*/} is skipped.
##
## @item "c++"
## Everything from @code{//} characters to the nearest end-line is skipped.
##
## @item "matlab"
## Everything from @code{%} characters to the nearest end-line is skipped.
##
## @item user-supplied.  Two options:
## (1) One string, or 1x1 cell string: Skip everything to the right of it;
## (2) 2x1 cell string array: Everything between the left and right strings
## is skipped.
## @end itemize
##
## @item "delimiter"
## Any character in @var{value} will be used to split @var{str} into words 
## (default value = any whitespace).
##
## @item "whitespace"
## Any character in @var{value} will be interpreted as whitespace and
## trimmed; the string defining whitespace must be enclosed in double
## quotes for proper processing of special characters like \t.
## The default value for whitespace = " \b\r\n\t" (note the space).
##
## @item "emptyvalue"
## Parts of the output where no word is available is filled with @var{value}.
##
## @item "treatasempty"
## Treat single occurrences (surrounded by delimiters or whitespace) of the
## string(s) in @var{value} as missing values.
##
## @item "returnonerror"
## If @var{value} true (1, default), ignore read errors and return normally.
## If false (0), return an error.
## 
## @end table
##
## @seealso{textscan, textread, load, dlmread, fscanf}
## @end deftypefn

function varargout = strread (str, format = "%f", varargin)

  ## Check input
  if (nargin < 1)
    print_usage ();
  endif

  if (isempty (format))
    format = "%f";
  endif

  if (! ischar (str) || ! ischar (format))
    error ("strread: STR and FORMAT arguments must be strings");
  endif

  ## Check for format string repeat count
  format_repeat_count = -1;
  if (nargin > 2 && isnumeric (varargin{1}))
    if (varargin{1} >= 0)
      format_repeat_count = varargin{1};
    endif
    if (nargin > 3)
      varargin = varargin(2:end);
    else
      varargin = {};
    endif
  endif

  ## Parse options.  First initialize defaults
  comment_flag = false;
  numeric_fill_value = NaN;
  white_spaces = " \b\r\n\t";
  delimiter_str = "";
  eol_char = "";
  empty_str = "";
  err_action = 0;
  for n = 1:2:length (varargin)
    switch (lower (varargin{n}))
      case "bufsize"
        ## We could synthesize this, but that just seems weird...
        warning ('strread: property "bufsize" is not implemented');
      case "commentstyle"
        comment_flag = true;
        switch (lower (varargin{n+1}))
          case "c"
            [comment_start, comment_end] = deal ("/*", "*/");
          case "c++"
            [comment_start, comment_end] = deal ("//", "\n");
          case "shell"
            [comment_start, comment_end] = deal ("#", "\n");
          case "matlab"
            [comment_start, comment_end] = deal ("%", "\n");
          otherwise
            if (ischar (varargin{n+1}) ||
               (numel (varargin{n+1}) == 1 && iscellstr (varargin{n+1})))
              tmp = char (varargin{n+1});
              [comment_start, comment_end] = deal (tmp, "\n");
            elseif (iscellstr (varargin{n+1}) && numel (varargin{n+1}) == 2)
              [comment_start, comment_end] = deal (varargin{n+1}{:});
            else
              ## FIXME - a user may have numeric values specified: {'//', 7}
              ##         this will lead to an error in the warning message
              error ("strread: unknown or unrecognized comment style '%s'", 
                      varargin{n+1});
            endif
        endswitch
      case "delimiter"
        delimiter_str = varargin{n+1};
      case "emptyvalue"
        numeric_fill_value = varargin{n+1};
      case "expchars"
        warning ('strread: property "expchars" is not implemented');
      case "whitespace"
        white_spaces = varargin{n+1};
      ## The following parameters are specific to textscan and textread
      case "endofline"
        eol_char = varargin{n+1};
      case "returnonerror"
        err_action = varargin{n+1};
      case "treatasempty"
        empty_str = varargin{n+1};
        if (ischar (empty_str))
          empty_str = {empty_str};
        endif
      otherwise
        warning ('strread: unknown property "%s"', varargin{n});
    endswitch
  endfor

  ## Parse format string to compare nr. of conversion fields and nargout
  idx = strfind (format, "%")';
  specif = format([idx, idx+1]);
  nspecif = length (idx);
  idx_star = strfind (format, "%*");
  nfields = length (idx) - length (idx_star);
  ## If str only has numeric fields, a (default) format ("%f") will do.
  ## Otherwise:
  if ((max (nargout, 1) != nfields) && ! strcmp (format, "%f"))
    error ("strread: the number of output variables must match that specified by FORMAT");
  endif

  ## Remove comments
  if (comment_flag)
    cstart = strfind (str, comment_start);
    cstop  = strfind (str, comment_end);
    if (length (cstart) > 0)
      ## Ignore nested openers.
      [idx, cidx] = unique (lookup (cstop, cstart), "first");
      if (idx(end) == length (cstop))
        cidx(end) = []; # Drop the last one if orphaned.
      endif
      cstart = cstart(cidx);
    endif
    if (length (cstop) > 0)
      ## Ignore nested closers.
      [idx, cidx] = unique (lookup (cstart, cstop), "first");
      if (idx(1) == 0)
        cidx(1) = []; # Drop the first one if orphaned.
      endif
      cstop = cstop(cidx);
    endif
    len = length (str);
    c2len = length (comment_end);
    str = cellslices (str, [1, cstop + c2len], [cstart - 1, len]);
    str = [str{:}];
  endif

  if (strcmpi (strtrim (format), "%f"))
    ## Default format specified.  Expand it (to desired nargout)
    num_words_per_line = nargout;
    fmt_words = cell (nargout, 1);
    fmt_words (1:nargout) = format;
  else
    ## Determine the number of words per line as a first guess.  Forms
    ## like %f<literal) (w/o delimiter in between) are fixed further on
    format = strrep (format, "%", " %");
    fmt_words = regexp (format, '[^ ]+', 'match');
    ## Format conversion specifiers following literals w/o space/delim
    ## in between are separate now.  Separate those w trailing literals
    idy2 = find (! cellfun ("isempty", strfind (fmt_words, "%")));
    a = strfind (fmt_words(idy2), "%");
    b = regexp (fmt_words(idy2), '[nfdus]', 'end');
    for jj = 1:numel (a)
      ii = numel (a) - jj + 1;
      if (! (length (fmt_words{idy2(ii)}) == b{ii}(1)))
        ## Fix format_words
        fmt_words(idy2(ii)+1 : end+1) = fmt_words(idy2(ii) : end);
        fmt_words{idy2(ii)} = fmt_words{idy2(ii)}(a{ii} : b{ii}(1));
        fmt_words{idy2(ii)+1} = fmt_words{idy2(ii)+1}(b{ii}+1:end);
      endif
    endfor
  endif
  num_words_per_line = numel (fmt_words);

  if (! isempty (white_spaces))
    ## Check for overlapping whitespaces and delimiters & trim whitespace
    if (! isempty (delimiter_str))
      [ovlp, iw] = intersect (white_spaces, delimiter_str);
      if (! isempty (ovlp))
        ## Remove delimiter chars from white_spaces
        white_spaces = cell2mat (strsplit (white_spaces, white_spaces(iw)));
      endif
    endif
  endif

  if (isempty (delimiter_str))
    delimiter_str = " ";
  endif

  if (! isempty (eol_char))
    ## eol_char is delimiter by default. First separate CRLF from single CR & LF
    if (strcmp (eol_char, "\r\n"))
      ## Strip CR from CRLF sequences
      str = strrep (str, "\r\n", "\n");
      ## CR serves no further purpose in function
      eol_char = "\n";  
    endif
    ## Add eol_char to delimiter collection
    delimiter_str = unique ([delimiter_str eol_char]);
  endif

  pad_out = 0;
  ## If needed, trim whitespace
  if (! isempty (white_spaces))
    ## Check if trailing "\n" might signal padding output arrays to equal size
    ## before it is trimmed away below
    if ((str(end) == 10) && (nargout > 1)) 
      pad_out = 1;
    endif
    ## Remove repeated white_space chars.  First find white_space positions
    idx = strchr (str, white_spaces);
    ## Find repeated white_spaces
    idx2 = ! (idx(2:end) - idx(1:end-1) - 1);
    ## Set all whitespace chars to spaces
    ## FIXME: this implies real spaces are always part of white_spaces
    str(idx) = ' ';
    ## Set all repeated white_space to \0
    str(idx(idx2)) = "\0";
    str = strsplit (str, "\0");
    ## Reconstruct trimmed str
    str = cell2mat (str);
    ## Remove leading & trailing space, but preserve delimiters.
    str = strtrim (str);
  endif

  ## Split 'str' into words
  words = split_by (str, delimiter_str);
  if (! isempty (white_spaces))
    ## Trim leading and trailing white_spaces
    words = strtrim (words);
  endif
  num_words = numel (words);
  ## First guess at number of lines in file (ignoring leading/trailing literals)
  num_lines = ceil (num_words / num_words_per_line);

  ## Replace TreatAsEmpty char sequences by empty strings
  if (! isempty (empty_str))
    ## FIXME: There should be a simpler way to do this with cellfun
    for ii = 1:numel (empty_str)
      idz = strmatch (empty_str{ii}, words, "exact");
      words(idz) = {""};
    endfor
  endif
  
  ## We now may have to cope with 3 cases:
  ## A: Trailing literals (%f<literal>) w/o delimiter in between.
  ## B: Leading literals (<literal>%f) w/o delimiter in between.
  ## C. Skipping leftover parts of specified skip fields (%*N )
  ## fmt_words has been split properly now, but words{} has only been split on
  ## delimiter positions.  Some words columns may have to be split further.
  ## We also don't know the number of lines (as EndOfLine may have been set to
  ## "" (empty) by the caller).

  ## Find indices and pointers to possible literals in fmt_words
  idf = cellfun ("isempty", strfind (fmt_words, "%"));
  ## Find indices and pointers to "%*" (skip) conversion specifiers
  idg = ! cellfun ("isempty", strfind (fmt_words, "%*"));
  ## Unselect those with specified width ("%*N")
  st = regexp (fmt_words, '\d');
  idy = find (idf);

  ## If needed, split up columns in three steps:
  if (! isempty (idy))
    ## Try-catch because complexity of strings to read can be infinite    
    try

      ## 1. Assess "period" in the split-up words array ( < num_words_per_line).
      ## Could be done using EndOfLine but that prohibits EndOfLine = "" option.
      fmt_in_word = cell (num_words_per_line, 1);
      words_period = 1;
      ## For each literal in turn
      for ii = 1:numel (idy)
        fmt_in_word(idy(ii)) = num_words;
        ## Find *current* "return period" for fmt_word{idy(ii)} in words 
        ## Search in first num_words_per_line of words
        litptrs = find (! cellfun ("isempty", strfind ...
                   (words(1:min (10*num_words_per_line, num_words)), ...
                   fmt_words{idy(ii)})));
        if (length (litptrs) > 1)
          litptr = sum (unique (litptrs(2:end) .- litptrs(1:end-1)));
        endif
      endfor
      words_period = max (words_period, litptr);
      num_lines = ceil (num_words / words_period);

      ## 2. Pad words array so that it can be reshaped
      tmp_lines = ceil (num_words / words_period);
      num_words_padded = tmp_lines * words_period - num_words;
      if (num_words_padded)
        words = [words'; cell(num_words_padded, 1)]; 
      endif
      words = reshape (words, words_period, tmp_lines);

      ## 3. Do the column splitting on rectangular words array
      icol = 1; ii = 1;    # icol = current column, ii = current fmt_word
      while (ii <= num_words_per_line)

        ## Check if fmt_words(ii) contains a literal
        if (idf(ii))             # Yes, fmt_words(ii) = literal
          [s, e] = regexp (words{icol, 1}, fmt_words{ii});
          if (isempty (s))
            warning ("Literal '%s' not found in column %d", fmt_words{ii}, icol);
          else
            if (! strcmp (fmt_words{ii}, words{icol, 1}))
              ## Column doesn't exactly match literal => split needed.  Add a column
              words(icol+1:end+1, :) = words(icol:end, :); 
              ## Watch out for empty cells
              jptr = find (! cellfun ("isempty", words(icol, :)));

              ## Distinguish leading or trailing literals
              if (!isempty (s) && s(1) == 1)
                ## Leading literal.  Assign literal to icol, paste rest in icol + 1
                ## Apply only to those cells that do have something beyond literal
                jptr = find ([cellfun(@(x) length(x), words(icol+1, jptr), ...
                              "UniformOutput", false){:}] > e(1));
                words(icol+1, jptr) = cellfun ...
                  (@(x) substr(x, e(1)+1, length(x)-e(1)), words(icol, jptr), ...
                  "UniformOutput", false);
                words(icol, jptr) = fmt_words{ii};

              else
                ## Trailing literal.  If preceding format == '%s' this is an error
                if (! isempty (strfind (fmt_words{ii-1}, "%s")))
                  warning ("Ambiguous '%s' specifier next to literal in column %d", icol);
                else
                  ## Some invoked code to avoid regexp which seems demanding
                  ## on large files
                  ## FIXME: this assumes char(254)/char(255) won't occur in input!
                  clear wrds;
                  wrds(1:2:2*numel (words(icol, jptr))) = ...
                       strrep (words(icol, jptr), fmt_words{ii}, ...
                       [char(255) char(254)]);
                  wrds(2:2:2*numel (words(icol, jptr))-1) = char(255);
                  wrds = strsplit ([wrds{:}], char(255));
                  words(icol, jptr) = ...
                    wrds(find (cellfun ("isempty", strfind (wrds, char(254)))));
                  wrds(find (cellfun ("isempty", strfind (wrds, char(254))))) ...
                     = char(255);
                  words(icol+1, jptr) = strsplit (strrep ([wrds{2:end}], ...
                     char(254), fmt_words{ii}), char(255));
                endif
                ## Former trailing literal may now be leading for next specifier
                --ii;
              endif
            endif
          endif

        else
          ## Conv. specifier.  Peek if next fmt_word needs split from current column
          if (ii < num_words_per_line && idf(ii+1))
            if (! isempty (strfind (words{icol, 1}, fmt_words{ii+1})))
              --icol;
            endif
          endif
        endif
        ## Next fmt_word, next column
        ++ii; ++icol;
      endwhile

      ## Done.  Reshape words back into 1 long vector and strip padded empty words
      words = reshape (words, 1, numel (words))(1 : end-num_words_padded);

    catch
      warning ("strread: unable to parse text or file with given format string");
      return;

    end_try_catch
  endif
  
  ## For each specifier, process corresponding column
  k = 1;
  for m = 1:num_words_per_line
    try
      if (format_repeat_count < 0)
        data = words(m:num_words_per_line:end);
      elseif (format_repeat_count == 0)
        data = {};
      else
        lastline = ...
          min (num_words_per_line * format_repeat_count + m - 1, numel (words));
        data = words(m:num_words_per_line:lastline);
      endif

      ## Map to format
      ## FIXME - add support for formats like "<%s>", "%[a-zA-Z]"
      ##         Someone with regexp experience is needed.
      switch fmt_words{m}(1:min (2, length (fmt_words{m})))
        case "%s"
          if (pad_out)
            data(end+1:num_lines) = {""}; 
          endif
          varargout{k} = data';
          k++;
        case {"%d", "%u", "%f", "%n"}
          n = cellfun ("isempty", data);
          ### FIXME - erroneously formatted data lead to NaN, not an error
          data = str2double (data);
          data(n) = numeric_fill_value;
          if (pad_out)
            data(end+1:num_lines) = numeric_fill_value;
          endif
          varargout{k} = data.';
          k++;
        case {"%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%8", "%9"}
          nfmt = strsplit (fmt_words{m}(2:end-1), '.');
          swidth = str2num (nfmt{1});
          switch fmt_words{m}(end)
            case {"d", "u", "f", "n%"}
              n = cellfun ("isempty", data);
              ### FIXME - erroneously formatted data lead to NaN, not an error
              ###         => ReturnOnError can't be implemented for numeric data
              data = str2double (strtrunc (data, swidth));
              data(n) = numeric_fill_value;
              if (pad_out)
                data(end+1:num_lines) = numeric_fill_value;
              endif
              if (numel (nfmt) > 1)
                sprec = str2num (nfmt{2});
                data = 10^-sprec * round (10^sprec * data);
              endif
              varargout{k} = data.';
              k++;
            case "s"
              if (pad_out)
                data(end+1:num_lines) = {""}
              endif
              varargout{k} = strtrunc (data, 3)';
              k++;
            otherwise
          endswitch
        case {"%*", "%*s"}
          ## skip the word
        otherwise
          ## Ensure descriptive content is consistent
          if (numel (unique (data)) > 1
              || ! strcmpi (unique (data), fmt_words{m}))
            error ("strread: FORMAT does not match data");
          endif
      endswitch
    catch
      ## As strread processes columnwise, ML-compatible error processing
      ## (row after row) is not feasible. In addition Octave sets unrecognizable
      ## numbers to NaN w/o error.  But maybe Octave is better in this respect.
      if (err_action)
        ## Just try the next column where ML bails out
      else
        rethrow (lasterror);
      endif
    end_try_catch
  endfor

endfunction

function out = split_by (text, sep)
  out = strsplit (text, sep);
  out(cellfun ("isempty", out)) = {""};
endfunction


%!test
%! [a, b] = strread ("1 2", "%f%f");
%! assert (a == 1 && b == 2);

%!test
%! str = "# comment\n# comment\n1 2 3";
%! [a, b] = strread (str, '%d %s', 'commentstyle', 'shell');
%! assert (a, [1; 3]);
%! assert (b, {"2"});

%!test
%! str = '';
%! a = rand (10, 1);
%! b = char (randi ([65, 85], 10, 1));
%! for k = 1:10
%!   str = sprintf ('%s %.6f %s\n', str, a(k), b(k));
%! endfor
%! [aa, bb] = strread (str, '%f %s');
%! assert (a, aa, 1e-5);
%! assert (cellstr (b), bb);

%!test
%! str = '';
%! a = rand (10, 1);
%! b = char (randi ([65, 85], 10, 1));
%! for k = 1:10
%!   str = sprintf ('%s %.6f %s\n', str, a(k), b(k));
%! endfor
%! aa = strread (str, '%f %*s');
%! assert (a, aa, 1e-5);

%!test
%! str = sprintf ('/* this is\nacomment*/ 1 2 3');
%! a = strread (str, '%f', 'commentstyle', 'c');
%! assert (a, [1; 2; 3]);

%!test
%! str = sprintf ("Tom 100 miles/hr\nDick 90 miles/hr\nHarry 80 miles/hr");
%! fmt = "%s %f miles/hr";
%! c = cell (1, 2);
%! [c{:}] = strread (str, fmt);
%! assert (c{1}, {"Tom"; "Dick"; "Harry"})
%! assert (c{2}, [100; 90; 80])

%!test
%! a = strread ("a b c, d e, , f", "%s", "delimiter", ",");
%! assert (a, {"a b c"; "d e"; ""; "f"});

%!test
%! # Bug #33536
%! [a, b, c] = strread ("1,,2", "%s%s%s", "delimiter", ",");
%! assert (a{1}, '1');
%! assert (b{1}, '');
%! assert (c{1}, '2');

%!test
%! # Bug #33536
%! a = strread ("[SomeText]", "%s", "delimiter", "]");
%! assert (a{1}, "[SomeText");
%! assert (a{2}, '');

%!test
%! dat = "Data file.\r\n=  =  =  =  =\r\nCOMPANY    : <Company name>\r\n";
%! a = strread (dat, "%s", 'delimiter', "\n", 'whitespace', '', 'endofline', "\r\n");
%! assert (a{2}, "=  =  =  =  =");
%! assert (double (a{3}(end-5:end)), [32 110 97 109 101 62]);

%!test
%! [a, b, c, d] = strread ("1,2,3,,5,6", "%d%d%d%d", 'delimiter', ',');
%! assert (c, 3);
%! assert (d, NaN);

%!test
%! [a, b, c, d] = strread ("1,2,3,,5,6\n", "%d%d%d%d", 'delimiter', ',');
%! assert (c, [3; NaN]);
%! assert (d, [NaN; NaN]);

%!test
%! # Default format (= %f)
%1 [a, b, c] = strread ("0.12 0.234 0.3567");
%1 assert (a, 0.12);
%1 assert (b, 0.234);
%1 assert (c, 0.3567);

%!test
%! [a, b] = strread('0.41 8.24 3.57 6.24 9.27', "%f%f", 2, 'delimiter', ' ');
%1 assert (a, [0.41; 3.57]);

%!test
%! # TreatAsEmpty
%! [a, b, c, d] = strread ("1,2,3,NN,5,6\n", "%d%d%d%d", 'delimiter', ',', 'TreatAsEmpty', 'NN');
%! assert (c, [3; NaN]);
%! assert (d, [NaN; NaN]);

%!test
%! # No delimiters at all besides EOL.  Plain reading numbers & strings
%! str = "Text1Text2Text\nText398Text4Text\nText57Text";
%! c = textscan (str, "Text%dText%1sText");
%! assert (c{1}, [1; 398; 57]);
%! assert (c{2}(1:2), {'2'; '4'});
%! assert (isempty (c{2}{3}), true);

%!test
%! # No delimiters at all besides EOL.  Skip fields, even empty fields
%! str = "Text1Text2Text\nTextText4Text\nText57Text";
%! c = textscan (str, "Text%*dText%dText");
%! assert (c{1}, [2; 4; NaN]);

