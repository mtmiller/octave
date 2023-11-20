# A library of shell functions for autopull.sh, autogen.sh, and bootstrap.

scriptlibversion=2023-08-29.21; # UTC

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
# script is maintained as top/bootstrap-funclib.sh in gnulib.  However,
# to be useful to your package, you should place a copy of it under
# version control in the top-level directory of your package.  The
# intent is that all customization can be done with a bootstrap.conf
# file also maintained in your version control; gnulib comes with a
# template build-aux/bootstrap.conf to get you started.

nl='
'

# Ensure file names are sorted consistently across platforms.
LC_ALL=C
export LC_ALL

# Honor $PERL, but work even if there is none.
PERL="${PERL-perl}"

default_gnulib_url=https://git.savannah.gnu.org/git/gnulib.git

# Copyright year, for the --version output.
copyright_year=`echo "$scriptlibversion" | sed -e 's/[^0-9].*//'`
copyright="Copyright (C) ${copyright_year} Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law."

# warnf_ FORMAT-STRING ARG1...
warnf_ ()
{
  warnf_format_=$1
  shift
  nl='
'
  case $* in
    *$nl*) me_=$(printf "$me"|tr "$nl|" '??')
       printf "$warnf_format_" "$@" | sed "s|^|$me_: |" ;;
    *) printf "$me: $warnf_format_" "$@" ;;
  esac >&2
}

# warn_ WORD1...
warn_ ()
{
  # If IFS does not start with ' ', set it and emit the warning in a subshell.
  case $IFS in
    ' '*) warnf_ '%s\n' "$*";;
    *)    (IFS=' '; warn_ "$@");;
  esac
}

# die WORD1...
die() { warn_ "$@"; exit 1; }

# ------------------------------ Configuration. ------------------------------

# Directory that contains package-specific gnulib modules and/or overrides.
local_gl_dir=gl

# Name of the Makefile.am
# XXX Not used.
gnulib_mk=gnulib.mk

# List of gnulib modules needed.
gnulib_modules=

# Any gnulib files needed that are not in modules.
gnulib_files=

# A function to be called for each unrecognized option.  Returns 0 if
# the option in $1 has been processed by the function.  Returns 1 if
# the option has not been processed by the function.  Override it via
# your own definition in bootstrap.conf
bootstrap_option_hook() { return 1; }

# A function to be called in order to print the --help information
# corresponding to user-defined command-line options.
bootstrap_print_option_usage_hook() { :; }

# A function to be called at the end of autopull.sh.
# Override it via your own definition in bootstrap.conf.
bootstrap_post_pull_hook() { :; }

# A function to be called right after gnulib-tool is run.
# Override it via your own definition in bootstrap.conf.
bootstrap_post_import_hook() { :; }

# A function to be called after everything else in this script.
# Override it via your own definition in bootstrap.conf.
bootstrap_epilogue() { :; }

# The command to download all .po files for a specified domain into a
# specified directory.  Fill in the first %s with the destination
# directory and the second with the domain name.
po_download_command_format=\
"wget --mirror --level=1 -nd -nv -A.po -P '%s' \
 https://translationproject.org/latest/%s/"

# Prefer a non-empty tarname (4th argument of AC_INIT if given), else
# fall back to the package name (1st argument with munging).
extract_package_name='
  /^AC_INIT(\[*/{
     s///
     /^[^,]*,[^,]*,[^,]*,[ []*\([^][ ,)]\)/{
       s//\1/
       s/[],)].*//
       p
       q
     }
     s/[],)].*//
     s/^GNU //
     y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/
     s/[^abcdefghijklmnopqrstuvwxyz0123456789_]/-/g
     p
  }
'
package=$(${AUTOCONF:-autoconf} --trace AC_INIT:\$4 configure.ac 2>/dev/null)
if test -z "$package"; then
  package=$(sed -n "$extract_package_name" configure.ac) \
      || die 'cannot find package name in configure.ac'
fi
gnulib_name=lib$package

build_aux=build-aux
source_base=lib
m4_base=m4
doc_base=doc
tests_base=tests
gnulib_extra_files="
        build-aux/install-sh
        build-aux/mdate-sh
        build-aux/texinfo.tex
        build-aux/depcomp
        build-aux/config.guess
        build-aux/config.sub
        doc/INSTALL
"

# Additional gnulib-tool options to use.  Use "\newline" to break lines.
gnulib_tool_option_extras=

# Other locale categories that need message catalogs.
EXTRA_LOCALE_CATEGORIES=

# Additional xgettext options to use.  Use "\\\newline" to break lines.
XGETTEXT_OPTIONS='\\\
 --flag=_:1:pass-c-format\\\
 --flag=N_:1:pass-c-format\\\
 --flag=error:3:c-format --flag=error_at_line:5:c-format\\\
'

# Package bug report address and copyright holder for gettext files
COPYRIGHT_HOLDER='Free Software Foundation, Inc.'
MSGID_BUGS_ADDRESS=bug-$package@gnu.org

# Files we don't want to import.
# XXX Not used.
excluded_files=

# File that should exist in the top directory of a checked out hierarchy,
# but not in a distribution tarball.
checkout_only_file=README-hacking

# Set this to '.cvsignore .gitignore' in bootstrap.conf if you want
# those files to be generated in directories like lib/, m4/, and po/.
# Or set it to 'auto' to make this script select which to use based
# on which version control system (if any) is used in the source directory.
vc_ignore=auto

# Set this to true in bootstrap.conf to enable --bootstrap-sync by
# default.
bootstrap_sync=false

# Override the default configuration, if necessary.
# Make sure that bootstrap.conf is sourced from the current directory
# if we were invoked as "sh bootstrap".
conffile=`dirname "$me"`/bootstrap.conf
test -r "$conffile" && . "$conffile"

# ------------------------- Build-time prerequisites -------------------------

