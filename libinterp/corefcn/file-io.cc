/*

Copyright (C) 1993-2015 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

// Originally written by John C. Campbell <jcc@bevo.che.wisc.edu>
//
// Thomas Baier <baier@ci.tuwien.ac.at> added the original versions of
// the following functions:
//
//   popen
//   pclose
//   execute       (now popen2.m)
//   sync_system   (now merged with system)
//   async_system  (now merged with system)

// Extensively revised by John W. Eaton <jwe@octave.org>,
// April 1996.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cerrno>
#include <cstdio>

#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_ZLIB_H
#  include <zlib.h>
#endif

#include "error.h"
#include "file-ops.h"
#include "file-stat.h"
#include "lo-ieee.h"
#include "oct-env.h"
#include "oct-locbuf.h"

#include "defun.h"
#include "errwarn.h"
#include "file-io.h"
#include "load-path.h"
#include "oct-fstrm.h"
#include "oct-iostrm.h"
#include "oct-map.h"
#include "oct-prcstrm.h"
#include "oct-stream.h"
#include "oct-strstrm.h"
#include "ov.h"
#include "ovl.h"
#include "pager.h"
#include "sysdep.h"
#include "utils.h"
#include "variables.h"

static octave_value stdin_file;
static octave_value stdout_file;
static octave_value stderr_file;

static octave_stream stdin_stream;
static octave_stream stdout_stream;
static octave_stream stderr_stream;

void
initialize_file_io (void)
{
  stdin_stream = octave_istream::create (&std::cin, "stdin");

  // This uses octave_stdout (see pager.h), not std::cout so that Octave's
  // standard output stream will pass through the pager.

  stdout_stream = octave_ostream::create (&octave_stdout, "stdout");

  stderr_stream = octave_ostream::create (&std::cerr, "stderr");

  stdin_file = octave_stream_list::insert (stdin_stream);
  stdout_file = octave_stream_list::insert (stdout_stream);
  stderr_file = octave_stream_list::insert (stderr_stream);
}

void
close_files (void)
{
  octave_stream_list::clear ();
}

// List of files to delete when we exit or crash.
//
// FIXME: this should really be static,
//        but that causes problems on some systems.
std::stack <std::string> tmp_files;

void
mark_for_deletion (const std::string& file)
{
  tmp_files.push (file);
}

void
cleanup_tmp_files (void)
{
  while (! tmp_files.empty ())
    {
      std::string filename = tmp_files.top ();
      tmp_files.pop ();
      gnulib::unlink (filename.c_str ());
    }
}

static void
normalize_fopen_mode (std::string& mode, bool& use_zlib)
{
  use_zlib = false;

  if (! mode.empty ())
    {
      // Could probably be faster, but does it really matter?

      // Accept 'W', 'R', and 'A' as 'w', 'r', and 'a' but we warn about
      // them because Matlab says they don't perform "automatic
      // flushing" but we don't know precisely what action that implies.

      size_t pos = mode.find ('W');

      if (pos != std::string::npos)
        {
          warning_with_id ("Octave:fopen-mode",
                           "fopen: treating mode \"W\" as equivalent to \"w\"");
          mode[pos] = 'w';
        }

      pos = mode.find ('R');

      if (pos != std::string::npos)
        {
          warning_with_id ("Octave:fopen-mode",
                           "fopen: treating mode \"R\" as equivalent to \"r\"");
          mode[pos] = 'r';
        }

      pos = mode.find ('A');

      if (pos != std::string::npos)
        {
          warning_with_id ("Octave:fopen-mode",
                           "fopen: treating mode \"A\" as equivalent to \"a\"");
          mode[pos] = 'a';
        }

      pos = mode.find ('z');

      if (pos != std::string::npos)
        {
#if defined (HAVE_ZLIB)
          use_zlib = true;
          mode.erase (pos, 1);
#else
          err_disabled_feature ("", "gzipped files (zlib)");
#endif
        }

      // Use binary mode if 't' is not specified, but don't add
      // 'b' if it is already present.

      size_t bpos = mode.find ('b');
      size_t tpos = mode.find ('t');

      if (bpos == std::string::npos && tpos == std::string::npos)
        mode += 'b';
    }
}

static std::ios::openmode
fopen_mode_to_ios_mode (const std::string& mode)
{
  std::ios::openmode retval = std::ios::in;

  if (mode == "rt")
    retval = std::ios::in;
  else if (mode == "wt")
    retval = std::ios::out | std::ios::trunc;
  else if (mode == "at")
    retval = std::ios::out | std::ios::app;
  else if (mode == "r+t" || mode == "rt+")
    retval = std::ios::in | std::ios::out;
  else if (mode == "w+t" || mode == "wt+")
    retval = std::ios::in | std::ios::out | std::ios::trunc;
  else if (mode == "a+t" || mode == "at+")
    retval = std::ios::in | std::ios::out | std::ios::app;
  else if (mode == "rb" || mode == "r")
    retval = std::ios::in | std::ios::binary;
  else if (mode == "wb" || mode == "w")
    retval = std::ios::out | std::ios::trunc | std::ios::binary;
  else if (mode == "ab" || mode == "a")
    retval = std::ios::out | std::ios::app | std::ios::binary;
  else if (mode == "r+b" || mode == "rb+" || mode == "r+")
    retval = std::ios::in | std::ios::out | std::ios::binary;
  else if (mode == "w+b" || mode == "wb+" || mode == "w+")
    retval = (std::ios::in | std::ios::out | std::ios::trunc
              | std::ios::binary);
  else if (mode == "a+b" || mode == "ab+" || mode == "a+")
    retval = (std::ios::in | std::ios::out | std::ios::app
              | std::ios::binary);
  else
    error ("invalid mode specified");

  return retval;
}

DEFUN (fclose, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {} fclose (@var{fid})\n\
@deftypefnx {} {} fclose (\"all\")\n\
@deftypefnx {} {@var{status} =} fclose (\"all\")\n\
Close the file specified by the file descriptor @var{fid}.\n\
\n\
If successful, @code{fclose} returns 0, otherwise, it returns -1.  The\n\
second form of the @code{fclose} call closes all open files except\n\
@code{stdin}, @code{stdout}, @code{stderr}, and any FIDs associated\n\
with gnuplot.\n\
@seealso{fopen, fflush, freport}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  return ovl (octave_stream_list::remove (args(0), "fclose"));
}

DEFUN (fclear, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {} fclear (@var{fid})\n\
Clear the stream state for the file specified by the file descriptor\n\
@var{fid}.\n\
@seealso{ferror, fopen}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  int fid = octave_stream_list::get_file_number (args(0));

  octave_stream os = octave_stream_list::lookup (fid, "fclear");

  os.clearerr ();

  return ovl ();
}

DEFUN (fflush, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {} fflush (@var{fid})\n\
Flush output to file descriptor @var{fid}.\n\
\n\
@code{fflush} returns 0 on success and an OS dependent error value\n\
(@minus{}1 on Unix) on error.\n\
\n\
Programming Note: Flushing is useful for ensuring that all pending output\n\
makes it to the screen before some other event occurs.  For example, it is\n\
always a good idea to flush the standard output stream before calling\n\
@code{input}.\n\
@seealso{fopen, fclose}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  octave_value retval = -1;

  // FIXME: any way to avoid special case for stdout?
  int fid = octave_stream_list::get_file_number (args(0));

  if (fid == 1)
    {
      flush_octave_stdout ();

      retval = 0;
    }
  else
    {
      octave_stream os = octave_stream_list::lookup (fid, "fflush");

      retval = os.flush ();
    }

  return retval;
}

DEFUN (fgetl, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{str} =} fgetl (@var{fid})\n\
@deftypefnx {} {@var{str} =} fgetl (@var{fid}, @var{len})\n\
Read characters from a file, stopping after a newline, or EOF,\n\
or @var{len} characters have been read.\n\
\n\
The characters read, excluding the possible trailing newline, are returned\n\
as a string.\n\
\n\
If @var{len} is omitted, @code{fgetl} reads until the next newline\n\
character.\n\
\n\
If there are no more characters to read, @code{fgetl} returns @minus{}1.\n\
\n\
To read a line and return the terminating newline see @code{fgets}.\n\
@seealso{fgets, fscanf, fread, fopen}\n\
@end deftypefn")
{
  static std::string who = "fgetl";

  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), who);

  octave_value len_arg = (nargin == 2) ? args(1) : octave_value ();

  bool err = false;

  std::string tmp = os.getl (len_arg, err, who);

  if (! err)
    return ovl (tmp, tmp.length ());
  else
    return ovl (-1, 0);
}

DEFUN (fgets, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{str} =} fgets (@var{fid})\n\
@deftypefnx {} {@var{str} =} fgets (@var{fid}, @var{len})\n\
Read characters from a file, stopping after a newline, or EOF,\n\
or @var{len} characters have been read.\n\
\n\
The characters read, including the possible trailing newline, are returned\n\
as a string.\n\
\n\
If @var{len} is omitted, @code{fgets} reads until the next newline\n\
character.\n\
\n\
If there are no more characters to read, @code{fgets} returns @minus{}1.\n\
\n\
To read a line and discard the terminating newline see @code{fgetl}.\n\
@seealso{fputs, fgetl, fscanf, fread, fopen}\n\
@end deftypefn")
{
  static std::string who = "fgets";

  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), who);

  octave_value len_arg = (nargin == 2) ? args(1) : octave_value ();

  bool err = false;

  std::string tmp = os.gets (len_arg, err, who);

  if (! err)
    return ovl (tmp, tmp.length ());
  else
    return ovl (-1.0, 0.0);
}

DEFUN (fskipl, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{nlines} =} fskipl (@var{fid})\n\
@deftypefnx {} {@var{nlines} =} fskipl (@var{fid}, @var{count})\n\
@deftypefnx {} {@var{nlines} =} fskipl (@var{fid}, Inf)\n\
Read and skip @var{count} lines from the file specified by the file\n\
descriptor @var{fid}.\n\
\n\
@code{fskipl} discards characters until an end-of-line is encountered\n\
exactly @var{count}-times, or until the end-of-file marker is found.\n\
\n\
If @var{count} is omitted, it defaults to 1.  @var{count} may also be\n\
@code{Inf}, in which case lines are skipped until the end of the file.\n\
This form is suitable for counting the number of lines in a file.\n\
\n\
Returns the number of lines skipped (end-of-line sequences encountered).\n\
@seealso{fgetl, fgets, fscanf, fopen}\n\
@end deftypefn")
{
  static std::string who = "fskipl";

  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), who);

  octave_value count_arg = (nargin == 2) ? args(1) : octave_value ();

  bool err = false;

  off_t tmp = os.skipl (count_arg, err, who);

  if (! err)
    return ovl (tmp);
  else
    return ovl ();
}


static octave_stream
do_stream_open (const std::string& name, const std::string& mode_arg,
                const std::string& arch, int& fid)
{
  octave_stream retval;

  fid = -1;

  std::string mode = mode_arg;
  bool use_zlib = false;
  normalize_fopen_mode (mode, use_zlib);

  std::ios::openmode md = fopen_mode_to_ios_mode (mode);

  oct_mach_info::float_format flt_fmt =
    oct_mach_info::string_to_float_format (arch);

  std::string fname = file_ops::tilde_expand (name);

  file_stat fs (fname);

  if (! (md & std::ios::out))
    fname = find_data_file_in_load_path ("fopen", fname);

  if (! fs.is_dir ())
    {
#if defined (HAVE_ZLIB)
      if (use_zlib)
        {
          FILE *fptr = gnulib::fopen (fname.c_str (), mode.c_str ());

          int fd = fileno (fptr);

          gzFile gzf = ::gzdopen (fd, mode.c_str ());

          if (fptr)
            retval = octave_zstdiostream::create (fname, gzf, fd,
                                                  md, flt_fmt);
          else
            retval.error (gnulib::strerror (errno));
        }
      else
#endif
        {
          FILE *fptr = gnulib::fopen (fname.c_str (), mode.c_str ());

          retval = octave_stdiostream::create (fname, fptr, md,
                                               flt_fmt);

          if (! fptr)
            retval.error (gnulib::strerror (errno));
        }

    }

  return retval;
}

static octave_stream
do_stream_open (const octave_value& tc_name, const octave_value& tc_mode,
                const octave_value& tc_arch, const char *fcn, int& fid)
{
  octave_stream retval;

  fid = -1;

  std::string name = tc_name.xstring_value ("%s: filename must be a string", fcn);
  std::string mode = tc_mode.xstring_value ("%s: file mode must be a string", fcn);
  std::string arch = tc_arch.xstring_value ("%s: architecture type must be a string", fcn);

  retval = do_stream_open (name, mode, arch, fid);

  return retval;
}

DEFUN (fopen, args, nargout,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{fid} =} fopen (@var{name})\n\
@deftypefnx {} {@var{fid} =} fopen (@var{name}, @var{mode})\n\
@deftypefnx {} {@var{fid} =} fopen (@var{name}, @var{mode}, @var{arch})\n\
@deftypefnx {} {[@var{fid}, @var{msg}] =} fopen (@dots{})\n\
@deftypefnx {} {@var{fid_list} =} fopen (\"all\")\n\
@deftypefnx {} {[@var{file}, @var{mode}, @var{arch}] =} fopen (@var{fid})\n\
Open a file for low-level I/O or query open files and file descriptors.\n\
\n\
The first form of the @code{fopen} function opens the named file with\n\
the specified mode (read-write, read-only, etc.) and architecture\n\
interpretation (IEEE big endian, IEEE little endian, etc.), and returns\n\
an integer value that may be used to refer to the file later.  If an\n\
error occurs, @var{fid} is set to @minus{}1 and @var{msg} contains the\n\
corresponding system error message.  The @var{mode} is a one or two\n\
character string that specifies whether the file is to be opened for\n\
reading, writing, or both.\n\
\n\
The second form of the @code{fopen} function returns a vector of file ids\n\
corresponding to all the currently open files, excluding the\n\
@code{stdin}, @code{stdout}, and @code{stderr} streams.\n\
\n\
The third form of the @code{fopen} function returns information about the\n\
open file given its file id.\n\
\n\
For example,\n\
\n\
@example\n\
myfile = fopen (\"splat.dat\", \"r\", \"ieee-le\");\n\
@end example\n\
\n\
@noindent\n\
opens the file @file{splat.dat} for reading.  If necessary, binary\n\
numeric values will be read assuming they are stored in IEEE format with\n\
the least significant bit first, and then converted to the native\n\
representation.\n\
\n\
Opening a file that is already open simply opens it again and returns a\n\
separate file id.  It is not an error to open a file several times,\n\
though writing to the same file through several different file ids may\n\
produce unexpected results.\n\
\n\
The possible values @samp{mode} may have are\n\
\n\
@table @asis\n\
@item @samp{r} (default)\n\
Open a file for reading.\n\
\n\
@item @samp{w}\n\
Open a file for writing.  The previous contents are discarded.\n\
\n\
@item @samp{a}\n\
Open or create a file for writing at the end of the file.\n\
\n\
@item @samp{r+}\n\
Open an existing file for reading and writing.\n\
\n\
@item @samp{w+}\n\
Open a file for reading or writing.  The previous contents are\n\
discarded.\n\
\n\
@item @samp{a+}\n\
Open or create a file for reading or writing at the end of the\n\
file.\n\
@end table\n\
\n\
Append a @qcode{\"t\"} to the mode string to open the file in text mode or a\n\
@qcode{\"b\"} to open in binary mode.  On Windows and Macintosh systems,\n\
text mode reading and writing automatically converts linefeeds to the\n\
appropriate line end character for the system (carriage-return linefeed on\n\
Windows, carriage-return on Macintosh).  The default when no mode is\n\
specified is binary mode.\n\
\n\
Additionally, you may append a @qcode{\"z\"} to the mode string to open a\n\
gzipped file for reading or writing.  For this to be successful, you\n\
must also open the file in binary mode.\n\
\n\
The parameter @var{arch} is a string specifying the default data format\n\
for the file.  Valid values for @var{arch} are:\n\
\n\
@table @asis\n\
@item @qcode{\"native\"} or @qcode{\"n\"} (default)\n\
The format of the current machine.\n\
\n\
@item @qcode{\"ieee-be\"} or @qcode{\"b\"}\n\
IEEE big endian format.\n\
\n\
@item @qcode{\"ieee-le\"} or @qcode{\"l\"}\n\
IEEE little endian format.\n\
@end table\n\
\n\
@noindent\n\
However, conversions are currently only supported for @samp{native},\n\
@samp{ieee-be}, and @samp{ieee-le} formats.\n\
\n\
When opening a new file that does not yet exist, permissions will be set to\n\
@code{0666 - @var{umask}}.\n\
@seealso{fclose, fgets, fgetl, fscanf, fread, fputs, fdisp, fprintf, fwrite, fskipl, fseek, frewind, ftell, feof, ferror, fclear, fflush, freport, umask}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 3)
    print_usage ();

  octave_value_list retval = ovl (-1.0);

  if (nargin == 1)
    {
      if (args(0).is_string ())
        {
          // If there is only one argument and it is a string but it
          // is not the string "all", we assume it is a file to open
          // with MODE = "r".  To open a file called "all", you have
          // to supply more than one argument.
          if (nargout < 2 && args(0).string_value () == "all")
            return octave_stream_list::open_file_numbers ();
        }
      else
        {
          string_vector tmp = octave_stream_list::get_info (args(0));

          retval = ovl (tmp(0), tmp(1), tmp(2));

          return retval;
        }
    }

  octave_value mode = (nargin == 2 || nargin == 3)
                      ? args(1) : octave_value ("r");

  octave_value arch = (nargin == 3)
                      ? args(2) : octave_value ("native");

  int fid = -1;

  octave_stream os = do_stream_open (args(0), mode, arch, "fopen", fid);

  if (os)
    retval = ovl (octave_stream_list::insert (os), "");
  else
    {
      int error_number = 0;

      retval = ovl (-1.0, os.error (false, error_number));
    }

  return retval;
}

DEFUN (freport, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {} freport ()\n\
Print a list of which files have been opened, and whether they are open\n\
for reading, writing, or both.\n\
\n\
For example:\n\
\n\
@example\n\
@group\n\
freport ()\n\
\n\
     @print{}  number  mode  arch       name\n\
     @print{}  ------  ----  ----       ----\n\
     @print{}     0     r    ieee-le    stdin\n\
     @print{}     1     w    ieee-le    stdout\n\
     @print{}     2     w    ieee-le    stderr\n\
     @print{}     3     r    ieee-le    myfile\n\
@end group\n\
@end example\n\
@seealso{fopen, fclose, is_valid_file_id}\n\
@end deftypefn")
{
  if (args.length () > 0)
    warning ("freport: ignoring extra arguments");

  octave_stdout << octave_stream_list::list_open_files ();

  return ovl ();
}

DEFUN (frewind, args, nargout,
       "-*- texinfo -*-\n\
@deftypefn  {} {} frewind (@var{fid})\n\
@deftypefnx {} {@var{status} =} frewind (@var{fid})\n\
Move the file pointer to the beginning of the file specified by file\n\
descriptor @var{fid}.\n\
\n\
@code{frewind} returns 0 for success, and -1 if an error is encountered.  It\n\
is equivalent to @code{fseek (@var{fid}, 0, SEEK_SET)}.\n\
@seealso{fseek, ftell, fopen}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  int result = -1;

  octave_stream os = octave_stream_list::lookup (args(0), "frewind");

  result = os.rewind ();

  if (nargout > 0)
    return ovl (result);
  else
    return ovl ();
}

DEFUN (fseek, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {} fseek (@var{fid}, @var{offset})\n\
@deftypefnx {} {} fseek (@var{fid}, @var{offset}, @var{origin})\n\
@deftypefnx {} {@var{status} =} fseek (@dots{})\n\
Set the file pointer to the location @var{offset} within the file @var{fid}.\n\
\n\
The pointer is positioned @var{offset} characters from the @var{origin},\n\
which may be one of the predefined variables @w{@code{SEEK_CUR}} (current\n\
position), @w{@code{SEEK_SET}} (beginning), or @w{@code{SEEK_END}} (end of\n\
file) or strings @qcode{\"cof\"}, @qcode{\"bof\"} or @qcode{\"eof\"}.  If\n\
@var{origin} is omitted, @w{@code{SEEK_SET}} is assumed.  @var{offset} may\n\
be positive, negative, or zero but not all combinations of @var{origin} and\n\
@var{offset} can be realized.\n\
\n\
@code{fseek} returns 0 on success and -1 on error.\n\
@seealso{fskipl, frewind, ftell, fopen}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 2 || nargin > 3)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "fseek");

  octave_value origin_arg = (nargin == 3) ? args(2) : octave_value (-1.0);

  return ovl (os.seek (args(1), origin_arg));
}

DEFUN (ftell, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {@var{pos} =} ftell (@var{fid})\n\
Return the position of the file pointer as the number of characters from the\n\
beginning of the file specified by file descriptor @var{fid}.\n\
@seealso{fseek, frewind, feof, fopen}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "ftell");

  return ovl (os.tell ());
}

DEFUN (fprintf, args, nargout,
       "-*- texinfo -*-\n\
@deftypefn  {} {} fprintf (@var{fid}, @var{template}, @dots{})\n\
@deftypefnx {} {} fprintf (@var{template}, @dots{})\n\
@deftypefnx {} {@var{numbytes} =} fprintf (@dots{})\n\
This function is equivalent to @code{printf}, except that the output is\n\
written to the file descriptor @var{fid} instead of @code{stdout}.\n\
\n\
If @var{fid} is omitted, the output is written to @code{stdout} making the\n\
function exactly equivalent to @code{printf}.\n\
\n\
The optional output returns the number of bytes written to the file.\n\
\n\
Implementation Note: For compatibility with @sc{matlab}, escape sequences in\n\
the template string (e.g., @qcode{\"@xbackslashchar{}n\"} => newline) are\n\
expanded even when the template string is defined with single quotes.\n\
@seealso{fputs, fdisp, fwrite, fscanf, printf, sprintf, fopen}\n\
@end deftypefn")
{
  static std::string who = "fprintf";

  int nargin = args.length ();

  if (! (nargin > 1 || (nargin > 0 && args(0).is_string ())))
    print_usage ();

  int result;

  octave_stream os;
  int fmt_n = 0;

  if (args(0).is_string ())
    os = octave_stream_list::lookup (1, who);
  else
    {
      fmt_n = 1;
      os = octave_stream_list::lookup (args(0), who);
    }

  if (! args(fmt_n).is_string ())
    error ("%s: format TEMPLATE must be a string", who.c_str ());

  octave_value_list tmp_args;

  if (nargin > 1 + fmt_n)
    {
      tmp_args.resize (nargin-fmt_n-1, octave_value ());

      for (int i = fmt_n + 1; i < nargin; i++)
        tmp_args(i-fmt_n-1) = args(i);
    }

  result = os.printf (args(fmt_n), tmp_args, who);

  if (nargout > 0)
    return ovl (result);
  else
    return ovl ();
}

DEFUN (printf, args, nargout,
       "-*- texinfo -*-\n\
@deftypefn {} {} printf (@var{template}, @dots{})\n\
Print optional arguments under the control of the template string\n\
@var{template} to the stream @code{stdout} and return the number of\n\
characters printed.\n\
@ifclear OCTAVE_MANUAL\n\
\n\
See the Formatted Output section of the GNU Octave manual for a\n\
complete description of the syntax of the template string.\n\
@end ifclear\n\
\n\
Implementation Note: For compatibility with @sc{matlab}, escape sequences in\n\
the template string (e.g., @qcode{\"@xbackslashchar{}n\"} => newline) are\n\
expanded even when the template string is defined with single quotes.\n\
@seealso{fprintf, sprintf, scanf}\n\
@end deftypefn")
{
  static std::string who = "printf";

  int nargin = args.length ();

  if (nargin == 0)
    print_usage ();

  int result;

  if (! args(0).is_string ())
    error ("%s: format TEMPLATE must be a string", who.c_str ());

  octave_value_list tmp_args;

  if (nargin > 1)
    {
      tmp_args.resize (nargin-1, octave_value ());

      for (int i = 1; i < nargin; i++)
        tmp_args(i-1) = args(i);
    }

  result = stdout_stream.printf (args(0), tmp_args, who);

  if (nargout > 0)
    return ovl (result);
  else
    return ovl ();
}

DEFUN (fputs, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {} fputs (@var{fid}, @var{string})\n\
@deftypefnx {} {@var{status} =} fputs (@var{fid}, @var{string})\n\
Write the string @var{string} to the file with file descriptor @var{fid}.\n\
\n\
The string is written to the file with no additional formatting.  Use\n\
@code{fdisp} instead to automatically append a newline character appropriate\n\
for the local machine.\n\
\n\
Return a non-negative number on success or EOF on error.\n\
@seealso{fdisp, fprintf, fwrite, fopen}\n\
@end deftypefn")
{
  static std::string who = "fputs";

  if (args.length () != 2)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), who);

  return ovl (os.puts (args(1), who));
}

DEFUN (puts, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {} puts (@var{string})\n\
@deftypefnx {} {@var{status} =} puts (@var{string})\n\
Write a string to the standard output with no formatting.\n\
\n\
The string is written verbatim to the standard output.  Use @code{disp} to\n\
automatically append a newline character appropriate for the local machine.\n\
\n\
Return a non-negative number on success and EOF on error.\n\
@seealso{fputs, disp}\n\
@end deftypefn")
{
  static std::string who = "puts";

  if (args.length () != 1)
    print_usage ();

  return ovl (stdout_stream.puts (args(0), who));
}

DEFUN (sprintf, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {} sprintf (@var{template}, @dots{})\n\
This is like @code{printf}, except that the output is returned as a\n\
string.\n\
\n\
Unlike the C library function, which requires you to provide a suitably\n\
sized string as an argument, Octave's @code{sprintf} function returns the\n\
string, automatically sized to hold all of the items converted.\n\
\n\
Implementation Note: For compatibility with @sc{matlab}, escape sequences in\n\
the template string (e.g., @qcode{\"@xbackslashchar{}n\"} => newline) are\n\
expanded even when the template string is defined with single quotes.\n\
@seealso{printf, fprintf, sscanf}\n\
@end deftypefn")
{
  static std::string who = "sprintf";

  int nargin = args.length ();

  if (nargin == 0)
    print_usage ();

  // We don't use octave_ostrstream::create here because need direct
  // access to the OSTR object so that we can extract a string object
  // from it to return.
  octave_ostrstream *ostr = new octave_ostrstream ();

  // The octave_stream destructor will delete OSTR for us.
  octave_stream os (ostr);

  if (! os.is_valid ())
    error ("%s: unable to create output buffer", who.c_str ());

  octave_value fmt_arg = args(0);

  if (! fmt_arg.is_string ())
    error ("%s: format TEMPLATE must be a string", who.c_str ());

  octave_value_list retval (3);

  octave_value_list tmp_args;
  if (nargin > 1)
    {
      tmp_args.resize (nargin-1, octave_value ());

      for (int i = 1; i < nargin; i++)
        tmp_args(i-1) = args(i);
    }

  // NOTE: Call to os.error must precede next call to ostr which might reset it.
  retval(2) = os.printf (fmt_arg, tmp_args, who);
  retval(1) = os.error ();

  std::string result = ostr->str ();
  char type = fmt_arg.is_sq_string () ? '\'' : '"';

  retval(0) = (result.empty () ? octave_value (charMatrix (1, 0), type)
                               : octave_value (result, type));

  return retval;
}

DEFUN (fscanf, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {[@var{val}, @var{count}, @var{errmsg}] =} fscanf (@var{fid}, @var{template}, @var{size})\n\
@deftypefnx {} {[@var{v1}, @var{v2}, @dots{}, @var{count}, @var{errmsg}] =} fscanf (@var{fid}, @var{template}, \"C\")\n\
In the first form, read from @var{fid} according to @var{template},\n\
returning the result in the matrix @var{val}.\n\
\n\
The optional argument @var{size} specifies the amount of data to read\n\
and may be one of\n\
\n\
@table @code\n\
@item Inf\n\
Read as much as possible, returning a column vector.\n\
\n\
@item @var{nr}\n\
Read up to @var{nr} elements, returning a column vector.\n\
\n\
@item [@var{nr}, Inf]\n\
Read as much as possible, returning a matrix with @var{nr} rows.  If the\n\
number of elements read is not an exact multiple of @var{nr}, the last\n\
column is padded with zeros.\n\
\n\
@item [@var{nr}, @var{nc}]\n\
Read up to @code{@var{nr} * @var{nc}} elements, returning a matrix with\n\
@var{nr} rows.  If the number of elements read is not an exact multiple\n\
of @var{nr}, the last column is padded with zeros.\n\
@end table\n\
\n\
@noindent\n\
If @var{size} is omitted, a value of @code{Inf} is assumed.\n\
\n\
A string is returned if @var{template} specifies only character conversions.\n\
\n\
The number of items successfully read is returned in @var{count}.\n\
\n\
If an error occurs, @var{errmsg} contains a system-dependent error message.\n\
\n\
In the second form, read from @var{fid} according to @var{template},\n\
with each conversion specifier in @var{template} corresponding to a\n\
single scalar return value.  This form is more ``C-like'', and also\n\
compatible with previous versions of Octave.  The number of successful\n\
conversions is returned in @var{count}\n\
@ifclear OCTAVE_MANUAL\n\
\n\
See the Formatted Input section of the GNU Octave manual for a\n\
complete description of the syntax of the template string.\n\
@end ifclear\n\
@seealso{fgets, fgetl, fread, scanf, sscanf, fopen}\n\
@end deftypefn")
{
  static std::string who = "fscanf";

  int nargin = args.length ();

  if (nargin < 2 || nargin > 3)
    print_usage ();

  octave_value_list retval;

  octave_stream os = octave_stream_list::lookup (args(0), who);

  if (! args(1).is_string ())
    error ("%s: format TEMPLATE must be a string", who.c_str ());

  if (nargin == 3 && args(2).is_string ())
    {
      retval = ovl (os.oscanf (args(1), who));
    }
  else
    {
      octave_idx_type count = 0;

      Array<double> size = (nargin == 3)
        ? args(2).vector_value ()
        : Array<double> (dim_vector (1, 1),
                         lo_ieee_inf_value ());

      octave_value tmp = os.scanf (args(1), size, count, who);

      retval = ovl (tmp, count, os.error ());
    }

  return retval;
}

static std::string
get_scan_string_data (const octave_value& val, const std::string& who)
{
  std::string retval;

  if (! val.is_string ())
    error ("%s: argument STRING must be a string", who.c_str ());

  octave_value tmp = val.reshape (dim_vector (1, val.numel ()));

  retval = tmp.string_value ();

  return retval;
}

DEFUN (sscanf, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {[@var{val}, @var{count}, @var{errmsg}, @var{pos}] =} sscanf (@var{string}, @var{template}, @var{size})\n\
@deftypefnx {} {[@var{v1}, @var{v2}, @dots{}, @var{count}, @var{errmsg}] =} sscanf (@var{string}, @var{template}, \"C\")\n\
This is like @code{fscanf}, except that the characters are taken from the\n\
string @var{string} instead of from a stream.\n\
\n\
Reaching the end of the string is treated as an end-of-file condition.  In\n\
addition to the values returned by @code{fscanf}, the index of the next\n\
character to be read is returned in @var{pos}.\n\
@seealso{fscanf, scanf, sprintf}\n\
@end deftypefn")
{
  static std::string who = "sscanf";

  int nargin = args.length ();

  if (nargin < 2 || nargin > 3)
    print_usage ();

  octave_value_list retval;

  std::string data = get_scan_string_data (args(0), who);

  octave_stream os = octave_istrstream::create (data);

  if (! os.is_valid ())
    error ("%s: unable to create temporary input buffer", who.c_str ());

  if (! args(1).is_string ())
    error ("%s: format TEMPLATE must be a string", who.c_str ());

  if (nargin == 3 && args(2).is_string ())
    {
      retval = ovl (os.oscanf (args(1), who));
    }
  else
    {
      octave_idx_type count = 0;

      Array<double> size = (nargin == 3) ? args(2).vector_value ()
                                         : Array<double> (dim_vector (1, 1),
                                                          lo_ieee_inf_value ());

      octave_value tmp = os.scanf (args(1), size, count, who);

      // FIXME: is this the right thing to do?
      // Extract error message first, because getting
      // position will clear it.
      std::string errmsg = os.error ();

      retval = ovl (tmp, count, errmsg,
                    (os.eof () ? data.length () : os.tell ()) + 1);
    }

  return retval;
}

DEFUN (scanf, args, nargout,
       "-*- texinfo -*-\n\
@deftypefn  {} {[@var{val}, @var{count}, @var{errmsg}] =} scanf (@var{template}, @var{size})\n\
@deftypefnx {} {[@var{v1}, @var{v2}, @dots{}, @var{count}, @var{errmsg}]] =} scanf (@var{template}, \"C\")\n\
This is equivalent to calling @code{fscanf} with @var{fid} = @code{stdin}.\n\
\n\
It is currently not useful to call @code{scanf} in interactive programs.\n\
@seealso{fscanf, sscanf, printf}\n\
@end deftypefn")
{
  int nargin = args.length ();

  octave_value_list tmp_args (nargin+1, octave_value ());

  tmp_args (0) = 0.0;
  for (int i = 0; i < nargin; i++)
    tmp_args(i+1) = args(i);

  return Ffscanf (tmp_args, nargout);
}

static octave_value_list
textscan_internal (const std::string& who, const octave_value_list& args)
{
  if (args.length () < 1)
    print_usage (who);

  octave_stream os;

  if (args(0).is_string ())
    {
      std::string data = get_scan_string_data (args(0), who);

      os = octave_istrstream::create (data);

      if (! os.is_valid ())
        error ("%s: unable to create temporary input buffer", who.c_str ());
    }
  else
    os =octave_stream_list::lookup (args(0), who);

  int nskip = 1;

  std::string fmt;

  if (args.length () == 1)
    {
      // ommited format = %f.  explicit "" = width from file
      fmt = "%f";
    }
  else if (args(1).is_string ())
    {
      fmt = args(1).string_value ();

      if (args(1).is_sq_string ())
        fmt = do_string_escapes (fmt);

      nskip++;
    }
  else
    error ("%s: FORMAT must be a string", who.c_str ());

  octave_idx_type ntimes = -1;

  if (args.length () > 2)
    {
      if (args(2).is_numeric_type ())
        {
          ntimes = args(2).idx_type_value ();

          if (ntimes < args(2).double_value ())
            error ("%s: REPEAT = %g is too large",
                   who.c_str (), args(2).double_value ());

          nskip++;
        }
    }

  octave_value_list options = args.splice (0, nskip);

  octave_idx_type count = 0;

  octave_value result = os.textscan (fmt, ntimes, options, who, count);

  std::string errmsg = os.error ();

  return ovl (result, count, errmsg);
}

DEFUN (textscan, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{C} =} textscan (@var{fid}, @var{format})\n\
@deftypefnx {} {@var{C} =} textscan (@var{fid}, @var{format}, @var{repeat})\n\
@deftypefnx {} {@var{C} =} textscan (@var{fid}, @var{format}, @var{param}, @var{value}, @dots{})\n\
@deftypefnx {} {@var{C} =} textscan (@var{fid}, @var{format}, @var{repeat}, @var{param}, @var{value}, @dots{})\n\
@deftypefnx {} {@var{C} =} textscan (@var{str}, @dots{})\n\
@deftypefnx {} {[@var{C}, @var{position}, @var{errmsg}] =} textscan (@dots{})\n\
Read data from a text file or string.\n\
\n\
The string @var{str} or file associated with @var{fid} is read from and\n\
parsed according to @var{format}.  The function is an extension of\n\
@code{strread} and @code{textread}.  Differences include: the ability to\n\
read from either a file or a string, additional options, and additional\n\
format specifiers.\n\
\n\
The input is interpreted as a sequence of words, delimiters (such as\n\
whitespace), and literals.  The characters that form delimiters and\n\
whitespace are determined by the options.  The format consists of format\n\
specifiers interspersed between literals.  In the format, whitespace forms\n\
a delimiter between consecutive literals, but is otherwise ignored.\n\
\n\
The output @var{C} is a cell array where the number of columns is determined\n\
by the number of format specifiers.\n\
\n\
The first word of the input is matched to the first specifier of the format\n\
and placed in the first column of the output; the second is matched to the\n\
second specifier and placed in the second column and so forth.  If there\n\
are more words than specifiers then the process is repeated until all words\n\
have been processed or the limit imposed by @var{repeat} has been met (see\n\
below).\n\
\n\
The string @var{format} describes how the words in @var{str} should be\n\
parsed.  As in @var{fscanf}, any (non-whitespace) text in the format that is\n\
not one of these specifiers is considered a literal.  If there is a literal\n\
between two format specifiers then that same literal must appear in the\n\
input stream between the matching words.\n\
\n\
The following specifiers are valid:\n\
\n\
@table @code\n\
@item  %f\n\
@itemx %f64\n\
@itemx %n\n\
The word is parsed as a number and converted to double.\n\
\n\
@item  %f32\n\
The word is parsed as a number and converted to single (float).\n\
\n\
@item  %d\n\
@itemx %d8\n\
@itemx %d16\n\
@itemx %d32\n\
@itemx %d64\n\
The word is parsed as a number and converted to int8, int16, int32, or\n\
int64.  If no size is specified then int32 is used.\n\
\n\
@item  %u\n\
@itemx %u8\n\
@itemx %u16\n\
@itemx %u32\n\
@itemx %u64\n\
The word is parsed as a number and converted to uint8, uint16, uint32, or\n\
uint64.  If no size is specified then uint32 is used.\n\
\n\
@item %s\n\
The word is parsed as a string ending at the last character before\n\
whitespace, an end-of-line, or a delimiter specified in the options.\n\
\n\
@item %q\n\
The word is parsed as a \"quoted string\".\n\
If the first character of the string is a double quote (\") then the string\n\
includes everything until a matching double quote---including whitespace,\n\
delimiters, and end-of-line characters.  If a pair of consecutive double\n\
quotes appears in the input, it is replaced in the output by a single\n\
double quote.  For examples, the input \"He said \"\"Hello\"\"\" would\n\
return the value 'He said \"Hello\"'.\n\
\n\
@item  %c\n\
The next character of the input is read.\n\
This includes delimiters, whitespace, and end-of-line characters.\n\
\n\
@item  %[@dots{}]\n\
@itemx %[^@dots{}]\n\
In the first form, the word consists of the longest run consisting of only\n\
characters between the brackets.  Ranges of characters can be specified by\n\
a hyphen; for example, %[0-9a-zA-Z] matches all alphanumeric characters (if\n\
the underlying character set is ASCII).  Since @sc{matlab} treats hyphens\n\
literally, this expansion only applies to alphanumeric characters.  To\n\
include '-' in the set, it should appear first or last in the brackets; to\n\
include ']', it should be the first character.  If the first character is\n\
'^' then the word consists of characters @strong{not} listed.\n\
\n\
@item %N@dots{}\n\
For %s, %c %d, %f, %n, %u, an optional width can be specified as %Ns, etc.\n\
where N is an integer > 1.  For %c, this causes exactly N characters to be\n\
read instead of a single character.  For the other specifiers, it is an\n\
upper bound on the number of characters read; normal delimiters can cause\n\
fewer characters to be read.  For complex numbers, this limit applies to\n\
the real and imaginary components individually.  For %f and %n, format\n\
specifiers like %N.Mf are allowed, where M is an upper bound on number of\n\
characters after the decimal point to be considered; subsequent digits are\n\
skipped.  For example, the specifier %8.2f would read 12.345e6 as 1.234e7.\n\
\n\
@item %*@dots{}\n\
The word specified by the remainder of the conversion specifier is skipped.\n\
\n\
@item literals\n\
In addition the format may contain literal character strings; these will be\n\
skipped during reading.  If the input string does not match this literal,\n\
the processing terminates.\n\
@end table\n\
\n\
Parsed words corresponding to the first specifier are returned in the first\n\
output argument and likewise for the rest of the specifiers.\n\
\n\
By default, if there is only one input argument, @var{format} is @t{\"%f\"}.\n\
This means that numbers are read from the input into a single column vector.\n\
If @var{format} is explicitly empty (@qcode{\"\"}) then textscan will\n\
return data in a number of columns matching the number of fields on the\n\
first data line of the input.  Either of these is suitable only when the\n\
input is exclusively numeric.\n\
\n\
For example, the string\n\
\n\
@smallexample\n\
@group\n\
@var{str} = \"\\\n\
Bunny Bugs   5.5\\n\\\n\
Duck Daffy  -7.5e-5\\n\\\n\
Penguin Tux   6\"\n\
@end group\n\
@end smallexample\n\
\n\
@noindent\n\
can be read using\n\
\n\
@example\n\
@var{a} = textscan (@var{str}, \"%s %s %f\");\n\
@end example\n\
\n\
The optional numeric argument @var{repeat} can be used for limiting the\n\
number of items read:\n\
\n\
@table @asis\n\
@item -1\n\
Read all of the string or file until the end (default).\n\
\n\
@item N\n\
Read until the first of two conditions occurs: 1) the format has been\n\
processed N times, or 2) N lines of the input have been processed.  Zero\n\
(0) is an acceptable value for @var{repeat}.  Currently, end-of-line\n\
characters inside %q, %c, and %[@dots{}]$ conversions do not contribute to\n\
the line count.  This is incompatible with @sc{matlab} and may change in\n\
future.\n\
@end table\n\
\n\
The behavior of @code{textscan} can be changed via property/value pairs.\n\
The following properties are recognized:\n\
\n\
@table @asis\n\
@item @qcode{\"BufSize\"}\n\
This specifies the number of bytes to use for the internal buffer.\n\
A modest speed improvement may be obtained by setting this to a large value\n\
when reading a large file, especially if the input contains long strings.\n\
The default is 4096, or a value dependent on @var{n} if that is specified.\n\
\n\
@item @qcode{\"CollectOutput\"}\n\
A value of 1 or true instructs @code{textscan} to concatenate consecutive\n\
columns of the same class in the output cell array.  A value of 0 or false\n\
(default) leaves output in distinct columns.\n\
\n\
@item @qcode{\"CommentStyle\"}\n\
Specify parts of the input which are considered comments and will be\n\
skipped.  @var{value} is the comment style and can be either (1) A string\n\
or 1x1 cell string, to skip everything to the right of it; (2) A cell array\n\
of two strings, to skip everything between the first and second strings.  \n\
Comments are only parsed where whitespace is accepted and do not act as\n\
delimiters.\n\
\n\
@item @qcode{\"Delimiter\"}\n\
If @var{value} is a string, any character in @var{value} will be used to\n\
split the input into words.  If @var{value} is a cell array of strings,\n\
any string in the array will be used to split the input into words.\n\
(default value = any whitespace.)\n\
\n\
@item @qcode{\"EmptyValue\"}\n\
Value to return for empty numeric values in non-whitespace delimited data.\n\
The default is NaN@.  When the data type does not support NaN (int32 for\n\
example), then the default is zero.\n\
\n\
@item @qcode{\"EndOfLine\"}\n\
@var{value} can be either an emtpy or one character specifying the\n\
end-of-line character, or the pair\n\
@qcode{\"@xbackslashchar{}r@xbackslashchar{}n\"} (CRLF).\n\
In the latter case, any of\n\
@qcode{\"@xbackslashchar{}r\"}, @qcode{\"@xbackslashchar{}n\"} or\n\
@qcode{\"@xbackslashchar{}r@xbackslashchar{}n\"} is counted as a (single)\n\
newline.  If no value is given,\n\
@qcode{\"@xbackslashchar{}r@xbackslashchar{}n\"} is used.\n\
@c If set to \"\" (empty string) EOLs are ignored as delimiters and added\n\
@c to whitespace.\n\
\n\
@c When reading from a character string, optional input argument @var{n}\n\
@c specifies the number of times @var{format} should be used (i.e., to limit\n\
@c the amount of data read).\n\
@c When reading from file, @var{n} specifies the number of data lines to read;\n\
@c in this sense it differs slightly from the format repeat count in strread.\n\
\n\
@item @qcode{\"HeaderLines\"}\n\
The first @var{value} number of lines of @var{fid} are skipped.  Note that\n\
this does not refer to the first non-comment lines, but the first lines of\n\
any type.\n\
\n\
@item @qcode{\"MultipleDelimsAsOne\"}\n\
If @var{value} is nonzero, treat a series of consecutive delimiters,\n\
without whitespace in between, as a single delimiter.  Consecutive\n\
delimiter series need not be vertically aligned.  Without this option, a\n\
single delimiter before the end of the line does not cause the line to be\n\
considered to end with an empty value, but a single delimiter at the start\n\
of a line causes the line to be considered to start with an empty value.\n\
\n\
@item @qcode{\"TreatAsEmpty\"}\n\
Treat single occurrences (surrounded by delimiters or whitespace) of the\n\
string(s) in @var{value} as missing values.\n\
\n\
@item @qcode{\"ReturnOnError\"}\n\
If set to numerical 1 or true, return normally as soon as an error is\n\
encountered, such as trying to read a string using @qcode{%f}.\n\
If set to 0 or false, return an error and no data.\n\
\n\
@item @qcode{\"Whitespace\"}\n\
Any character in @var{value} will be interpreted as whitespace and trimmed;\n\
The default value for whitespace is\n\
@c Note: the next line specifically has a newline which generates a space\n\
@c       in the output of qcode, but keeps the next line < 80 characters.\n\
@qcode{\"\n\
@xbackslashchar{}b@xbackslashchar{}r@xbackslashchar{}n@xbackslashchar{}t\"}\n\
(note the space).  Unless whitespace is set to @qcode{\"\"} (empty) AND at\n\
least one @qcode{\"%s\"} format conversion specifier is supplied, a space is\n\
always part of whitespace.\n\
\n\
@end table\n\
\n\
When the number of words in @var{str} or @var{fid} doesn't match an exact\n\
multiple of the number of format conversion specifiers, @code{textscan}'s\n\
behavior depends on whether the last character of the string or file is an\n\
end-of-line as specified by the @code{EndOfLine} option:\n\
\n\
@table @asis\n\
@item last character = end-of-line\n\
Data columns are padded with empty fields, NaN or 0 (for integer fields) so\n\
that all columns have equal length\n\
\n\
@item last character is not end-of-line\n\
Data columns are not padded; @code{textscan} returns columns of unequal\n\
length\n\
@end table\n\
\n\
\n\
The second output @var{position} provides the location, in characters\n\
from the beginning of the file or string, where processing stopped.\n\
\n\
@seealso{dlmread, fscanf, load, strread, textread}\n\
@end deftypefn")
{
  static std::string who = "textscan";

  return textscan_internal (who, args);
}

DEFUN (__textscan__, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {@var{C} =} __textscan__ (@var{who}, @dots{})\n\
Like @code{textscan} but accept additional argument @var{who} to use\n\
as the name of the function when reporting errors.\n\
@end deftypefn")
{
  if (args.length () == 0)
    print_usage ();

  return textscan_internal (args(0).string_value (), args.splice (0, 1));
}

/*
%!test
%! str = "1,  2,  3,  4\n 5,  ,  ,  8\n 9, 10, 11, 12";
%! fmtstr = "%f %d %f %s";
%! c = textscan (str, fmtstr, 2, "delimiter", ",", "emptyvalue", -Inf);
%! assert (c{1}, [1;5]);
%! assert (c{3}, [3; -Inf]);
%! assert (iscellstr (c{4}));

%!test
%! b = [10:10:100];
%! b = [b; 8*b/5];
%! str = sprintf ("%g miles/hr = %g kilometers/hr\n", b);
%! fmt = "%f miles/hr = %f kilometers/hr";
%! c = textscan (str, fmt);
%! assert (c{1}, b(1,:)', 1e-5);
%! assert (c{2}, b(2,:)', 1e-5);

%!test
%! str = "13, -, NA, str1, -25\r\n// Middle line\r\n36, na, 05, str3, 6";
%! c = textscan (str, "%d %n %f %s %n", "delimiter", ",",
%!                    "treatAsEmpty", {"NA", "na", "-"}, "commentStyle", "//");
%! assert (c{1}, int32 ([13; 36]));
%! assert (c{2}, [NaN; NaN]);
%! assert (c{3}, [NaN; 5]);
%! assert (c{4}, {"str1"; "str3"});
%! assert (c{5}, [-25; 6]);

%!test
%! str = "Km:10 = hhhBjjj miles16hour\r\n";
%! str = [str "Km:15 = hhhJjjj miles241hour\r\n"];
%! str = [str "Km:2 = hhhRjjj miles3hour\r\n"];
%! str = [str "Km:25 = hhhZ\r\n"];
%! fmt = "Km:%d = hhh%1sjjj miles%dhour";
%! c = textscan (str, fmt, "delimiter", " ");
%! assert (c{1}', int32 ([10, 15, 2, 25]));
%! assert (c{2}', {'B' 'J' 'R' 'Z'});
%! assert (c{3}', int32 ([16, 241, 3, 0]));

## Test with default EndOfLine parameter
%!test
%! c = textscan ("L1\nL2", "%s");
%! assert (c{:}, {"L1"; "L2"});

## Test with EndofLine parameter set to "" (empty) - newline should be in word
%!test
%! c = textscan ("L1\nL2", "%s", "endofline", "");
%! assert (int8 ([c{:}{:}]), int8 ([76, 49, 10, 76, 50]));

###  Matlab fails this test.  A literal after a conversion is not a delimiter
#%!test
#%! ## No delimiters at all besides EOL.  Skip fields, even empty fields
#%! str = "Text1Text2Text\nTextText4Text\nText57Text";
#%! c = textscan (str, "Text%*dText%dText");
#%! assert (c{1}, int32 ([2; 4; 0]));

## CollectOutput test
%!test
%! b = [10:10:100];
%! b = [b; 8*b/5; 8*b*1000/5];
%! str = sprintf ("%g miles/hr = %g (%g) kilometers (meters)/hr\n", b);
%! fmt = "%f miles%s %s %f (%f) kilometers %*s";
%! c = textscan (str, fmt, "collectoutput", 1);
%! assert (size (c{3}), [10, 2]);
%! assert (size (c{2}), [10, 2]);

## CollectOutput test with uneven column length files
%!test
%! b = [10:10:100];
%! b = [b; 8*b/5; 8*b*1000/5];
%! str = sprintf ("%g miles/hr = %g (%g) kilometers (meters)/hr\n", b);
%! str = [str "110 miles/hr"];
%! fmt = "%f miles%s %s %f (%f) kilometers %*s";
%! c = textscan (str, fmt, "collectoutput", 1);
%! assert (size (c{1}), [11, 1]);
%! assert (size (c{3}), [11, 2]);
%! assert (size (c{2}), [11, 2]);
%! assert (c{3}(end), NaN);
%! assert (c{2}{11, 1}, "/hr");
%! assert (isempty (c{2}{11, 2}), true);

## Double quoted string
%!test
%! str = 'First    "the second called ""the middle""" third';
%! fmt = "%q";
%! c = textscan (str, fmt);
%! assert (c{1}, {"First"; 'the second called "the middle"'; "third"});

## Arbitrary character
%!test
%! c = textscan ("a first, \n second, third", "%s %c %11c", "delimiter", " ,");
%! assert (c{1}, {"a"; "ond"});
%! assert (c{2}, {"f"; "t"});
%! assert (c{3}, {"irst, \n sec"; "hird"});

## Field width and non-standard delimiters
%!test
%! str = "12;34;123456789;7";
%! c = textscan (str, "%4d %4d", "delimiter", ";", "collectOutput", 1);
%! assert (c, {[12, 34; 1234, 5678; 9, 7]});

## Field width and non-standard delimiters (2)
%!test
%! str = "12;34;123456789;7";
%! c = textscan (str, "%4f %f", "delimiter", ";", "collectOutput", 1);
%! assert (c, {[12, 34; 1234, 56789; 7, NaN]});

## Ignore trailing delimiter, but use leading one
%!test
%! str = "12.234e+2,34, \n12345.789-9876j,78\n,10|3";
%! c = textscan (str, "%10.2f %f", "delimiter", ",", "collectOutput", 1,
%!                    "expChars", "e|");
%! assert (c, {[1223, 34; 12345.79-9876j, 78; NaN, 10000]}, 1e-6);

## Multi-character delimiter
%!test
%! str = "99end2 space88gap 4564";
%! c = textscan (str, "%d %s", "delimiter", {"end", "gap", "space"});
%! assert (c{1}, int32 ([99; 88]));
%! assert (c{2}, {"2 "; "4564"});

## FIXME: Following two tests still fail (4/13/2016)?
### Delimiters as part of literals, and following literals
#%!test
#%! str = "12 R&D & 7";
#%! c = textscan (str, "%f R&D %f", "delimiter", "&", "collectOutput", 1,
#%!                    "EmptyValue", -99);
#%! assert (c, {[12, -99; 7, -99]});
#
### Delimiters as part of literals, and before literals
#%!test
#%! str = "12 & R&D 7";
#%! c = textscan (str, "%f R&D %f", "delimiter", "&", "collectOutput", 1);
#%! assert (c, {[12 7]});

## Check number of lines read, not number of passes through format string
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, "1\n2\n3\n4\n5\n6");
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "%f %f", 2);
%! E = feof (fid);
%! fclose (fid);
%! unlink (f);
%! assert (c, {1, 2});
%! assert (! E);

## Check number of lines read, not number of passes through format string
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, "1\r\n2\r3\n4\r\n5\n6");
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "%f %f", 4);
%! fclose (fid);
%! unlink (f);
%! assert (c, {[1;3], [2;4]});

## Check number of lines read, with multiple delimiters
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, "1-\r\n-2\r3-\n-4\r\n5\n6");
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "%f %f", 4, "delimiter", "-", "multipleDelimsAsOne", 1);
%! fclose (fid);
%! unlink (f);
%! assert (c, {[1;3], [2;4]});

## Check ReturnOnError
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! str = "1 2 3\n4 s 6";
%! fprintf (fid, str);
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "%f %f %f", "ReturnOnError", 1);
%! fseek (fid, 0, "bof");
%! fclose (fid);
%! unlink (f);
%! u = textscan (str, "%f %f %f", "ReturnOnError", 1);
%! assert (c, {[1;4], [2], [3]});
%! assert (u, {[1;4], [2], [3]});

%! ## Check ReturnOnError (2)
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! str = "1 2 3\n4 s 6\n";
%! fprintf (fid, str);
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "%f %f %f", "ReturnOnError", 1);
%! fseek (fid, 0, "bof");
%! fclose (fid);
%! unlink (f);
%! u = textscan (str, "%f %f %f", "ReturnOnError", 1);
%! assert (c, {[1;4], 2, 3});
%! assert (u, {[1;4], 2, 3});

%!error <Read error in field 2 of row 2>
%! textscan ("1 2 3\n4 s 6", "%f %f %f", "ReturnOnError", 0);

## Check ReturnOnError (3)
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, "1 s 3\n4 5 6");
%! fseek (fid, 0, "bof");
%! c = textscan (fid, "", "ReturnOnError", 1);
%! fseek (fid, 0, "bof");
%! fclose (fid);
%! unlink (f);
%! assert (c, {1});

## Check ReturnOnError with empty fields
%!test
%! c = textscan ("1,,3\n4,5,6", "", "Delimiter", ",", "ReturnOnError", 1);
%! assert (c, {[1;4], [NaN;5], [3;6]});

## Check ReturnOnError with empty fields (2)
%!test
%! c = textscan ("1,,3\n4,5,6", "%f %f %f", "Delimiter", ",",
%!               "ReturnOnError", 1);
%! assert (c, {[1;4], [NaN;5], [3;6]});

## Check ReturnOnError in first column
%!test
%! c = textscan ("1 2 3\ns 5 6", "", "ReturnOnError", 1);
%! assert (c, {1, 2, 3});

## FIXME: This test fails (4/14/16)
### Test incomplete first data line
#%!test
#%! R = textscan (['Empty1' char(10)], 'Empty%d %f');
#%! assert (R{1}, int32 (1));
#%! assert (isempty (R{2}), true);

## bug #37023
%!test
%! data = textscan ("   1. 1 \n 2 3\n", '%f %f');
%! assert (data{1}, [1; 2], 1e-15);
%! assert (data{2}, [1; 3], 1e-15);

## Whitespace test (bug #37333) using delimiter ";"
%!test
%! tc{1, 1} = "C:/code;";
%! tc{1, end+1} = "C:/code/meas;";
%! tc{1, end+1} = " C:/code/sim;";
%! tc{1, end+1} = "C:/code/utils;";
%! string = [tc{:}];
%! c = textscan (string, "%s", "delimiter", ";");
%! for k = 1:max (numel (c{1}), numel (tc))
%!   lh = c{1}{k};
%!   rh = tc{k};
%!   rh(rh == ";") = "";
%!   rh = strtrim (rh);
%!   assert (strcmp (lh, rh));
%! endfor

## Whitespace test (bug #37333), adding multipleDelimsAsOne true arg
%!test
%! tc{1, 1} = "C:/code;";
%! tc{1, end+1} = " C:/code/meas;";
%! tc{1, end+1} = "C:/code/sim;;";
%! tc{1, end+1} = "C:/code/utils;";
%! string = [tc{:}];
%! c = textscan (string, "%s", "delimiter", ";", "multipleDelimsAsOne", 1);
%! for k = 1:max (numel (c{1}), numel (tc))
%!   lh = c{1}{k};
%!   rh = tc{k};
%!   rh(rh == ";") = "";
%!   rh = strtrim (rh);
%!   assert (strcmp (lh, rh));
%! endfor

## Whitespace test (bug #37333), adding multipleDelimsAsOne false arg
%!test
%! tc{1, 1} = "C:/code;";
%! tc{1, end+1} = " C:/code/meas;";
%! tc{1, end+1} = "C:/code/sim;;";
%! tc{1, end+1} = "";
%! tc{1, end+1} = "C:/code/utils;";
%! string = [tc{:}];
%! c = textscan (string, "%s", "delimiter", ";", "multipleDelimsAsOne", 0);
%! for k = 1:max (numel (c{1}), numel (tc))
%!   lh = c{1}{k};
%!   rh = tc{k};
%!   rh(rh == ";") = "";
%!   rh = strtrim (rh);
%!   assert (strcmp (lh, rh));
%! endfor

## Whitespace test (bug #37333) whitespace "" arg
%!test
%! tc{1, 1} = "C:/code;";
%! tc{1, end+1} = " C:/code/meas;";
%! tc{1, end+1} = "C:/code/sim;";
%! tc{1, end+1} = "C:/code/utils;";
%! string = [tc{:}];
%! c = textscan (string, "%s", "delimiter", ";", "whitespace", "");
%! for k = 1:max (numel (c{1}), numel (tc))
%!   lh = c{1}{k};
%!   rh = tc{k};
%!   rh(rh == ";") = "";
%!   assert (strcmp (lh, rh));
%! endfor

## Whitespace test (bug #37333), whitespace " " arg
%!test
%! tc{1, 1} = "C:/code;";
%! tc{1, end+1} = " C:/code/meas;";
%! tc{1, end+1} = "C:/code/sim;";
%! tc{1, end+1} = "C:/code/utils;";
%! string = [tc{:}];
%! c = textscan (string, "%s", "delimiter", ";", "whitespace", " ");
%! for k = 1:max (numel (c{1}), numel (tc))
%!   lh = c{1}{k};
%!   rh = tc{k};
%!   rh(rh == ";") = "";
%!   rh = strtrim (rh);
%!   assert (strcmp (lh, rh));
%! endfor

## Tests reading with empty format, should return proper nr of columns
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, " 1 2 3 4\n5 6 7 8");
%! fseek (fid, 0, "bof");
%! C = textscan (fid, "");
%! E = feof (fid);
%! fclose (fid);
%! unlink (f);
%! assert (C{1}, [1 ; 5], 1e-6);
%! assert (C{2}, [2 ; 6], 1e-6);
%! assert (C{3}, [3 ; 7], 1e-6);
%! assert (C{4}, [4 ; 8], 1e-6);
%! assert (E);

## Test leaving the file at the correct position on exit
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, "1,2\n3,4\n");
%! fseek (fid, 0, "bof");
%! C = textscan (fid, "%s %f", 2, "Delimiter", ",");
%! E = ftell (fid);
%! fclose (fid);
%! unlink (f);
%! assert (E, 8);

## Tests reading with empty format; empty fields & incomplete lower row
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid, " ,2,,4\n5,6");
%! fseek (fid, 0, "bof");
%! C = textscan (fid, "", "delimiter", ",", "EmptyValue", 999,
%!                    "CollectOutput" , 1);
%! fclose (fid);
%! unlink (f);
%! assert (C{1}, [999, 2, 999, 4; 5, 6, 999, 999], 1e-6);

## Error message tests

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! msg1 = "textscan: 1 parameters given, but only 0 values";
%! try
%!   C = textscan (fid, "", "headerlines");
%! end_try_catch;
%! assert (!feof (fid));
%! fclose (fid);
%! unlink (f);
%! assert (msg1, lasterr);

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! msg1 = "textscan: HeaderLines must be numeric";
%! try
%!   C = textscan (fid, "", "headerlines", "hh");
%! end_try_catch;
%! fclose (fid);
%! unlink (f);
%! assert (msg1, lasterr);

## Skip headerlines
%!test
%! C = textscan ("field 1  field2\n 1 2\n3 4", "", "headerlines", 1,
%!               "collectOutput", 1);
%! assert (C, {[1 2; 3 4]});

## Skip headerlines with non-default EOL
%!test
%! C = textscan ("field 1  field2\r 1 2\r3 4", "", "headerlines", 2,
%!               "collectOutput", 1, "EndOfLine", '\r');
%! assert (C, {[3 4]});

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid,"some_string");
%! fseek (fid, 0, "bof");
%! msg1 = "textscan: EndOfLine must be at most one character or '\\r\\n'";
%! try
%!   C = textscan (fid, "%f", "EndOfLine", "\n\r");
%! end_try_catch;
%! fclose (fid);
%! unlink (f);
%! assert (msg1, lasterr);

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid,"some_string");
%! fseek (fid, 0, "bof");
%! msg1 = "textscan: EndOfLine must be at most one character or '\\r\\n'";
%! try
%!   C = textscan (fid, "%f", "EndOfLine", 33);
%! end_try_catch;
%! fclose (fid);
%! unlink (f);
%! assert (msg1, lasterr);

## Bug #41824
%!assert (textscan ("123", "", "whitespace", " "){:}, 123);

## Bug #42343-1, just test supplied emptyvalue
%!assert (textscan (",NaN", "", "delimiter", "," ,"emptyValue" ,Inf),
%!        {Inf, NaN})

## Bug #42343-2, test padding with supplied emptyvalue
%!test
%! c = textscan (",1,,4\nInf,  ,NaN\n", "", "delimiter", ",",
%!               "emptyvalue", -10);
%! assert (cell2mat (c), [-10, 1, -10, 4; Inf, -10, NaN, -10]);

## Bug #42528
%!test
%! assert (textscan ("1i", ""){1},  0+1i);
%! C = textscan ("3, 2-4i, NaN\n -i, 1, 23.4+2.2i\n 1+1 1+1j", "",
%!               "delimiter", ",");
%! assert (cell2mat (C), [3+0i, 2-4i, NaN+0i; 0-i,  1+0i, 23.4+2.2i; 1 1 1+1i]);

%!test
%! ## TreatAsEmpty
%! C = textscan ("1,2,3,NN,5,6\n", "%d%d%d%f", "delimiter", ",",
%!               "TreatAsEmpty", "NN");
%! assert (C{3}(1), int32 (3));
%! assert (C{4}(1), NaN);

## MultipleDelimsAsOne
%!test
%! str = "11, 12, 13,, 15\n21,, 23, 24, 25\n,, 33, 34, 35\n";
%! C = textscan (str, "%f %f %f %f", "delimiter", ",",
%!                    "multipledelimsasone", 1, "endofline", "\n");
%! assert (C{1}', [11, 21, 33]);
%! assert (C{2}', [12, 23, 34]);
%! assert (C{3}', [13, 24, 35]);
%! assert (C{4}', [15, 25, NaN]);

## Single-quoted escape sequences
%!test
%! str = "11\t12\t13\r21\t22\t23";
%! c = textscan (str, "", "delimiter", '\t', "EndOfLine", '\r');
%! assert (c{1}', [11, 21]);
%! assert (c{2}', [12, 22]);
%! assert (c{3}', [13, 23]);

## Bug #44750
%!test
%! c = textscan ("/home/foo/", "%s", "delimiter", "/",
%!               "MultipleDelimsAsOne", 1);
%! assert (c{1}, {"home"; "foo"});

## FIXME: Test still fails (4/13/2016)?
## Allow cuddling %sliteral, but warn it is ambiguous
#%!test
#%! C = textscan ("abcxyz51\nxyz83\n##xyz101", "%s xyz %d");
#%! assert (C{1}([1 3]), {"abc"; "##"});
#%! assert (isempty (C{1}{2}), true);
#%! assert (C{2}, int32 ([51; 83; 101]));
### Literals are not delimiters.

## Test for false positives in check for non-supported format specifiers
%!test
%! c = textscan ("Total: 32.5 % (of cm values)",
%!               "Total: %f %% (of cm values)");
%! assert (c{1}, 32.5, 1e-5);

## Test various forms of string format specifiers (bug #45712)
%!test
%! str = "14 :1 z:2 z:3 z:5 z:11";
%! C = textscan (str, "%f %s %*s %3s %*3s %f", "delimiter", ":");
%! assert (C, {14, {"1 z"}, {"3 z"}, 11});

## Bit width, fixed width conversion specifiers
%!test
%! str2 = "123456789012345 ";
%! str2 = [str2 str2 str2 str2 str2 str2 str2 str2];
%! str2 = [str2 "123456789.01234 1234567890.1234 12345.678901234 12345.678901234"];
%! pttrn = "%3u8%*s %5u16%*s %10u32%*s %15u64 %3d8%*s %5d16%*s %10d32%*s %15d64 %9f32%*s %14f64%*s %10.2f32%*s %12.2f64%*s";
%! C = textscan (str2, pttrn, "delimiter", " ");
%! assert (C{1}, uint8 (123));
%! assert (C{2}, uint16 (12345));
%! assert (C{3}, uint32 (1234567890));
%! assert (C{4}, uint64 (123456789012345));
%! assert (C{5}, int8 (123));
%! assert (C{6}, int16 (12345));
%! assert (C{7}, int32 (1234567890));
%! assert (C{8}, int64 (123456789012345));
%! assert (C{9}, single (123456789), 1e-12);
%! assert (C{10}, double (1234567890.123), 1e-15);
%! assert (C{11}, single (12345.68), 1e-5);
%! assert (C{12}, double (12345.68), 1e-11);

## Bit width, fixed width conv. specifiers -- check the right amount is left
%!test
%! str2 = "123456789012345 ";
%! str2 = [str2 str2 "123456789.01234"];
%! pttrn = "%3u8 %5u16 %10u32 %3d8 %5d16 %10d32 %9f32 %9f";
%! C = textscan (str2, pttrn, "delimiter", " ");
%! assert (C{1}, uint8 (123));
%! assert (C{2}, uint16 (45678));
%! assert (C{3}, uint32 (9012345));
%! assert (C{4}, int8 (123));
%! assert (C{5}, int16 (45678));
%! assert (C{6}, int32 (9012345));
%! assert (C{7}, single (123456789), 1e-12);
%! assert (C{8}, double (0.01234), 1e-12);

%!test
%! C = textscan ("123.123", "%2f %3f %3f");
%! assert (C{1}, 12);
%! assert (C{2}, 3.1, 1e-11);
%! assert (C{3}, 23);

%!test
%! C = textscan ("123.123", "%3f %3f %3f");
%! assert (C{1}, 123);
%! assert (C{2}, 0.12, 1e-11);
%! assert (C{3}, 3);

%!test
%! C = textscan ("123.123", "%4f %3f");
%! assert (C{1}, 123);
%! assert (C{2}, 123);

## field width interrupts exponent.  (Matlab incorrectly gives [12, 2e12])
%!test
%! assert (textscan ("12e12",  "%4f"), {[120;  2]});
%! assert (textscan ("12e+12", "%5f"), {[120;  2]});
%! assert (textscan ("125e-12","%6f"), {[12.5; 2]});

## %[] tests
## Plain [..] and *[..]
%!test
%! ar = "abcdefguvwxAny\nacegxyzTrailing\nJunk";
%! C = textscan (ar, "%[abcdefg] %*[uvwxyz] %s");
%! assert (C{1}, {"abcdefg"; "aceg"; ""});
%! assert (C{2}, {"Any"; "Trailing"; "Junk"});

%!test
%! assert (textscan ("A2 B2 C3", "%*[ABC]%d", 3), {int32([2; 2; 3])});

## [^..] and *[^..]
%!test
%! br = "abcdefguvwx1Any\nacegxyz2Trailing\n3Junk";
%! C = textscan (br, "%[abcdefg] %*[^0123456789] %s");
%! assert (C{1}, {"abcdefg"; "aceg"; ""});
%! assert (C{2}, {"1Any"; "2Trailing"; "3Junk"});

## [..] and [^..] containing delimiters
%!test
%! cr = "ab cd efguv wx1Any\na ce gx yz2Trailing\n   3Junk";
%! C = textscan (cr, "%[ abcdefg] %*[^0123456789] %s", "delimiter", " \n",
%!                   "whitespace", "");
%! assert (C{1}, {"ab cd efg"; "a ce g"; "   "});
%! assert (C{2}, {"1Any"; "2Trailing"; "3Junk"});

## Bug #36464
%!assert (textscan ("1 2 3 4 5 6", "%*n%n%*[^\n]"){1}, 2);

## test %[]] and %[^]]
%!test
%! assert (textscan ("345]", "%*[123456]%[]]"){1}{1}, "]");
%! assert (textscan ("345]", "%*[^]]%s"){1}{1}, "]");

## Test that "-i" checks the next two characters
%!test
%! C = textscan ("-i -in -inf -infinity", "%f %f%s %f %f %s");
%! assert (C, {-i, -i, {"n"}, -Inf, -Inf, {"inity"}});

## Again for "+i", this time with custom parser
%!test
%! C = textscan ("+i +in +inf +infinity", "%f %f%s %f %f %s", "ExpChars", "eE");
%! assert (C, {i, i, {"n"}, Inf, Inf, {"inity"}});

## Check single quoted format interprets control sequences
%!test
%! C = textscan ("1 2\t3 4", '%f %[^\t] %f %f');
%! assert (C, {1, {"2"}, 3, 4});

%% Check a non-empty line with no valid conversion registers empytValue
%!test
%! C = textscan ("Empty\n", "Empty%f %f");
%! assert (C, { NaN, NaN });

## Check overflow and underflow of integer types
%!test
%! a = "-1e90 ";
%! b = "1e90 ";
%! fmt = "%d8 %d16 %d32 %d64 %u8 %u16 %u32 %u64 ";
%! C = textscan ([a a a a a a a a b b b b b b b b], fmt);
%! assert (C{1}, int8 ([-128; 127]));
%! assert (C{2}, int16 ([-32768; 32767]));
%! assert (C{3}, int32 ([-2147483648; 2147483647]));
%! assert (C{4}, int64 ([-9223372036854775808; 9223372036854775807]));
%! assert (C{5}, uint8 ([0; 255]));
%! assert (C{6}, uint16 ([0; 65535]));
%! assert (C{7}, uint32 ([0; 4294967295]));
%! assert (C{8}, uint64 ([0; 18446744073709551615]));

## Tests from Matlab (does The MathWorks have any copyright over the input?)
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid,"09/12/2005 Level1 12.34 45 1.23e10 inf Nan Yes 5.1+3i\n");
%! fprintf (fid,"10/12/2005 Level2 23.54 60 9e19 -inf  0.001 No 2.2-.5i\n");
%! fprintf (fid,"11/12/2005 Level3 34.90 12 2e5   10  100   No 3.1+.1i\n");
%! fseek (fid, 0, "bof");
%! C = textscan (fid,"%s %s %f32 %d8 %u %f %f %s %f");
%! %assert (C{1}, {"09/12/2005";"10/12/2005";"11/12/2005"});
%! assert (C{2}, {"Level1";"Level2";"Level3"});
%! assert (C{3}, [single(12.34);single(23.54);single(34.90)]);
%! assert (C{4}, [int8(45);int8(60);int8(12)]);
%! assert (C{5}, [uint32(4294967295);uint32(4294967295);uint32(200000)]);
%! assert (C{6}, [inf;-inf;10]);
%! assert (C{7}, [NaN;0.001;100], eps);
%! assert (C{8}, {"Yes";"No";"No"});
%! assert (C{9}, [5.1+3i;2.2-0.5i;3.1+0.1i]);
%! fseek (fid, 0, "bof");
%! C = textscan (fid,"%s Level%d %f32 %d8 %u %f %f %s %f");
%! assert (C{2}, [int32(1);int32(2);int32(3)]);
%! assert (C{3}, [single(12.34);single(23.54);single(34.90)]);
%! fseek (fid, 0, "bof");
%! C = textscan (fid, '%s %*[^\n]');
%! fclose (fid);
%! unlink (f);
%! assert (C, {{"09/12/2005";"10/12/2005";"11/12/2005"}});

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid,"1,  2,  3,  4,   ,  6\n");
%! fprintf (fid,"7,  8,  9,   , 11, 12\n");
%! fseek (fid, 0, "bof");
%! C = textscan (fid,"%f %f %f %f %u8 %f", "Delimiter",",","EmptyValue",-Inf);
%! fclose (fid);
%! unlink (f);
%! assert (C{4}, [4; -Inf]);
%! assert (C{5}, uint8 ([0; 11]));

%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! fprintf (fid,"abc, 2, NA, 3, 4\n");
%! fprintf (fid,"// Comment Here\n");
%! fprintf (fid,"def, na, 5, 6, 7\n");
%! fseek (fid, 0, "bof");
%! C = textscan (fid, "%s %n %n %n %n", "Delimiter", ",",
%!                    "TreatAsEmpty", {"NA","na"}, "CommentStyle", "//");
%! fclose (fid);
%! unlink (f);
%! assert (C{1}, {"abc";"def"});
%! assert (C{2}, [2; NaN]);
%! assert (C{3}, [NaN; 5]);
%! assert (C{4}, [3; 6]);
%! assert (C{5}, [4; 7]);

## FIXME: Almost passes.  Second return value is {"/"}.  Tested 4/14/16.
### Test start of comment as string
#%!test
#%! c = textscan ("1 / 2 // 3", "%n %s %u8", "CommentStyle", {"//"});
#%! assert (c(1), {1, "/", 2});

%!assert (textscan (["1 2 3 4"; "5 6 7 8"], "%f"), {[15; 26; 37; 48]})

## Check for delimiter after exponent
%!assert (textscan ("1e-3|42", "%f", "delimiter", "|"), {[1e-3; 42]})
*/

