#! /bin/sh

########################################################################
##
## Copyright (C) 2016-2023 The Octave Project Developers
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

# Generate a header file that provides the public symbols from Octave's
# autoconf-generated config.h file.  See the notes at the top of the
# generated octave-config.h file for more details.

SED=${SED:-sed}

if [ $# -ne 1 ]; then
  echo "usage: mk-octave-config-h.sh CONFIG-FILE" 1>&2
  exit 1
fi

config_h_file=$1

cat << EOF
// DO NOT EDIT!  Generated by mk-octave-config-h.sh.

////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016-2023 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

// All Octave source files should begin with
//
//   #if defined (HAVE_CONFIG_H)
//   #  include "config.h"
//   #endif
//
// All public Octave header files should have the form
//
//   #if ! defined (INCLUSION_GUARD_SYMBOL)
//   #define INCLUSION_GUARD_SYMBOL 1
//
//   #include "octave-config.h"
//
//   ... Contents of header file ...
//
//   #endif

// In Octave source files, INCLUSION_GUARD_SYMBOL should have the form
//
//   octave_NAME_h
//
// with NAME formed from the header file name with '-' replaced by '_'.
//
// It is safe to include octave-config.h unconditionally since it will
// expand to an empty file if it is included after Octave's
// autoconf-generated config.h file.
//
// Users of Octave's libraries should not need to include octave-config.h
// since all of Octave's header files already include it.

#if ! defined (octave_octave_config_h)
#define octave_octave_config_h 1

#if ! defined (OCTAVE_AUTOCONFIG_H_INCLUDED)
EOF

$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_64.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_BOUNDS_CHECK.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_FLOAT_TRUNCATE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_INTERNAL_CHECKS.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_LIB_VISIBILITY_FLAGS.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_OPENMP.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_F77_INT_TYPE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_LONG_LONG_INT.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_OVERLOAD_CHAR_INT8_TYPES.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_STD_PMR_POLYMORPHIC_ALLOCATOR.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_UNSIGNED_LONG_LONG_INT.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_IDX_TYPE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_SIZEOF_F77_INT_TYPE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_SIZEOF_IDX_TYPE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_SIZEOF_INT.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) gid_t.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) uid_t.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) nlink_t.*$\)/#  \1/p' $config_h_file

if grep "#undef HAVE_DEV_T" $config_h_file > /dev/null; then
  cat << EOF
typedef short dev_t;
EOF
else
  cat << EOF
/* typedef short dev_t; */
EOF
fi

if grep "#undef HAVE_INO_T" $config_h_file > /dev/null; then
    cat << EOF
typedef unsigned long ino_t;
EOF
else
    cat << EOF
/* typedef unsigned long ino_t; */
EOF
fi

echo ""

have_roundl=no
if grep "#define HAVE_ROUNDL" $config_h_file > /dev/null; then
  have_roundl=yes
fi

sizeof_long_double="`$SED -n 's/#define SIZEOF_LONG_DOUBLE \([0-9]*\)/\1/p' $config_h_file`"

if test -z "$sizeof_long_double"; then
  echo "mk-octave-config-h.sh: failed to find SIZEOF_LONG_DOUBLE in $config_h_file" 1>&2
  exit 1
fi

if test $sizeof_long_double -ge 10 && test $have_roundl = yes; then
  echo "#  define OCTAVE_INT_USE_LONG_DOUBLE 1"
  if test $sizeof_long_double -lt 16; then
    cat << EOF
#  if (defined (__i386__) || defined (__x86_64__)) && defined (__GNUC__)
#    define OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED 1
#  endif
EOF
  else
    cat << EOF
/* #  undef OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED */
EOF
  fi
else
  cat << EOF
/* #  undef OCTAVE_INT_USE_LONG_DOUBLE */
/* #  undef OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED */
EOF
fi

echo ""

$SED -n 's/#\(\(undef\|define\) F77_USES_.*$\)/#  \1/p' $config_h_file

echo ""

$SED -n 's/#\(\(undef\|define\) F77_FUNC.*$\)/#  \1/p' $config_h_file

cat << EOF

/* Enable inline functions or typedefs that provide access to
   symbols that have been moved to the octave namespace so that
   users of Octave may continue to access symbols using the
   deprecated names.  */
#  define OCTAVE_PROVIDE_DEPRECATED_SYMBOLS 1

#  include "oct-conf-post-public.h"

#endif

#endif
EOF