check_exists() {
  if test "$1" = "--verbose"; then
    ($2 --version </dev/null) >/dev/null 2>&1
    if test $? -ge 126; then
      # If not found, run with diagnostics as one may be
      # presented with env variables to set to find the right version
      ($2 --version </dev/null)
    fi
  else
    ($1 --version </dev/null) >/dev/null 2>&1
  fi

  test $? -lt 126
}

# Note this deviates from the version comparison in automake
# in that it treats 1.5 < 1.5.0, and treats 1.4.4a < 1.4-p3a
# but this should suffice as we won't be specifying old
# version formats or redundant trailing .0 in bootstrap.conf.
# If we did want full compatibility then we should probably
# use m4_version_compare from autoconf.
sort_ver() { # sort -V is not generally available
  ver1="$1"
  ver2="$2"

  # split on '.' and compare each component
  i=1
  while : ; do
    p1=$(echo "$ver1" | cut -d. -f$i)
    p2=$(echo "$ver2" | cut -d. -f$i)
    if [ ! "$p1" ]; then
      echo "$1 $2"
      break
    elif [ ! "$p2" ]; then
      echo "$2 $1"
      break
    elif [ ! "$p1" = "$p2" ]; then
      if [ "$p1" -gt "$p2" ] 2>/dev/null; then # numeric comparison
        echo "$2 $1"
      elif [ "$p2" -gt "$p1" ] 2>/dev/null; then # numeric comparison
        echo "$1 $2"
      else # numeric, then lexicographic comparison
        lp=$(printf "%s\n%s\n" "$p1" "$p2" | LANG=C sort -n | tail -n1)
        if [ "$lp" = "$p2" ]; then
          echo "$1 $2"
        else
          echo "$2 $1"
        fi
      fi
      break
    fi
    i=$(($i+1))
  done
}

get_version_sed='
# Move version to start of line.
s/.*[v ]\([0-9]\)/\1/

# Skip lines that do not start with version.
/^[0-9]/!d

# Remove characters after the version.
s/[^.a-z0-9-].*//

# The first component must be digits only.
s/^\([0-9]*\)[a-z-].*/\1/

#the following essentially does s/5.005/5.5/
s/\.0*\([1-9]\)/.\1/g
p
q'

get_version() {
  app=$1

  $app --version >/dev/null 2>&1 || { $app --version; return 1; }

  $app --version 2>&1 | sed -n "$get_version_sed"
}

check_versions() {
  ret=0

  while read app req_ver; do
    # We only need libtoolize from the libtool package.
    if test "$app" = libtool; then
      app=libtoolize
    fi
    # Exempt git if git is not needed.
    if test "$app" = git; then
      $check_git || continue
    fi
    # Honor $APP variables ($TAR, $AUTOCONF, etc.)
    appvar=$(echo $app | LC_ALL=C tr '[a-z]-' '[A-Z]_')
    test "$appvar" = TAR && appvar=AMTAR
    case $appvar in
        GZIP) ;; # Do not use $GZIP:  it contains gzip options.
        PERL::*) ;; # Keep perl modules as-is
        *) eval "app=\${$appvar-$app}" ;;
    esac

    # Handle the still-experimental Automake-NG programs specially.
    # They remain named as the mainstream Automake programs ("automake",
    # and "aclocal") to avoid gratuitous incompatibilities with
    # preexisting usages (by, say, autoreconf, or custom autogen.sh
    # scripts), but correctly identify themselves (as being part of
    # "GNU automake-ng") when asked their version.
    case $app in
      automake-ng|aclocal-ng)
        app=${app%-ng}
        ($app --version | grep '(GNU automake-ng)') >/dev/null 2>&1 || {
          warn_ "Error: '$app' not found or not from Automake-NG"
          ret=1
          continue
        } ;;
      # Another check is for perl modules.  These can be written as
      # e.g. perl::XML::XPath in case of XML::XPath module, etc.
      perl::*)
        # Extract module name
        app="${app#perl::}"
        if ! $PERL -m"$app" -e 'exit 0' >/dev/null 2>&1; then
          warn_ "Error: perl module '$app' not found"
          ret=1
        fi
        continue
        ;;
    esac
    if [ "$req_ver" = "-" ]; then
      # Merely require app to exist; not all prereq apps are well-behaved
      # so we have to rely on $? rather than get_version.
      if ! check_exists --verbose $app; then
        warn_ "Error: '$app' not found"
        ret=1
      fi
    else
      # Require app to produce a new enough version string.
      inst_ver=$(get_version $app)
      if [ ! "$inst_ver" ]; then
        warn_ "Error: '$app' not found"
        ret=1
      else
        latest_ver=$(sort_ver $req_ver $inst_ver | cut -d' ' -f2)
        if [ ! "$latest_ver" = "$inst_ver" ]; then
          warnf_ '%s\n'                                        \
              "Error: '$app' version == $inst_ver is too old"  \
              "       '$app' version >= $req_ver is required"
          ret=1
        fi
      fi
    fi
  done

  return $ret
}

print_versions() {
  echo "Program    Min_version"
  echo "----------------------"
  printf %s "$buildreq"
  echo "----------------------"
  # can't depend on column -t
}

# check_build_prerequisites check_git
check_build_prerequisites()
{
  check_git="$1"

  # gnulib-tool requires at least automake and autoconf.
  # If either is not listed, add it (with minimum version) as a prerequisite.
  case $buildreq in
    *automake*) ;;
    *) buildreq="automake 1.9
$buildreq" ;;
  esac
  case $buildreq in
    *autoconf*) ;;
    *) buildreq="autoconf 2.59
$buildreq" ;;
  esac

  # When we can deduce that gnulib-tool will require patch,
  # and when patch is not already listed as a prerequisite, add it, too.
  if test -d "$local_gl_dir" \
     && ! find "$local_gl_dir" -name '*.diff' -exec false {} +; then
    case $buildreq in
      *patch*) ;;
      *) buildreq="patch -