// These tests have end-comment sequences, so can't just be in a comment
#if 0
## Test unfinished comment
%!test
%! c = textscan ("1 2 /* half comment", "%n %u8", "CommentStyle", {"/*", "*/"});
%! assert (c, {1, 2});

## Test reading from a real file
%!test
%! f = tempname ();
%! fid = fopen (f, "w+");
%! d = rand (1, 4);
%! fprintf (fid, "  %f %f /* comment */  %f  %f ", d);
%! fseek (fid, 0, "bof");
%! A = textscan (fid, "%f %f", "CommentStyle", {"/*", "*/"});
%! E = feof (fid);
%! fclose (fid);
%! unlink (f);
%! assert (A{1}, [d(1); d(3)], 1e-6);
%! assert (A{2}, [d(2); d(4)], 1e-6);
%! assert (E);
#endif

/*
## Test input validation
%!error textscan ()
%!error textscan (single (40))
%!error textscan ({40})
%!error <must be a string> textscan ("Hello World", 2)
%!error <at most one character or>
%! textscan ("Hello World", "%s", "EndOfLine", 3);
%!error <'%z' is not a valid format specifier> textscan ("1.0", "%z")
%!error <no valid format conversion specifiers> textscan ("1.0", "foo")
*/

static octave_value
do_fread (octave_stream& os, const octave_value& size_arg,
          const octave_value& prec_arg, const octave_value& skip_arg,
          const octave_value& arch_arg, octave_idx_type& count)
{
  count = -1;

  Array<double> size = size_arg.xvector_value ("fread: invalid SIZE specified");

  std::string prec = prec_arg.xstring_value ("fread: PRECISION must be a string");

  int block_size = 1;
  oct_data_conv::data_type input_type;
  oct_data_conv::data_type output_type;

  try
    {
      oct_data_conv::string_to_data_type (prec, block_size,
                                          input_type, output_type);
    }
  catch (octave_execution_exception& e)
    {
      error (e, "fread: invalid PRECISION specified");
    }

  int skip = 0;

  try
    {
      skip = skip_arg.int_value (true);
    }
  catch (octave_execution_exception& e)
    {
      error (e, "fread: SKIP must be an integer");
    }

  std::string arch = arch_arg.xstring_value ("fread: ARCH architecture type must be a string");

  oct_mach_info::float_format flt_fmt
    = oct_mach_info::string_to_float_format (arch);

  return os.read (size, block_size, input_type, output_type, skip,
                  flt_fmt, count);
}

