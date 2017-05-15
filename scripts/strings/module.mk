FCN_FILE_DIRS += scripts/strings

%canon_reldir%_FCN_FILES = \
  %reldir%/base2dec.m \
  %reldir%/bin2dec.m \
  %reldir%/blanks.m \
  %reldir%/cstrcat.m \
  %reldir%/deblank.m \
  %reldir%/dec2base.m \
  %reldir%/dec2bin.m \
  %reldir%/dec2hex.m \
  %reldir%/findstr.m \
  %reldir%/hex2dec.m \
  %reldir%/index.m \
  %reldir%/isletter.m \
  %reldir%/isstrprop.m \
  %reldir%/mat2str.m \
  %reldir%/native2unicode.m \
  %reldir%/ostrsplit.m \
  %reldir%/regexptranslate.m \
  %reldir%/rindex.m \
  %reldir%/str2num.m \
  %reldir%/strcat.m \
  %reldir%/strchr.m \
  %reldir%/strjoin.m \
  %reldir%/strjust.m \
  %reldir%/strmatch.m \
  %reldir%/strsplit.m \
  %reldir%/strtok.m \
  %reldir%/strtrim.m \
  %reldir%/strtrunc.m \
  %reldir%/substr.m \
  %reldir%/unicode2native.m \
  %reldir%/untabify.m \
  %reldir%/validatestring.m

%canon_reldir%dir = $(fcnfiledir)/strings

%canon_reldir%_DATA = $(%canon_reldir%_FCN_FILES)

FCN_FILES += $(%canon_reldir%_FCN_FILES)

PKG_ADD_FILES += %reldir%/PKG_ADD

DIRSTAMP_FILES += %reldir%/$(octave_dirstamp)