$buildreq" ;;
    esac
  fi

  if ! printf '%s' "$buildreq" | check_versions; then
    echo >&2
    if test -f README-prereq; then
      die "See README-prereq for how to get the prerequisite programs"
    else
      die "Please install the prerequisite programs"
    fi
  fi

  # Warn the user if autom4te appears to be broken; this causes known
  # issues with at least gettext 0.18.3.
  probe=$(echo 'm4_quote([hi])' | autom4te -l M4sugar -t 'm4_quote:$%' -)
  if test "x$probe" != xhi; then
    warn_ "WARNING: your autom4te wrapper eats stdin;"
    warn_ "if bootstrap fails, consider upgrading your autotools"
  fi
}

# find_tool ENVVAR NAMES...
# -------------------------
# Search for a required program.  Use the value of ENVVAR, if set,
# otherwise find the first of the NAMES that can be run.
# If found, set ENVVAR to the program name, die otherwise.
#
# FIXME: code duplication, see also gnu-web-doc-update.
find_tool ()
{
  find_tool_envvar=$1
  shift
  find_tool_names=$@
  eval "find_tool_res=\$$find_tool_envvar"
  if test x"$find_tool_res" = x; then
    for i; do
      if check_exists $i; then
        find_tool_res=$i
        break
      fi
    done
  fi
  if test x"$find_tool_res" = x; then
    warn_ "one of these is required: $find_tool_names;"
    die   "alternatively set $find_tool_envvar to a compatible tool"
  fi
  eval "$find_tool_envvar=\$find_tool_res"
  eval "export $find_tool_envvar"
}

# --------------------- Preparing GNULIB_SRCDIR for use. ---------------------
# This is part of autopull.sh, but bootstrap needs it too, for self-upgrading.

cleanup_gnulib() {
  status=$?
  # XXX It's a bad idea to erase the submodule directory if it contains local
  #     modifications.
  rm -fr "$gnulib_path"
  exit $status
}

git_modules_config () {
  test -f .gitmodules && git config --file .gitmodules "$@"
}

prepare_GNULIB_SRCDIR ()
{
  if test -n "$GNULIB_SRCDIR"; then
    # Use GNULIB_SRCDIR directly.
    # We already checked that $GNULIB_SRCDIR references a directory.
    # Verify that it contains a gnulib checkout.
    test -f "$GNULIB_SRCDIR/gnulib-tool" \
      || die "Error: --gnulib-srcdir or \$GNULIB_SRCDIR is specified," \
             "but does not contain gnulib-tool"
  elif $use_git; then
    gnulib_path=$(git_modules_config submodule.gnulib.path)
    test -z "$gnulib_path" && gnulib_path=gnulib

    # Get gnulib files.  Populate $gnulib_path, possibly updating a
    # submodule, for use in the rest of the script.

    if test -n "$GNULIB_REFDIR" && test -d "$GNULIB_REFDIR"/.git \
       && git_modules_config submodule.gnulib.url >/dev/null; then
      # Use GNULIB_REFDIR as a reference.
      echo "$0: getting gnulib files..."
      if git submodule -h|grep -- --reference > /dev/null; then
        # Prefer the one-liner available in git 1.6.4 or newer.
        git submodule update --init --reference "$GNULIB_REFDIR" \
          "$gnulib_path" || exit $?
      else
        # This fallback allows at least git 1.5.5.
        if test -f "$gnulib_path"/gnulib-tool; then
          # Since file already exists, assume submodule init already complete.
          git submodule update -- "$gnulib_path" || exit $?
        else
          # Older git can't clone into an empty directory.
          rmdir "$gnulib_path" 2>/dev/null
          git clone --reference "$GNULIB_REFDIR" \
            "$(git_modules_config submodule.gnulib.url)" "$gnulib_path" \
            && git submodule init -- "$gnulib_path" \
            && git submodule update -- "$gnulib_path" \
            || exit $?
        fi
      fi
    else
      # GNULIB_REFDIR is not set or not usable. Ignore it.
      if git_modules_config submodule.gnulib.url >/dev/null; then
        echo "$0: getting gnulib files..."
        git submodule init -- "$gnulib_path" || exit $?
        git submodule update -- "$gnulib_path" || exit $?

      elif [ ! -d "$gnulib_path" ]; then
        echo "$0: getting gnulib files..."

        trap cleanup_gnulib HUP INT PIPE TERM

        shallow=
        if test -z "$GNULIB_REVISION"; then
          if git clone -h 2>&1 | grep -- --depth > /dev/null; then
            shallow='--depth 2'
          fi
          git clone $shallow ${GNULIB_URL:-$default_gnulib_url} "$gnulib_path" \
            || cleanup_gnulib
        else
          if git fetch -h 2>&1 | grep -- --depth > /dev/null; then
            shallow='--depth 2'
          fi
          mkdir -p "$gnulib_path"
          # Only want a shallow checkout of $GNULIB_REVISION, but git does not
          # support cloning by commit hash. So attempt a shallow fetch by commit
          # hash to minimize the amount of data downloaded and changes needed to
          # be processed, which can drastically reduce download and processing
          # time for checkout. If the fetch by commit fails, a shallow fetch can
          # not be performed because we do not know what the depth of the commit
          # is without fetching all commits. So fall back to fetching all
          # commits.
          git -C "$gnulib_path" init
          git -C "$gnulib_path" remote add origin \
              ${GNULIB_URL:-$default_gnulib_url}
          git -C "$gnulib_path" fetch $shallow origin "$GNULIB_REVISION" \
            || git -C "$gnulib_path" fetch origin \
            || cleanup_gnulib
          git -C "$gnulib_path" reset --hard FETCH_HEAD
        fi

        trap - HUP INT PIPE TERM

      elif test -n "$GNULIB_REVISION" \
           && ! git --git-dir="$gnulib_path"/.git cat-file \
                commit "$GNULIB_REVISION"; then
        git --git-dir="$gnulib_path"/.git fetch
      fi
    fi
    GNULIB_SRCDIR=$gnulib_path
    # Verify that the submodule contains a gnulib checkout.
    test -f "$gnulib_path/gnulib-tool" \
      || die "Error: $gnulib_path is supposed to contain a gnulib checkout," \
             "but does not contain gnulib-tool"
  fi

  # XXX Should this be done if $use_git is false?
  if test -d "$GNULIB_SRCDIR"/.git && test -n "$GNULIB_REVISION" \
     && ! git_modules_config submodule.gnulib.url >/dev/null; then
    if ! git --git-dir="$GNULIB_SRCDIR"/.git cat-file \
         commit "$GNULIB_REVISION"; then
      git --git-dir="$GNULIB_SRCDIR"/.git fetch
    fi
    (cd "$GNULIB_SRCDIR" && git checkout "$GNULIB_REVISION") || cleanup_gnulib
  fi

  # $GNULIB_SRCDIR now points to the version of gnulib to use, and
  # we no longer need to use git or $gnulib_path below here.
}

