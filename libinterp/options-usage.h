////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1993-2021 The Octave Project Developers
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

#if ! defined (octave_options_usage_h)
#define octave_options_usage_h 1

#include "octave-config.h"

#include <iosfwd>

#include "version.h"

// Usage message
static const char *usage_string =
  "octave [-HVWdfhiqvx] [--debug] [--debug-jit] [--doc-cache-file file]\n\
       [--echo-commands] [--eval CODE] [--exec-path path]\n\
       [--gui] [--help] [--image-path path]\n\
       [--info-file file] [--info-program prog] [--interactive]\n\
       [--jit-compiler] [--line-editing] [--no-gui] [--no-history]\n\
       [--no-init-file] [--no-init-path] [--no-line-editing]\n\
       [--no-site-file] [--no-window-system] [--norc] [-p path]\n\
       [--path path] [--persist] [--silent] [--traditional]\n\
       [--verbose] [--version] [file]";

// This is here so that it's more likely that the usage message and
// the real set of options will agree.  Note: the '+' must come first
// to prevent getopt from permuting arguments!
static const char *short_opts = "+HWVdfhip:qvx";


#if defined (HAVE_PRAGMA_GCC_DIAGNOSTIC)
   // Disable warning temporarily.
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

// Long options.  See the comments in getopt.h for the meanings of the
// fields in this structure.
#define BUILT_IN_DOCSTRINGS_FILE_OPTION 1
#define DOC_CACHE_FILE_OPTION 2
#define EVAL_OPTION 3
#define EXEC_PATH_OPTION 4
#define GUI_OPTION 5
#define IMAGE_PATH_OPTION 6
#define INFO_FILE_OPTION 7
#define INFO_PROG_OPTION 8
#define DEBUG_JIT_OPTION 9
#define JIT_COMPILER_OPTION 10
#define LINE_EDITING_OPTION 11
#define NO_GUI_OPTION 12
#define NO_INIT_FILE_OPTION 13
#define NO_INIT_PATH_OPTION 14
#define NO_LINE_EDITING_OPTION 15
#define NO_SITE_FILE_OPTION 16
#define PERSIST_OPTION 17
#define TEXI_MACROS_FILE_OPTION 18
#define TRADITIONAL_OPTION 19
struct octave_getopt_options long_opts[] =
{
  { "braindead",                octave_no_arg,       0, TRADITIONAL_OPTION },
  { "built-in-docstrings-file", octave_required_arg, 0, BUILT_IN_DOCSTRINGS_FILE_OPTION },
  { "debug",                    octave_no_arg,       0, 'd' },
  { "debug-jit",                octave_no_arg,       0, DEBUG_JIT_OPTION },
  { "doc-cache-file",           octave_required_arg, 0, DOC_CACHE_FILE_OPTION },
  { "echo-commands",            octave_no_arg,       0, 'x' },
  { "eval",                     octave_required_arg, 0, EVAL_OPTION },
  { "exec-path",                octave_required_arg, 0, EXEC_PATH_OPTION },
  { "gui",                      octave_no_arg,       0, GUI_OPTION },
  { "help",                     octave_no_arg,       0, 'h' },
  { "image-path",               octave_required_arg, 0, IMAGE_PATH_OPTION },
  { "info-file",                octave_required_arg, 0, INFO_FILE_OPTION },
  { "info-program",             octave_required_arg, 0, INFO_PROG_OPTION },
  { "interactive",              octave_no_arg,       0, 'i' },
  { "jit-compiler",             octave_no_arg,       0, JIT_COMPILER_OPTION },
  { "line-editing",             octave_no_arg,       0, LINE_EDITING_OPTION },
  { "no-gui",                   octave_no_arg,       0, NO_GUI_OPTION },
  { "no-history",               octave_no_arg,       0, 'H' },
  { "no-init-file",             octave_no_arg,       0, NO_INIT_FILE_OPTION },
  { "no-init-path",             octave_no_arg,       0, NO_INIT_PATH_OPTION },
  { "no-line-editing",          octave_no_arg,       0, NO_LINE_EDITING_OPTION },
  { "no-site-file",             octave_no_arg,       0, NO_SITE_FILE_OPTION },
  { "no-window-system",         octave_no_arg,       0, 'W' },
  { "norc",                     octave_no_arg,       0, 'f' },
  { "path",                     octave_required_arg, 0, 'p' },
  { "persist",                  octave_no_arg,       0, PERSIST_OPTION },
  { "quiet",                    octave_no_arg,       0, 'q' },
  { "silent",                   octave_no_arg,       0, 'q' },
  { "texi-macros-file",         octave_required_arg, 0, TEXI_MACROS_FILE_OPTION },
  { "traditional",              octave_no_arg,       0, TRADITIONAL_OPTION },
  { "verbose",                  octave_no_arg,       0, 'V' },
  { "version",                  octave_no_arg,       0, 'v' },
  { 0,                          0,                   0, 0 }
};