DEFUN (fread, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{val} =} fread (@var{fid})\n\
@deftypefnx {} {@var{val} =} fread (@var{fid}, @var{size})\n\
@deftypefnx {} {@var{val} =} fread (@var{fid}, @var{size}, @var{precision})\n\
@deftypefnx {} {@var{val} =} fread (@var{fid}, @var{size}, @var{precision}, @var{skip})\n\
@deftypefnx {} {@var{val} =} fread (@var{fid}, @var{size}, @var{precision}, @var{skip}, @var{arch})\n\
@deftypefnx {} {[@var{val}, @var{count}] =} fread (@dots{})\n\
Read binary data from the file specified by the file descriptor @var{fid}.\n\
\n\
The optional argument @var{size} specifies the amount of data to read\n\
and may be one of\n\
\n\
@table @code\n\
@item Inf\n\
Read as much as possible, returning a column vector.\n\
\n\
@item @var{nr}\n\
Read up to @var{nr} elements, returning a column vector.\n\
\n\
@item [@var{nr}, Inf]\n\
Read as much as possible, returning a matrix with @var{nr} rows.  If the\n\
number of elements read is not an exact multiple of @var{nr}, the last\n\
column is padded with zeros.\n\
\n\
@item [@var{nr}, @var{nc}]\n\
Read up to @code{@var{nr} * @var{nc}} elements, returning a matrix with\n\
@var{nr} rows.  If the number of elements read is not an exact multiple\n\
of @var{nr}, the last column is padded with zeros.\n\
@end table\n\
\n\
@noindent\n\
If @var{size} is omitted, a value of @code{Inf} is assumed.\n\
\n\
The optional argument @var{precision} is a string specifying the type of\n\
data to read and may be one of\n\
\n\
@table @asis\n\
@item  @qcode{\"schar\"}\n\
@itemx @qcode{\"signed char\"}\n\
Signed character.\n\
\n\
@item  @qcode{\"uchar\"}\n\
@itemx @qcode{\"unsigned char\"}\n\
Unsigned character.\n\
\n\
@item  @qcode{\"int8\"}\n\
@itemx @qcode{\"integer*1\"}\n\
\n\
8-bit signed integer.\n\
\n\
@item  @qcode{\"int16\"}\n\
@itemx @qcode{\"integer*2\"}\n\
16-bit signed integer.\n\
\n\
@item  @qcode{\"int32\"}\n\
@itemx @qcode{\"integer*4\"}\n\
32-bit signed integer.\n\
\n\
@item  @qcode{\"int64\"}\n\
@itemx @qcode{\"integer*8\"}\n\
64-bit signed integer.\n\
\n\
@item @qcode{\"uint8\"}\n\
8-bit unsigned integer.\n\
\n\
@item @qcode{\"uint16\"}\n\
16-bit unsigned integer.\n\
\n\
@item @qcode{\"uint32\"}\n\
32-bit unsigned integer.\n\
\n\
@item @qcode{\"uint64\"}\n\
64-bit unsigned integer.\n\
\n\
@item  @qcode{\"single\"}\n\
@itemx @qcode{\"float32\"}\n\
@itemx @qcode{\"real*4\"}\n\
32-bit floating point number.\n\
\n\
@item  @qcode{\"double\"}\n\
@itemx @qcode{\"float64\"}\n\
@itemx @qcode{\"real*8\"}\n\
64-bit floating point number.\n\
\n\
@item  @qcode{\"char\"}\n\
@itemx @qcode{\"char*1\"}\n\
Single character.\n\
\n\
@item @qcode{\"short\"}\n\
Short integer (size is platform dependent).\n\
\n\
@item @qcode{\"int\"}\n\
Integer (size is platform dependent).\n\
\n\
@item @qcode{\"long\"}\n\
Long integer (size is platform dependent).\n\
\n\
@item  @qcode{\"ushort\"}\n\
@itemx @qcode{\"unsigned short\"}\n\
Unsigned short integer (size is platform dependent).\n\
\n\
@item  @qcode{\"uint\"}\n\
@itemx @qcode{\"unsigned int\"}\n\
Unsigned integer (size is platform dependent).\n\
\n\
@item  @qcode{\"ulong\"}\n\
@itemx @qcode{\"unsigned long\"}\n\
Unsigned long integer (size is platform dependent).\n\
\n\
@item @qcode{\"float\"}\n\
Single precision floating point number (size is platform dependent).\n\
@end table\n\
\n\
@noindent\n\
The default precision is @qcode{\"uchar\"}.\n\
\n\
The @var{precision} argument may also specify an optional repeat\n\
count.  For example, @samp{32*single} causes @code{fread} to read\n\
a block of 32 single precision floating point numbers.  Reading in\n\
blocks is useful in combination with the @var{skip} argument.\n\
\n\
The @var{precision} argument may also specify a type conversion.\n\
For example, @samp{int16=>int32} causes @code{fread} to read 16-bit\n\
integer values and return an array of 32-bit integer values.  By\n\
default, @code{fread} returns a double precision array.  The special\n\
form @samp{*TYPE} is shorthand for @samp{TYPE=>TYPE}.\n\
\n\
The conversion and repeat counts may be combined.  For example, the\n\
specification @samp{32*single=>single} causes @code{fread} to read\n\
blocks of single precision floating point values and return an array\n\
of single precision values instead of the default array of double\n\
precision values.\n\
\n\
The optional argument @var{skip} specifies the number of bytes to skip\n\
after each element (or block of elements) is read.  If it is not\n\
specified, a value of 0 is assumed.  If the final block read is not\n\
complete, the final skip is omitted.  For example,\n\
\n\
@example\n\
fread (f, 10, \"3*single=>single\", 8)\n\
@end example\n\
\n\
@noindent\n\
will omit the final 8-byte skip because the last read will not be\n\
a complete block of 3 values.\n\
\n\
The optional argument @var{arch} is a string specifying the data format\n\
for the file.  Valid values are\n\
\n\
@table @asis\n\
@item @qcode{\"native\"} or @qcode{\"n\"}\n\
The format of the current machine.\n\
\n\
@item @qcode{\"ieee-be\"} or @qcode{\"b\"}\n\
IEEE big endian.\n\
\n\
@item @qcode{\"ieee-le\"} or @qcode{\"l\"}\n\
IEEE little endian.\n\
@end table\n\
\n\
If no @var{arch} is given the value used in the call to @code{fopen} which\n\
created the file descriptor is used.  Otherwise, the value specified with\n\
@code{fread} overrides that of @code{fopen} and determines the data format.\n\
\n\
The output argument @var{val} contains the data read from the file.\n\
\n\
The optional return value @var{count} contains the number of elements read.\n\
@seealso{fwrite, fgets, fgetl, fscanf, fopen}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 5)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "fread");

  octave_value size = lo_ieee_inf_value ();
  octave_value prec = "uchar";
  octave_value skip = 0;
  octave_value arch = "unknown";

  int idx = 1;

  if (nargin > idx && ! args(idx).is_string ())
    size = args(idx++);

  if (nargin > idx)
    prec = args(idx++);

  if (nargin > idx)
    skip = args(idx++);

  if (nargin > idx)
    arch = args(idx++);
  else if (skip.is_string ())
    {
      arch = skip;
      skip = 0;
    }

  octave_idx_type count = -1;

  octave_value tmp = do_fread (os, size, prec, skip, arch, count);

  return ovl (tmp, count);
}