# -------- Upgrading bootstrap to the version found in GNULIB_SRCDIR. --------

upgrade_bootstrap ()
{
  if test -f "$medir"/bootstrap-funclib.sh; then
    update_lib=true
    { cmp -s "$medir"/bootstrap "$GNULIB_SRCDIR/top/bootstrap" \
      && cmp -s "$medir"/bootstrap-funclib.sh \
                "$GNULIB_SRCDIR/top/bootstrap-funclib.sh" \
      && cmp -s "$medir"/autopull.sh "$GNULIB_SRCDIR/top/autopull.sh" \
      && cmp -s "$medir"/autogen.sh "$GNULIB_SRCDIR/top/autogen.sh"; \
    }
  else
    update_lib=false
    cmp -s "$medir"/bootstrap "$GNULIB_SRCDIR/build-aux/bootstrap"
  fi || {
    if $update_lib; then
      echo "$0: updating bootstrap & companions and restarting..."
    else
      echo "$0: updating bootstrap and restarting..."
    fi
    case $(sh -c 'echo "$1"' -- a) in
      a) ignored=--;;
      *) ignored=ignored;;
    esac
    u=$update_lib
    exec sh -c \
      '{ if '$u' && test -f "$1"; then cp "$1" "$3"; else cp "$2" "$3"; fi; } &&
       { if '$u' && test -f "$4"; then cp "$4" "$5"; else rm -f "$5"; fi; } &&
       { if '$u' && test -f "$6"; then cp "$6" "$7"; else rm -f "$7"; fi; } &&
       { if '$u' && test -f "$8"; then cp "$8" "$9"; else rm -f "$9"; fi; } &&
       shift && shift && shift && shift && shift &&
       shift && shift && shift && shift &&
       exec "${CONFIG_SHELL-/bin/sh}" "$@"' \
      $ignored \
      "$GNULIB_SRCDIR/top/bootstrap" "$GNULIB_SRCDIR/build-aux/bootstrap" \
      "$medir/bootstrap" \
      "$GNULIB_SRCDIR/top/bootstrap-funclib.sh" "$medir/bootstrap-funclib.sh" \
      "$GNULIB_SRCDIR/top/autopull.sh" "$medir/autopull.sh" \
      "$GNULIB_SRCDIR/top/autogen.sh" "$medir/autogen.sh" \
      "$0" "$@" --no-bootstrap-sync
  }
}

# ----------------------------------------------------------------------------

if test x"$gnulib_modules$gnulib_files$gnulib_extra_files" = x; then
  use_gnulib=false
else
  use_gnulib=true
fi

# -------- Fetch auxiliary files from the network.  --------------------------

autopull_usage() {
  cat <<EOF
Usage: $me [OPTION]...
Bootstrap this package from the checked-out sources.

Optional environment variables:
  GNULIB_SRCDIR            Specifies the local directory where gnulib
                           sources reside.  Use this if you already
                           have gnulib sources on your machine, and
                           you want to use these sources.
  GNULIB_REFDIR            Specifies the local directory where a gnulib
                           repository (with a .git subdirectory) resides.
                           Use this if you already have gnulib sources
                           and history on your machine, and do not want
                           to waste your bandwidth downloading them again.
  GNULIB_URL               URL of the gnulib repository.  The default is
                           $default_gnulib_url,
                           which is Gnulib's upstream repository.

Options:
  --bootstrap-sync         if this bootstrap script is not identical to
                           the version in the local gnulib sources,
                           update this script, and then restart it with
                           /bin/sh or the shell \$CONFIG_SHELL
  --no-bootstrap-sync      do not check whether bootstrap is out of sync
  --force                  attempt to bootstrap even if the sources seem
                           not to have been checked out
  --no-git                 do not use git to update gnulib.  Requires that
                           \$GNULIB_SRCDIR or the --gnulib-srcdir option
                           points to a gnulib repository with the correct
                           revision
  --skip-po                do not download po files
EOF
  bootstrap_print_option_usage_hook
  cat <<EOF
If the file bootstrap.conf exists in the same directory as this script, its
contents are read as shell variables to configure the bootstrap.

For build prerequisites, environment variables like \$AUTOCONF and \$AMTAR
are honored.

Gnulib sources can be fetched in various ways:

 * If the environment variable GNULIB_SRCDIR is set (either as an
   environment variable or via the --gnulib-srcdir option), then sources
   are fetched from that local directory.  If it is a git repository and
   the configuration variable GNULIB_REVISION is set in bootstrap.conf,
   then that revision is checked out.

 * Otherwise, if this package is in a git repository with a 'gnulib'
   submodule configured, then that submodule is initialized and updated
   and sources are fetched from there.  If GNULIB_REFDIR is set (either
   as an environment variable or via the --gnulib-refdir option) and is
   a git repository, then it is used as a reference.

 * Otherwise, if the 'gnulib' directory does not exist, Gnulib sources
   are cloned into that directory using git from \$GNULIB_URL, defaulting
   to $default_gnulib_url.
   If the configuration variable GNULIB_REVISION is set in bootstrap.conf,
   then that revision is checked out.

 * Otherwise, the existing Gnulib sources in the 'gnulib' directory are
   used.  If it is a git repository and the configuration variable
   GNULIB_REVISION is set in bootstrap.conf, then that revision is
   checked out.

If you maintain a package and want to pin a particular revision of the
Gnulib sources that has been tested with your package, then there are
two possible approaches: either configure a 'gnulib' submodule with the
appropriate revision, or set GNULIB_REVISION (and if necessary
GNULIB_URL) in bootstrap.conf.

Running without arguments will suffice in most cases.
EOF
}