#if defined (HAVE_PRAGMA_GCC_DIAGNOSTIC)
   // Restore prevailing warning state for remainder of the file.
#  pragma GCC diagnostic pop
#endif

// Usage message with extra help.

static void
octave_print_verbose_usage_and_exit (void)
{
  std::cout << octave_name_version_copyright_copying_and_warranty ()
            << "\n\
\n\
Usage: octave [options] [FILE]\n\
\n\
Options:\n\
\n\
  --built-in-docstrings-file FILE Use docs for built-ins from FILE.\n\
  --debug, -d             Enter parser debugging mode.\n\
  --debug-jit             Enable JIT compiler debugging/tracing.\n\
  --doc-cache-file FILE   Use doc cache file FILE.\n\
  --echo-commands, -x     Echo commands as they are executed.\n\
  --eval CODE             Evaluate CODE.  Exit when done unless --persist.\n\
  --exec-path PATH        Set path for executing subprograms.\n\
  --gui                   Start the graphical user interface.\n\
  --help, -h,             Print short help message and exit.\n\
  --image-path PATH       Add PATH to head of image search path.\n\
  --info-file FILE        Use top-level info file FILE.\n\
  --info-program PROGRAM  Use PROGRAM for reading info files.\n\
  --interactive, -i       Force interactive behavior.\n\
  --jit-compiler          Enable the JIT compiler.\n\
  --line-editing          Force readline use for command-line editing.\n\
  --no-gui                Disable the graphical user interface.\n\
  --no-history, -H        Don't save commands to the history list\n\
  --no-init-file          Don't read the ~/.octaverc or .octaverc files.\n\
  --no-init-path          Don't initialize function search path.\n\
  --no-line-editing       Don't use readline for command-line editing.\n\
  --no-site-file          Don't read the site-wide octaverc file.\n\
  --no-window-system, -W  Disable window system, including graphics.\n\
  --norc, -f              Don't read any initialization files.\n\
  --path PATH, -p PATH    Add PATH to head of function search path.\n\
  --persist               Go interactive after --eval or reading from FILE.\n\
  --silent, --quiet, -q   Don't print message at startup.\n\
  --texi-macros-file FILE Use Texinfo macros in FILE for makeinfo command.\n\
  --traditional           Set variables for closer MATLAB compatibility.\n\
  --verbose, -V           Enable verbose output in some cases.\n\
  --version, -v           Print version number and exit.\n\
\n\
  FILE                    Execute commands from FILE.  Exit when done\n\
                          unless --persist is also specified.\n\
\n"
            << octave_www_statement ()
            << "\n\n"
            << octave_contrib_statement ()
            << "\n\n"
            << octave_bugs_statement ()
            << "\n";

  exit (0);
}

// Terse usage message.

static void
octave_print_terse_usage_and_exit (void)
{
  std::cerr << "\nusage: " << usage_string << "\n\n";

  exit (1);
}

static void
octave_print_version_and_exit (void)
{
  std::cout << octave_name_version_copyright_copying_warranty_and_bugs ()
            << "\n";
  exit (0);
}

#endif