static int
do_fwrite (octave_stream& os, const octave_value& data,
           const octave_value& prec_arg, const octave_value& skip_arg,
           const octave_value& arch_arg)
{
  std::string prec = prec_arg.xstring_value ("fwrite: PRECISION must be a string");

  int block_size = 1;
  oct_data_conv::data_type output_type;

  try
    {
      oct_data_conv::string_to_data_type (prec, block_size, output_type);
    }
  catch (octave_execution_exception& e)
    {
      error (e, "fwrite: invalid PRECISION specified");
    }

  int skip = 0;

  try
    {
      skip = skip_arg.int_value (true);
    }
  catch (octave_execution_exception& e)
    {
      error (e, "fwrite: SKIP must be an integer");
    }

  std::string arch = arch_arg.xstring_value ("fwrite: ARCH architecture type must be a string");

  oct_mach_info::float_format flt_fmt
    = oct_mach_info::string_to_float_format (arch);

  return os.write (data, block_size, output_type, skip, flt_fmt);
}

DEFUN (fwrite, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {} fwrite (@var{fid}, @var{data})\n\
@deftypefnx {} {} fwrite (@var{fid}, @var{data}, @var{precision})\n\
@deftypefnx {} {} fwrite (@var{fid}, @var{data}, @var{precision}, @var{skip})\n\
@deftypefnx {} {} fwrite (@var{fid}, @var{data}, @var{precision}, @var{skip}, @var{arch})\n\
@deftypefnx {} {@var{count} =} fwrite (@dots{})\n\
Write data in binary form to the file specified by the file descriptor\n\
@var{fid}, returning the number of values @var{count} successfully written\n\
to the file.\n\
\n\
The argument @var{data} is a matrix of values that are to be written to\n\
the file.  The values are extracted in column-major order.\n\
\n\
The remaining arguments @var{precision}, @var{skip}, and @var{arch} are\n\
optional, and are interpreted as described for @code{fread}.\n\
\n\
The behavior of @code{fwrite} is undefined if the values in @var{data}\n\
are too large to fit in the specified precision.\n\
@seealso{fread, fputs, fprintf, fopen}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 2 || nargin > 5)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "fwrite");

  octave_value prec = "uchar";
  octave_value skip = 0;
  octave_value arch = "unknown";

  int idx = 1;

  octave_value data = args(idx++);

  if (nargin > idx)
    prec = args(idx++);

  if (nargin > idx)
    skip = args(idx++);

  if (nargin > idx)
    arch = args(idx++);
  else if (skip.is_string ())
    {
      arch = skip;
      skip = 0;
    }

  return ovl (do_fwrite (os, data, prec, skip, arch));
}