# Fetch auxiliary files that are omitted from the version control
# repository of this package.
autopull()
{
  # Ensure that CDPATH is not set.  Otherwise, the output from cd
  # would cause trouble in at least one use below.
  (unset CDPATH) >/dev/null 2>&1 && unset CDPATH

  # Parse options.

  # Use git to update gnulib sources
  use_git=true

  for option
  do
    case $option in
    --help)
      autopull_usage
      return;;
    --version)
      set -e
      echo "autopull.sh $scriptlibversion"
      echo "$copyright"
      return 0
      ;;
    --skip-po)
      SKIP_PO=t;;
    --force)
      checkout_only_file=;;
    --bootstrap-sync)
      bootstrap_sync=true;;
    --no-bootstrap-sync)
      bootstrap_sync=false;;
    --no-git)
      use_git=false;;
    *)
      bootstrap_option_hook $option || die "$option: unknown option";;
    esac
  done

  $use_git || test -n "$GNULIB_SRCDIR" \
    || die "Error: --no-git requires \$GNULIB_SRCDIR environment variable" \
           "or --gnulib-srcdir option"
  test -z "$GNULIB_SRCDIR" || test -d "$GNULIB_SRCDIR" \
    || die "Error: \$GNULIB_SRCDIR environment variable" \
           "or --gnulib-srcdir option is specified," \
           "but does not denote a directory"

  if test -n "$checkout_only_file" && test ! -r "$checkout_only_file"; then
    die "Running this script from a non-checked-out distribution is risky."
  fi

  check_build_prerequisites $use_git

  if $use_gnulib || $bootstrap_sync; then
    prepare_GNULIB_SRCDIR
    if $bootstrap_sync; then
      upgrade_bootstrap
    fi
  fi

  # Find sha1sum, named gsha1sum on MacPorts, shasum on Mac OS X 10.6.
  # Also find the compatible sha1 utility on the BSDs
  if test x"$SKIP_PO" = x; then
    find_tool SHA1SUM sha1sum gsha1sum shasum sha1
  fi

  # See if we can use gnulib's git-merge-changelog merge driver.
  if $use_git && test -d .git && check_exists git; then
    if git config merge.merge-changelog.driver >/dev/null ; then
      :
    elif check_exists git-merge-changelog; then
      echo "$0: initializing git-merge-changelog driver"
      git config merge.merge-changelog.name 'GNU-style ChangeLog merge driver'
      git config merge.merge-changelog.driver 'git-merge-changelog %O %A %B'
    else
      echo "$0: consider installing git-merge-changelog from gnulib"
    fi
  fi

  case $SKIP_PO in
  '')
    if test -d po; then
      update_po_files po $package || return
    fi

    if test -d runtime-po; then
      update_po_files runtime-po $package-runtime || return
    fi;;
  esac

  # ---------------------------------------------------------------------------

  bootstrap_post_pull_hook \
    || die "bootstrap_post_pull_hook failed"

  # Don't proceed if there are uninitialized submodules.  In particular,
  # autogen.sh will remove dangling links, which might be links into
  # uninitialized submodules.
  # But it's OK if the 'gnulib' submodule is uninitialized, as long as
  # GNULIB_SRCDIR is set.
  if $use_git; then
    # Uninitialized submodules are listed with an initial dash.
    uninitialized=`git submodule | grep '^-' | awk '{ print $2 }'`
    if test -n "$GNULIB_SRCDIR"; then
      uninitialized=`echo "$uninitialized" | grep -v '^gnulib$'`
    fi
    if test -n "$uninitialized"; then
      uninit_comma=`echo "$uninitialized" | tr '\n' ',' | sed -e 's|,$|.|'`
      die "Some git submodules are not initialized: "$uninit_comma \
          "Either use option '--no-git'," \
          "or run 'git submodule update --init' and bootstrap again."
    fi
  fi

  if test -f "$medir"/autogen.sh; then
    echo "$0: done.  Now you can run '$medir/autogen.sh'."
  fi
}

# ----------------------------- Get translations. -----------------------------

download_po_files() {
  subdir=$1
  domain=$2
  echo "$me: getting translations into $subdir for $domain..."
  cmd=$(printf "$po_download_command_format" "$subdir" "$domain")
  eval "$cmd"
}

# Mirror .po files to $po_dir/.reference and copy only the new
# or modified ones into $po_dir.  Also update $po_dir/LINGUAS.
# Note po files that exist locally only are left in $po_dir but will
# not be included in LINGUAS and hence will not be distributed.
update_po_files() {
  # Directory containing primary .po files.
  # Overwrite them only when we're sure a .po file is new.
  po_dir=$1
  domain=$2

  # Mirror *.po files into this dir.
  # Usually contains *.s1 checksum files.
  ref_po_dir="$po_dir/.reference"

  test -d $ref_po_dir || mkdir $ref_po_dir || return
  download_po_files $ref_po_dir $domain \
    && ls "$ref_po_dir"/*.po 2>/dev/null |
      sed 's|.*/||; s|\.po$||' > "$po_dir/LINGUAS" || return

  langs=$(cd $ref_po_dir && echo *.po | sed 's/\.po//g')
  test "$langs" = '*' && langs=x
  for po in $langs; do
    case $po in x) continue;; esac
    new_po="$ref_po_dir/$po.po"
    cksum_file="$ref_po_dir/$po.s1"
    if ! test -f "$cksum_file" ||
        ! test -f "$po_dir/$po.po" ||
        ! $SHA1SUM -c "$cksum_file" < "$new_po" > /dev/null 2>&1; then
      echo "$me: updated $po_dir/$po.po..."
      cp "$new_po" "$po_dir/$po.po" \
          && $SHA1SUM < "$new_po" > "$cksum_file" || return
    fi
  done
}

