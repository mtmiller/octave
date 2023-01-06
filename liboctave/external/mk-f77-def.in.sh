#! /bin/sh

########################################################################
##
## Copyright (C) 2006-2023 The Octave Project Developers
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

: ${SED=@SED@}
: ${AWK=@AWK@}

F77_TOLOWER="@F77_TOLOWER@"
F77_APPEND_UNDERSCORE="@F77_APPEND_UNDERSCORE@"
F77_APPEND_EXTRA_UNDERSCORE="@F77_APPEND_EXTRA_UNDERSCORE@"

if test x$F77_TOLOWER = xyes; then
  case_cmd=tolower
else
  case_cmd=toupper
fi

if test x$F77_APPEND_UNDERSCORE = xyes; then
  uscore=_
else
  uscore=
fi

if test x$F77_APPEND_EXTRA_UNDERSCORE = xyes; then
  awkcmd="$AWK '{ if (\$0 ~ /_/) extra = \"_\"; else extra = \"\"; printf (\"%s%s%s\n\", $case_cmd (\$0), \"$uscore\", extra); }'"
else
  awkcmd="$AWK '{ printf (\"%s%s\n\", tolower (\$0), \"$uscore\"); }'"
fi

if [ $# -gt 1 ]; then
  srcdir="$1"
  shift
fi

echo EXPORTS
for arg
do
  case "$arg" in
    *.f)
      ## There are TABS in this sed command.
      $SED -n -e 'y/ABCDEFGHIJLKMNOPQRSTUVWXYZ/abcdefghijlkmnopqrstuvwxyz/; s/^\(      \|	\)[ 	]*\(.*function\|subroutine\|entry\)[ 	]*\([^ 	(]*\).*$/\3/p' "$srcdir/$arg" | eval $awkcmd
    ;;
  esac
done