DEFUNX ("feof", Ffeof, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {@var{status} =} feof (@var{fid})\n\
Return 1 if an end-of-file condition has been encountered for the file\n\
specified by file descriptor @var{fid} and 0 otherwise.\n\
\n\
Note that @code{feof} will only return 1 if the end of the file has already\n\
been encountered, not if the next read operation will result in an\n\
end-of-file condition.\n\
@seealso{fread, frewind, fseek, fclear, fopen}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "feof");

  return ovl (os.eof () ? 1.0 : 0.0);
}

DEFUNX ("ferror", Fferror, args, ,
        "-*- texinfo -*-\n\
@deftypefn  {} {@var{msg} =} ferror (@var{fid})\n\
@deftypefnx {} {[@var{msg}, @var{err}] =} ferror (@var{fid})\n\
@deftypefnx {} {[@var{dots}] =} ferror (@var{fid}, \"clear\")\n\
Query the error status of the stream specified by file descriptor @var{fid}\n\
\n\
If an error condition exists then return a string @var{msg} describing the\n\
error.  Otherwise, return an empty string @qcode{\"\"}.\n\
\n\
The second input @qcode{\"clear\"} is optional.  If supplied, the error\n\
state on the stream will be cleared.\n\
\n\
The optional second output is a numeric indication of the error status.\n\
@var{err} is 1 if an error condition has been encountered and 0 otherwise.\n\
\n\
Note that @code{ferror} indicates if an error has already occurred, not\n\
whether the next operation will result in an error condition.\n\
@seealso{fclear, fopen}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave_stream os = octave_stream_list::lookup (args(0), "ferror");

  bool clear = false;

  if (nargin == 2)
    {
      std::string opt = args(1).string_value ();

      clear = (opt == "clear");
    }

  int error_number = 0;

  std::string error_message = os.error (clear, error_number);

  return ovl (error_message, error_number);
}

DEFUNX ("popen", Fpopen, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {@var{fid} =} popen (@var{command}, @var{mode})\n\
Start a process and create a pipe.\n\
\n\
The name of the command to run is given by @var{command}.  The argument\n\
@var{mode} may be\n\
\n\
@table @code\n\
@item @qcode{\"r\"}\n\
The pipe will be connected to the standard output of the process, and\n\
open for reading.\n\
\n\
@item @qcode{\"w\"}\n\
The pipe will be connected to the standard input of the process, and\n\
open for writing.\n\
@end table\n\
\n\
The file identifier corresponding to the input or output stream of the\n\
process is returned in @var{fid}.\n\
\n\
For example:\n\
\n\
@example\n\
@group\n\
fid = popen (\"ls -ltr / | tail -3\", \"r\");\n\
while (ischar (s = fgets (fid)))\n\
  fputs (stdout, s);\n\
endwhile\n\
\n\
   @print{} drwxr-xr-x  33 root  root  3072 Feb 15 13:28 etc\n\
   @print{} drwxr-xr-x   3 root  root  1024 Feb 15 13:28 lib\n\
   @print{} drwxrwxrwt  15 root  root  2048 Feb 17 14:53 tmp\n\
@end group\n\
@end example\n\
@seealso{popen2}\n\
@end deftypefn")
{
  if (args.length () != 2)
    print_usage ();

  std::string name = args(0).xstring_value ("popen: COMMAND must be a string");
  std::string mode = args(1).xstring_value ("popen: MODE must be a string");

  octave_value retval;

  if (mode == "r")
    {
      octave_stream ips = octave_iprocstream::create (name);

      retval = octave_stream_list::insert (ips);
    }
  else if (mode == "w")
    {
      octave_stream ops = octave_oprocstream::create (name);

      retval = octave_stream_list::insert (ops);
    }
  else
    error ("popen: invalid MODE specified");

  return retval;
}

DEFUNX ("pclose", Fpclose, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} pclose (@var{fid})\n\
Close a file identifier that was opened by @code{popen}.\n\
\n\
The function @code{fclose} may also be used for the same purpose.\n\
@seealso{fclose, popen}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  return ovl (octave_stream_list::remove (args(0), "pclose"));
}

DEFUN (tempname, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {@var{fname} =} tempname ()\n\
@deftypefnx {} {@var{fname} =} tempname (@var{dir})\n\
@deftypefnx {} {@var{fname} =} tempname (@var{dir}, @var{prefix})\n\
Return a unique temporary filename as a string.\n\
\n\
If @var{prefix} is omitted, a value of @qcode{\"oct-\"} is used.\n\
\n\
If @var{dir} is also omitted, the default directory for temporary files\n\
(@code{P_tmpdir}) is used.  If @var{dir} is provided, it must exist,\n\
otherwise the default directory for temporary files is used.\n\
\n\
Programming Note: Because the named file is not opened by @code{tempname},\n\
it is possible, though relatively unlikely, that it will not be available\n\
by the time your program attempts to open it.  If this is a concern,\n\
see @code{tmpfile}.\n\
@seealso{mkstemp, tempdir, P_tmpdir, tmpfile}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin > 2)
    print_usage ();

  std::string dir;

  if (nargin > 0)
    dir = args(0).xstring_value ("tempname: DIR must be a string");

  std::string pfx ("oct-");

  if (nargin > 1)
    pfx = args(1).xstring_value ("tempname: PREFIX must be a string");

  return ovl (octave_tempnam (dir, pfx));
}

/*
%!test
%! if (ispc ())
%!   envname = "TMP";
%! else
%!   envname = "TMPDIR";
%! endif
%! envdir = getenv (envname);
%! unsetenv (envname);
%! ## Strip trailing file separators from P_tmpdir
%! def_tmpdir = P_tmpdir;
%! while (length (def_tmpdir) > 2 && strfind (filesep ("all"), def_tmpdir(end)))
%!   def_tmpdir(end) = [];
%! endwhile
%! unwind_protect
%!   ## Test 0-argument form
%!   fname = tempname ();
%!   [tmpdir, tmpfname] = fileparts (fname);
%!   assert (tmpdir, def_tmpdir);
%!   assert (tmpfname (1:4), "oct-");
%!   ## Test 1-argument form
%!   tmp_tmpdir = [def_tmpdir filesep() substr(tmpfname, -5)];
%!   mkdir (tmp_tmpdir) || error ("Unable to create tmp dir");
%!   setenv (envname, def_tmpdir);
%!   fname = tempname (tmp_tmpdir);
%!   [tmpdir, tmpfname] = fileparts (fname);
%!   assert (tmpdir, tmp_tmpdir);
%!   assert (tmpfname (1:4), "oct-");
%!   ## Test 1-argument form w/null tmpdir
%!   fname = tempname ("");
%!   [tmpdir, tmpfname] = fileparts (fname);
%!   assert (tmpdir, def_tmpdir);
%!   assert (tmpfname (1:4), "oct-");
%!   ## Test 2-argument form
%!   fname = tempname (tmp_tmpdir, "pfx-");
%!   [tmpdir, tmpfname] = fileparts (fname);
%!   assert (tmpdir, tmp_tmpdir);
%!   assert (tmpfname (1:4), "pfx-");
%!   ## Test 2-argument form w/null prefix
%!   fname = tempname (tmp_tmpdir, "");
%!   [tmpdir, tmpfname] = fileparts (fname);
%!   assert (tmpdir, tmp_tmpdir);
%!   assert (tmpfname (1:4), "file");
%! unwind_protect_cleanup
%!   rmdir (tmp_tmpdir);
%!   if (isempty (envdir))
%!     unsetenv (envname);
%!   else
%!     setenv (envname, envdir);
%!   endif
%! end_unwind_protect
*/

DEFUN (tmpfile, args, ,
       "-*- texinfo -*-\n\
@deftypefn {} {[@var{fid}, @var{msg}] =} tmpfile ()\n\
Return the file ID corresponding to a new temporary file with a unique\n\
name.\n\
\n\
The file is opened in binary read/write (@qcode{\"w+b\"}) mode and will be\n\
deleted automatically when it is closed or when Octave exits.\n\
\n\
If successful, @var{fid} is a valid file ID and @var{msg} is an empty\n\
string.  Otherwise, @var{fid} is -1 and @var{msg} contains a\n\
system-dependent error message.\n\
@seealso{tempname, mkstemp, tempdir, P_tmpdir}\n\
@end deftypefn")
{
  if (args.length () != 0)
    print_usage ();

  octave_value_list retval;

  FILE *fid = gnulib::tmpfile ();

  if (fid)
    {
      std::string nm;

      std::ios::openmode md = fopen_mode_to_ios_mode ("w+b");

      octave_stream s = octave_stdiostream::create (nm, fid, md);

      if (! s)
        error ("tmpfile: failed to create octave_stdiostream object");

      retval = ovl (octave_stream_list::insert (s), "");
    }
  else
    {
      retval = ovl (-1, gnulib::strerror (errno));
    }

  return retval;
}

DEFUN (mkstemp, args, ,
       "-*- texinfo -*-\n\
@deftypefn  {} {[@var{fid}, @var{name}, @var{msg}] =} mkstemp (\"@var{template}\")\n\
@deftypefnx {} {[@var{fid}, @var{name}, @var{msg}] =} mkstemp (\"@var{template}\", @var{delete})\n\
Return the file descriptor @var{fid} corresponding to a new temporary file\n\
with a unique name created from @var{template}.\n\
\n\
The last six characters of @var{template} must be @qcode{\"XXXXXX\"} and\n\
these are replaced with a string that makes the filename unique.  The file\n\
is then created with mode read/write and permissions that are system\n\
dependent (on GNU/Linux systems, the permissions will be 0600 for versions\n\
of glibc 2.0.7 and later).  The file is opened in binary mode and with the\n\
@w{@code{O_EXCL}} flag.\n\
\n\
If the optional argument @var{delete} is supplied and is true, the file will\n\
be deleted automatically when Octave exits.\n\
\n\
If successful, @var{fid} is a valid file ID, @var{name} is the name of the\n\
file, and @var{msg} is an empty string.  Otherwise, @var{fid} is -1,\n\
@var{name} is empty, and @var{msg} contains a system-dependent error\n\
message.\n\
@seealso{tempname, tempdir, P_tmpdir, tmpfile, fopen}\n\
@end deftypefn")
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  std::string tmpl8 = args(0).xstring_value ("mkstemp: TEMPLATE argument must be a string");

  octave_value_list retval = ovl (-1, "", "");

  OCTAVE_LOCAL_BUFFER (char, tmp, tmpl8.size () + 1);
  strcpy (tmp, tmpl8.c_str ());

  int fd = gnulib::mkostemp (tmp, O_BINARY);

  if (fd < 0)
    {
      retval(0) = fd;
      retval(2) = gnulib::strerror (errno);
    }
  else
    {
      const char *fopen_mode = "w+b";

      FILE *fid = fdopen (fd, fopen_mode);

      if (! fid)
        {
          retval(0) = -1;
          retval(2) = gnulib::strerror (errno);
        }
      else
        {
          std::string nm = tmp;

          std::ios::openmode md = fopen_mode_to_ios_mode (fopen_mode);

          octave_stream s = octave_stdiostream::create (nm, fid, md);

          if (! s)
            error ("mkstemp: failed to create octave_stdiostream object");

          retval(0) = octave_stream_list::insert (s);
          retval(1) = nm;

          if (nargin == 2 && args(1).is_true ())
            mark_for_deletion (nm);
        }
    }

  return retval;
}

// FIXME: This routine also exists verbatim in syscalls.cc.
//        Maybe change to be a general utility routine.
static int
convert (int x, int ibase, int obase)
{
  int retval = 0;

  int tmp = x % obase;

  if (tmp > ibase - 1)
    error ("umask: invalid digit");

  retval = tmp;
  int mult = ibase;
  while ((x = (x - tmp) / obase))
    {
      tmp = x % obase;

      if (tmp > ibase - 1)
        error ("umask: invalid digit");

      retval += mult * tmp;
      mult *= ibase;
    }

  return retval;
}

DEFUNX ("umask", Fumask, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} umask (@var{mask})\n\
Set the permission mask for file creation.\n\
\n\
The parameter @var{mask} is an integer, interpreted as an octal number.\n\
\n\
If successful, returns the previous value of the mask (as an integer to be\n\
interpreted as an octal number); otherwise an error message is printed.\n\
\n\
The permission mask is a UNIX concept used when creating new objects on a\n\
file system such as files, directories, or named FIFOs.  The object to be\n\
created has base permissions in an octal number @var{mode} which are\n\
modified according to the octal value of @var{mask}.  The final permissions\n\
for the new object are @code{@var{mode} - @var{mask}}.\n\
@seealso{fopen, mkdir, mkfifo}\n\
@end deftypefn")
{
  if (args.length () != 1)
    print_usage ();

  int mask = args(0).xint_value ("umask: MASK must be an integer");

  if (mask < 0)
    error ("umask: MASK must be a positive integer value");

  int oct_mask = convert (mask, 8, 10);

  int status = convert (octave_umask (oct_mask), 10, 8);

  if (status >= 0)
    return ovl (status);
  else
    return ovl ();
}

static octave_value
const_value (const char *, const octave_value_list& args, int val)
{
  if (args.length () != 0)
    print_usage ();

  return octave_value (val);
}

DEFUNX ("P_tmpdir", FP_tmpdir, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} P_tmpdir ()\n\
Return the name of the host system's @strong{default} directory for\n\
temporary files.\n\
\n\
Programming Note: The value returned by @code{P_tmpdir} is always the\n\
default location.  This value may not agree with that returned from\n\
@code{tempdir} if the user has overridden the default with the @env{TMPDIR}\n\
environment variable.\n\
@seealso{tempdir, tempname, mkstemp, tmpfile}\n\
@end deftypefn")
{
  if (args.length () != 0)
    print_usage ();

  return ovl (get_P_tmpdir ());
}

// NOTE: the values of SEEK_SET, SEEK_CUR, and SEEK_END have to be
// this way for Matlab compatibility.

DEFUNX ("SEEK_SET", FSEEK_SET, args, ,
        "-*- texinfo -*-\n\
@deftypefn  {} {} SEEK_SET ()\n\
@deftypefnx {} {} SEEK_CUR ()\n\
@deftypefnx {} {} SEEK_END ()\n\
Return the numerical value to pass to @code{fseek} to perform one of the\n\
following actions:\n\
\n\
@table @code\n\
@item SEEK_SET\n\
Position file relative to the beginning.\n\
\n\
@item SEEK_CUR\n\
Position file relative to the current position.\n\
\n\
@item SEEK_END\n\
Position file relative to the end.\n\
@end table\n\
@seealso{fseek}\n\
@end deftypefn")
{
  return const_value ("SEEK_SET", args, -1);
}

DEFUNX ("SEEK_CUR", FSEEK_CUR, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} SEEK_CUR ()\n\
Return the numerical value to pass to @code{fseek} to\n\
position the file pointer relative to the current position.\n\
@seealso{SEEK_SET, SEEK_END}\n\
@end deftypefn")
{
  return const_value ("SEEK_CUR", args, 0);
}

DEFUNX ("SEEK_END", FSEEK_END, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} SEEK_END ()\n\
Return the numerical value to pass to @code{fseek} to\n\
position the file pointer relative to the end of the file.\n\
@seealso{SEEK_SET, SEEK_CUR}\n\
@end deftypefn")
{
  return const_value ("SEEK_END", args, 1);
}

static octave_value
const_value (const char *, const octave_value_list& args,
             const octave_value& val)
{
  if (args.length () != 0)
    print_usage ();

  return octave_value (val);
}

DEFUNX ("stdin", Fstdin, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} stdin ()\n\
Return the numeric value corresponding to the standard input stream.\n\
\n\
When Octave is used interactively, stdin is filtered through the command\n\
line editing functions.\n\
@seealso{stdout, stderr}\n\
@end deftypefn")
{
  return const_value ("stdin", args, stdin_file);
}

DEFUNX ("stdout", Fstdout, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} stdout ()\n\
Return the numeric value corresponding to the standard output stream.\n\
\n\
Data written to the standard output is normally filtered through the pager.\n\
@seealso{stdin, stderr}\n\
@end deftypefn")
{
  return const_value ("stdout", args, stdout_file);
}

DEFUNX ("stderr", Fstderr, args, ,
        "-*- texinfo -*-\n\
@deftypefn {} {} stderr ()\n\
Return the numeric value corresponding to the standard error stream.\n\
\n\
Even if paging is turned on, the standard error is not sent to the pager.\n\
It is useful for error messages and prompts.\n\
@seealso{stdin, stdout}\n\
@end deftypefn")
{
  return const_value ("stderr", args, stderr_file);
}