# -------- Generate files automatically from existing sources.  --------------

autogen_usage() {
  cat <<EOF
Usage: $me [OPTION]...
Bootstrap this package from the checked-out sources.

Optional environment variables:
  GNULIB_SRCDIR            Specifies the local directory where gnulib
                           sources reside.  Use this if you already
                           have gnulib sources on your machine, and
                           you want to use these sources.

Options:
  --copy                   copy files instead of creating symbolic links
  --force                  attempt to bootstrap even if the sources seem
                           not to have been checked out
EOF
  bootstrap_print_option_usage_hook
  cat <<EOF
If the file bootstrap.conf exists in the same directory as this script, its
contents are read as shell variables to configure the bootstrap.

For build prerequisites, environment variables like \$AUTOCONF and \$AMTAR
are honored.

Gnulib sources are assumed to be present:
  * in \$GNULIB_SRCDIR, if that environment variable is set,
  * otherwise, in the 'gnulib' submodule, if such a submodule is configured,
  * otherwise, in the 'gnulib' subdirectory.

Running without arguments will suffice in most cases.
EOF
}


version_controlled_file() {
  parent=$1
  file=$2
  if test -d .git; then
    git rm -n "$file" > /dev/null 2>&1
  elif test -d .svn; then
    svn log -r HEAD "$file" > /dev/null 2>&1
  elif test -d CVS; then
    grep -F "/${file##*/}/" "$parent/CVS/Entries" 2>/dev/null |
             grep '^/[^/]*/[0-9]' > /dev/null
  else
    warn_ "no version control for $file?"
    false
  fi
}

# Strip blank and comment lines to leave significant entries.
gitignore_entries() {
  sed '/^#/d; /^$/d' "$@"
}

# If $STR is not already on a line by itself in $FILE, insert it at the start.
# Entries are inserted at the start of the ignore list to ensure existing
# entries starting with ! are not overridden.  Such entries support
# whitelisting exceptions after a more generic blacklist pattern.
insert_if_absent() {
  file=$1
  str=$2
  test -f $file || touch $file
  test -r $file || die "Error: failed to read ignore file: $file"
  duplicate_entries=$(gitignore_entries $file | sort | uniq -d)
  if [ "$duplicate_entries" ] ; then
    die "Error: Duplicate entries in $file: " $duplicate_entries
  fi
  linesold=$(gitignore_entries $file | wc -l)
  linesnew=$( { echo "$str"; cat $file; } | gitignore_entries | sort -u | wc -l)
  if [ $linesold != $linesnew ] ; then
    { echo "$str" | cat - $file > $file.bak && mv $file.bak $file; } \
      || die "insert_if_absent $file $str: failed"
  fi
}

# Adjust $PATTERN for $VC_IGNORE_FILE and insert it with
# insert_if_absent.
insert_vc_ignore() {
  vc_ignore_file="$1"
  pattern="$2"
  case $vc_ignore_file in
  *.gitignore)
    # A .gitignore entry that does not start with '/' applies
    # recursively to subdirectories, so prepend '/' to every
    # .gitignore entry.
    pattern=$(echo "$pattern" | sed s,^,/,);;
  esac
  insert_if_absent "$vc_ignore_file" "$pattern"
}

symlink_to_dir()
{
  src=$1/$2
  dst=${3-$2}

  test -f "$src" && {

    # If the destination directory doesn't exist, create it.
    # This is required at least for "lib/uniwidth/cjk.h".
    dst_dir=$(dirname "$dst")
    if ! test -d "$dst_dir"; then
      mkdir -p "$dst_dir"

      # If we've just created a directory like lib/uniwidth,
      # tell version control system(s) it's ignorable.
      # FIXME: for now, this does only one level
      parent=$(dirname "$dst_dir")
      for dot_ig in x $vc_ignore; do
        test $dot_ig = x && continue
        ig=$parent/$dot_ig
        insert_vc_ignore $ig "${dst_dir##*/}/"
      done
    fi

    if $copy; then
      {
        test ! -h "$dst" || {
          echo "$me: rm -f $dst" &&
          rm -f "$dst"
        }
      } &&
      test -f "$dst" &&
      cmp -s "$src" "$dst" || {
        echo "$me: cp -fp $src $dst" &&
        cp -fp "$src" "$dst"
      }
    else
      # Leave any existing symlink alone, if it already points to the source,
      # so that broken build tools that care about symlink times
      # aren't confused into doing unnecessary builds.  Conversely, if the
      # existing symlink's timestamp is older than the source, make it afresh,
      # so that broken tools aren't confused into skipping needed builds.  See
      # <https://lists.gnu.org/r/bug-gnulib/2011-05/msg00326.html>.
      test -h "$dst" &&
      src_ls=$(ls -diL "$src" 2>/dev/null) && set $src_ls && src_i=$1 &&
      dst_ls=$(ls -diL "$dst" 2>/dev/null) && set $dst_ls && dst_i=$1 &&
      test "$src_i" = "$dst_i" &&
      both_ls=$(ls -dt "$src" "$dst") &&
      test "X$both_ls" = "X$dst$nl$src" || {
        dot_dots=
        case $src in
        /*) ;;
        *)
          case /$dst/ in
          *//* | */../* | */./* | /*/*/*/*/*/)
             die "invalid symlink calculation: $src -> $dst";;
          /*/*/*/*/)    dot_dots=../../../;;
          /*/*/*/)      dot_dots=../../;;
          /*/*/)        dot_dots=../;;
          esac;;
        esac

        echo "$me: ln -fs $dot_dots$src $dst" &&
        ln -fs "$dot_dots$src" "$dst"
      }
    fi
  }
}

