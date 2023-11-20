#!/bin/sh
# Convenience script for regenerating all autogeneratable files that are
# omitted from the version control repository. In particular, this script
# also regenerates all aclocal.m4, config.h.in, Makefile.in, configure files
# with new versions of autoconf or automake.

# Copyright (C) 2003-2023 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Originally written by Paul Eggert.  The canonical version of this
# script is maintained as top/autogen.sh in gnulib.  However, to be
# useful to your package, you should place a copy of it under version
# control in the top-level directory of your package.  The intent is
# that all customization can be done with a bootstrap.conf file also
# maintained in your version control; gnulib comes with a template
# build-aux/bootstrap.conf to get you started.
#
# Alternatively, you can use an autogen.sh script that is specific
# to your package.

me="$0"
medir=`dirname "$me"`

# Read the function library and the configuration.
. "$medir"/bootstrap-funclib.sh

autogen "$@"