# Regenerate all autogeneratable files that are omitted from the
# version control repository.  In particular, regenerate all
# aclocal.m4, config.h.in, Makefile.in, configure files with new
# versions of autoconf or automake.
autogen()
{
  # Ensure that CDPATH is not set.  Otherwise, the output from cd
  # would cause trouble in at least one use below.
  (unset CDPATH) >/dev/null 2>&1 && unset CDPATH

  # Environment variables that may be set by the user.
  : "${AUTOPOINT=autopoint}"
  : "${AUTORECONF=autoreconf}"

  if test "$vc_ignore" = auto; then
    vc_ignore=
    test -d .git && vc_ignore=.gitignore
    test -d CVS && vc_ignore="$vc_ignore .cvsignore"
  fi


  # Parse options.

  # Whether to use copies instead of symlinks.
  copy=false

  for option
  do
    case $option in
    --help)
      autogen_usage
      return;;
    --version)
      set -e
      echo "autogen.sh $scriptlibversion"
      echo "$copyright"
      return 0
      ;;
    --force)
      checkout_only_file=;;
    --copy)
      copy=true;;
    *)
      bootstrap_option_hook $option || die "$option: unknown option";;
    esac
  done

  test -z "$GNULIB_SRCDIR" || test -d "$GNULIB_SRCDIR" \
    || die "Error: \$GNULIB_SRCDIR environment variable or --gnulib-srcdir" \
           "option is specified, but does not denote a directory"

  if test -n "$checkout_only_file" && test ! -r "$checkout_only_file"; then
    die "Running this script from a non-checked-out distribution is risky."
  fi

  if $use_gnulib; then
    if test -z "$GNULIB_SRCDIR"; then
      gnulib_path=$(test -f .gitmodules &&
                    git config --file .gitmodules submodule.gnulib.path)
      test -z "$gnulib_path" && gnulib_path=gnulib
      GNULIB_SRCDIR=$gnulib_path
    fi
  fi

  # Die if there is no AC_CONFIG_AUX_DIR($build_aux) line in configure.ac.
  found_aux_dir=no
  grep '^[	 ]*AC_CONFIG_AUX_DIR(\['"$build_aux"'])' configure.ac \
      >/dev/null && found_aux_dir=yes
  grep '^[	 ]*AC_CONFIG_AUX_DIR('"$build_aux"')' configure.ac \
      >/dev/null && found_aux_dir=yes
  test $found_aux_dir = yes \
    || die "configure.ac lacks 'AC_CONFIG_AUX_DIR([$build_aux])'; add it"

  # If $build_aux doesn't exist, create it now, otherwise some bits
  # below will malfunction.  If creating it, also mark it as ignored.
  if test ! -d $build_aux; then
    mkdir $build_aux
    for dot_ig in x $vc_ignore; do
      test $dot_ig = x && continue
      insert_vc_ignore $dot_ig $build_aux/
    done
  fi

  check_build_prerequisites false

  use_libtool=0
  # We'd like to use grep -E, to see if any of LT_INIT,
  # AC_PROG_LIBTOOL, AM_PROG_LIBTOOL is used in configure.ac,
  # but that's not portable enough (e.g., for Solaris).
  grep '^[	 ]*A[CM]_PROG_LIBTOOL' configure.ac >/dev/null \
    && use_libtool=1
  grep '^[	 ]*LT_INIT' configure.ac >/dev/null \
    && use_libtool=1
  if test $use_libtool = 1; then
    find_tool LIBTOOLIZE glibtoolize libtoolize
  fi

  if $use_gnulib; then
    gnulib_tool=$GNULIB_SRCDIR/gnulib-tool
    <$gnulib_tool || return
  fi

  # NOTE: we have to be careful to run both autopoint and libtoolize
  # before gnulib-tool, since gnulib-tool is likely to provide newer
  # versions of files "installed" by these two programs.
  # Then, *after* gnulib-tool (see below), we have to be careful to
  # run autoreconf in such a way that it does not run either of these
  # two just-pre-run programs.

  # Import from gettext.
  with_gettext=yes
  grep '^[	 ]*AM_GNU_GETTEXT_VERSION(' configure.ac >/dev/null || \
      with_gettext=no

  if test $with_gettext = yes || test $use_libtool = 1; then

    tempbase=.bootstrap$$
    trap "rm -f $tempbase.0 $tempbase.1" HUP INT PIPE TERM

    > $tempbase.0 > $tempbase.1 &&
    find . ! -type d -print | sort > $tempbase.0 || return

    if test $with_gettext = yes; then
      # Released autopoint has the tendency to install macros that have been
      # obsoleted in current gnulib, so run this before gnulib-tool.
      echo "$0: $AUTOPOINT --force"
      $AUTOPOINT --force || return
    fi

    # Autoreconf runs aclocal before libtoolize, which causes spurious
    # warnings if the initial aclocal is confused by the libtoolized
    # (or worse out-of-date) macro directory.
    # libtoolize 1.9b added the --install option; but we support back
    # to libtoolize 1.5.22, where the install action was default.
    if test $use_libtool = 1; then
      install=
      case $($LIBTOOLIZE --help) in
        *--install*) install=--install ;;
      esac
      echo "running: $LIBTOOLIZE $install --copy"
      $LIBTOOLIZE $install --copy
    fi

    find . ! -type d -print | sort >$tempbase.1
    old_IFS=$IFS
    IFS=$nl
    for file in $(comm -13 $tempbase.0 $tempbase.1); do
      IFS=$old_IFS
      parent=${file%/*}
      version_controlled_file "$parent" "$file" || {
        for dot_ig in x $vc_ignore; do
          test $dot_ig = x && continue
          ig=$parent/$dot_ig
          insert_vc_ignore "$ig" "${file##*/}"
        done
      }
    done
    IFS=$old_IFS

    rm -f $tempbase.0 $tempbase.1
    trap - HUP INT PIPE TERM
  fi

  # Import from gnulib.

  if $use_gnulib; then
    gnulib_tool_options="\
     --no-changelog\
     --aux-dir=$build_aux\
     --doc-base=$doc_base\
     --lib=$gnulib_name\
     --m4-base=$m4_base/\
     --source-base=$source_base/\
     --tests-base=$tests_base\
     --local-dir=$local_gl_dir\
     $gnulib_tool_option_extras\
    "
    if test $use_libtool = 1; then
      case "$gnulib_tool_options " in
        *' --libtool '*) ;;
        *) gnulib_tool_options="$gnulib_tool_options --libtool" ;;
      esac
    fi
    echo "$0: $gnulib_tool $gnulib_tool_options --import ..."
    $gnulib_tool $gnulib_tool_options --import $gnulib_modules \
      || die "gnulib-tool failed"

    for file in $gnulib_files; do
      symlink_to_dir "$GNULIB_SRCDIR" $file \
        || die "failed to symlink $file"
    done
  fi

  bootstrap_post_import_hook \
    || die "bootstrap_post_import_hook failed"

  # Remove any dangling symlink matching "*.m4" or "*.[ch]" in some
  # gnulib-populated directories.  Such .m4 files would cause aclocal to fail.
  # The following requires GNU find 4.2.3 or newer.  Considering the usual
  # portability constraints of this script, that may seem a very demanding
  # requirement, but it should be ok.  Ignore any failure, which is fine,
  # since this is only a convenience to help developers avoid the relatively
  # unusual case in which a symlinked-to .m4 file is git-removed from gnulib
  # between successive runs of this script.
  find "$m4_base" "$source_base" \
    -depth \( -name '*.m4' -o -name '*.[ch]' \) \
    -type l -xtype l -delete > /dev/null 2>&1

  # Invoke autoreconf with --force --install to ensure upgrades of tools
  # such as ylwrap.
  AUTORECONFFLAGS="--verbose --install --force -I $m4_base $ACLOCAL_FLAGS"
  AUTORECONFFLAGS="$AUTORECONFFLAGS --no-recursive"

  # Tell autoreconf not to invoke autopoint or libtoolize; they were run above.
  echo "running: AUTOPOINT=true LIBTOOLIZE=true $AUTORECONF $AUTORECONFFLAGS"
  AUTOPOINT=true LIBTOOLIZE=true $AUTORECONF $AUTORECONFFLAGS \
    || die "autoreconf failed"

  # Get some extra files from gnulib, overriding existing files.
  for file in $gnulib_extra_files; do
    case $file in
      */INSTALL) dst=INSTALL;;
      build-aux/*) dst=$build_aux/${file#build-aux/};;
      *) dst=$file;;
    esac
    symlink_to_dir "$GNULIB_SRCDIR" $file $dst \
      || die "failed to symlink $file"
  done

  if test $with_gettext = yes; then
    # Create gettext configuration.
    echo "$0: Creating po/Makevars from po/Makevars.template ..."
    rm -f po/Makevars
    sed '
      /^EXTRA_LOCALE_CATEGORIES *=/s/=.*/= '"$EXTRA_LOCALE_CATEGORIES"'/
      /^COPYRIGHT_HOLDER *=/s/=.*/= '"$COPYRIGHT_HOLDER"'/
      /^MSGID_BUGS_ADDRESS *=/s|=.*|= '"$MSGID_BUGS_ADDRESS"'|
      /^XGETTEXT_OPTIONS *=/{
        s/$/ \\/
        a\
            '"$XGETTEXT_OPTIONS"' $${end_of_xgettext_options+}
      }
    ' po/Makevars.template >po/Makevars \
      || die 'cannot generate po/Makevars'

    # If the 'gettext' module is in use, grab the latest Makefile.in.in.
    # If only the 'gettext-h' module is in use, assume autopoint already
    # put the correct version of this file into place.
    case $gnulib_modules in
      *gettext-h*) ;;
      *gettext*)
        cp $GNULIB_SRCDIR/build-aux/po/Makefile.in.in po/Makefile.in.in \
          || die "cannot create po/Makefile.in.in"
        ;;
    esac

    if test -d runtime-po; then
      # Similarly for runtime-po/Makevars, but not quite the same.
      rm -f runtime-po/Makevars
      sed '
        /^DOMAIN *=.*/s/=.*/= '"$package"'-runtime/
        /^subdir *=.*/s/=.*/= runtime-po/
        /^MSGID_BUGS_ADDRESS *=/s/=.*/= bug-'"$package"'@gnu.org/
        /^XGETTEXT_OPTIONS *=/{
          s/$/ \\/
          a\
              '"$XGETTEXT_OPTIONS_RUNTIME"' $${end_of_xgettext_options+}
        }
      ' po/Makevars.template >runtime-po/Makevars \
      || die 'cannot generate runtime-po/Makevars'

      # Copy identical files from po to runtime-po.
      (cd po && cp -p Makefile.in.in *-quot *.header *.sed *.sin ../runtime-po)
    fi
  fi

  bootstrap_epilogue

  echo "$0: done.  Now you can run './configure'."
}

# ----------------------------------------------------------------------------

# Local Variables:
# eval: (add-hook 'before-save-hook 'time-stamp)
# time-stamp-start: "scriptlibversion="
# time-stamp-format: "%:y-%02m-%02d.%02H"
# time-stamp-time-zone: "UTC0"
# time-stamp-end: "; # UTC"
# End:
