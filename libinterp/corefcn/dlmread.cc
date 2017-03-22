/*

Copyright (C) 2008-2017 Jonathan Stickel
Copyright (C) 2010 Jaroslav Hajek

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

// Adapted from previous version of dlmread.occ as authored by Kai
// Habel, but core code has been completely re-written.

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cctype>
#include <fstream>
#include <limits>

#include "file-ops.h"
#include "lo-ieee.h"

#include "defun.h"
#include "oct-stream.h"
#include "error.h"
#include "ovl.h"
#include "utils.h"

static const octave_idx_type idx_max =
  std::numeric_limits<octave_idx_type>::max () - 1;

static const double idx_max_dbl = double (idx_max);

static bool
read_cell_spec (std::istream& is, octave_idx_type& row, octave_idx_type& col)
{
  bool stat = false;

  if (is.peek () == std::istream::traits_type::eof ())
    stat = true;
  else
    {
      if (::isalpha (is.peek ()))
        {
          col = 0;
          while (is && ::isalpha (is.peek ()))
            {
              char ch = is.get ();
              col *= 26;
              if (ch >= 'a')
                col += ch - 'a' + 1;
              else
                col += ch - 'A' + 1;
            }
          col--;

          if (is)
            {
              is >> row;
              row--;
              if (is)
                stat = true;
            }
        }
    }

  return stat;
}

static bool
parse_range_spec (const octave_value& range_spec,
                  octave_idx_type& rlo, octave_idx_type& clo,
                  octave_idx_type& rup, octave_idx_type& cup)
{
  bool stat = true;

  if (range_spec.is_string ())
    {
      std::istringstream is (range_spec.string_value ());
      char ch = is.peek ();

      if (ch == '.' || ch == ':')
        {
          rlo = 0;
          clo = 0;
          ch = is.get ();
          if (ch == '.')
            {
              ch = is.get ();
              if (ch != '.')
                stat = false;
            }
        }
      else
        {
          stat = read_cell_spec (is, rlo, clo);

          if (stat)
            {
              ch = is.peek ();

              if (ch == '.' || ch == ':')
                {
                  ch = is.get ();
                  if (ch == '.')
                    {
                      ch = is.get ();
                      if (! is || ch != '.')
                        stat = false;
                    }

                  rup = idx_max;
                  cup = idx_max;
                }
              else
                {
                  rup = rlo;
                  cup = clo;
                  if (! is || ! is.eof ())
                    stat = false;
                }
            }
        }

      if (stat && is && ! is.eof ())
        stat = read_cell_spec (is, rup, cup);

      if (! is || ! is.eof ())
        stat = false;
    }
  else if (range_spec.is_real_matrix () && range_spec.numel () == 4)
    {
      ColumnVector range (range_spec.vector_value ());
      // double --> unsigned int avoiding any overflow
      rlo = static_cast<octave_idx_type> (std::min (range(0), idx_max_dbl));
      clo = static_cast<octave_idx_type> (std::min (range(1), idx_max_dbl));
      rup = static_cast<octave_idx_type> (std::min (range(2), idx_max_dbl));
      cup = static_cast<octave_idx_type> (std::min (range(3), idx_max_dbl));
    }
  else
    stat = false;

  return stat;
}

DEFUN (dlmread, args, ,
       doc: /* -*- texinfo -*-
@deftypefn  {} {@var{data} =} dlmread (@var{file})
@deftypefnx {} {@var{data} =} dlmread (@var{file}, @var{sep})
@deftypefnx {} {@var{data} =} dlmread (@var{file}, @var{sep}, @var{r0}, @var{c0})
@deftypefnx {} {@var{data} =} dlmread (@var{file}, @var{sep}, @var{range})
@deftypefnx {} {@var{data} =} dlmread (@dots{}, "emptyvalue", @var{EMPTYVAL})
Read numeric data from the text file @var{file} which uses the delimiter
@var{sep} between data values.

If @var{sep} is not defined the separator between fields is determined from
the file itself.

The optional scalar arguments @var{r0} and @var{c0} define the starting row
and column of the data to be read.  These values are indexed from zero,
i.e., the first data row corresponds to an index of zero.

The @var{range} parameter specifies exactly which data elements are read.
The first form of the parameter is a 4-element vector containing the upper
left and lower right corners @code{[@var{R0},@var{C0},@var{R1},@var{C1}]}
where the indices are zero-based.  Alternatively, a spreadsheet style
form such as @qcode{"A2..Q15"} or @qcode{"T1:AA5"} can be used.  The
lowest alphabetical index @qcode{'A'} refers to the first column.  The
lowest row index is 1.

@var{file} should be a filename or a file id given by @code{fopen}.  In the
latter case, the file is read until end of file is reached.

The @qcode{"emptyvalue"} option may be used to specify the value used to
fill empty fields.  The default is zero.  Note that any non-numeric values,
such as text, are also replaced by the @qcode{"emptyvalue"}.
@seealso{csvread, textscan, textread, dlmwrite}
@end deftypefn */)
{
  int nargin = args.length ();

  double empty_value = 0.0;

  if (nargin > 2 && args(nargin-2).is_string ()
      && args(nargin-2).string_value () == "emptyvalue")
    {
      empty_value = args(nargin-1).double_value ();

      nargin -= 2;
    }

  if (nargin < 1 || nargin > 4)
    print_usage ();

  std::istream *input = 0;
  std::ifstream input_file;

  if (args(0).is_string ())
    {
      // Filename.
      std::string fname (args(0).string_value ());

      std::string tname = octave::sys::file_ops::tilde_expand (fname);

      tname = find_data_file_in_load_path ("dlmread", tname);

      input_file.open (tname.c_str (), std::ios::in);

      if (! input_file)
        error ("dlmread: unable to open file '%s'", fname.c_str ());

      input = &input_file;
    }
  else if (args(0).is_scalar_type ())
    {
      octave_stream is = octave_stream_list::lookup (args(0), "dlmread");

      input = is.input_stream ();

      if (! input)
        error ("dlmread: stream FILE not open for input");
    }
  else
    error ("dlmread: FILE argument must be a string or file id");

  // Set default separator.
  std::string sep;
  if (nargin > 1)
    {
      if (args(1).is_sq_string ())
        sep = do_string_escapes (args(1).string_value ());
      else
        sep = args(1).string_value ();
    }

  // Take a subset if a range was given.
  octave_idx_type r0 = 0;
  octave_idx_type c0 = 0;
  octave_idx_type r1 = idx_max;
  octave_idx_type c1 = idx_max;
  if (nargin > 2)
    {
      if (nargin == 3)
        {
          if (! parse_range_spec (args(2), r0, c0, r1, c1))
            error ("dlmread: error parsing RANGE");
        }
      else if (nargin == 4)
        {
          r0 = args(2).idx_type_value ();
          c0 = args(3).idx_type_value ();
        }

      if (r0 < 0 || c0 < 0)
        error ("dlmread: left & top must be positive");

      // Short-circuit and return if range is empty 
      if (r1 < r0 || c1 < c0)
        return ovl (Matrix (0,0));
    }

  octave_idx_type i = 0;
  octave_idx_type j = 0;
  octave_idx_type r = 1;
  octave_idx_type c = 1;
  octave_idx_type rmax = 0;
  octave_idx_type cmax = 0;

  Matrix rdata;
  ComplexMatrix cdata;

  bool iscmplx = false;
  bool sepflag = false;

  std::string line;

  // Skip the r0 leading lines
  octave_idx_type rcnt = r0;
  while (rcnt > 0 && getline (*input, line))
    rcnt--;

  if (rcnt > 0)
    return ovl (Matrix (0,0));  // Not enough lines in file to satisfy RANGE
  else
    r1 -= r0;

  std::istringstream tmp_stream;

  // Read in the data one field at a time, growing the data matrix
  // as needed.
  while (getline (*input, line))
    {
      // Skip blank lines for compatibility.
      if (line.find_first_not_of (" \t") == std::string::npos)
        continue;

      // To be compatible with matlab, blank separator should
      // correspond to whitespace as delimter.
      if (! sep.length ())
        {
          size_t n = line.find_first_of (",:; \t",
                                         line.find_first_of ("0123456789"));
          if (n == std::string::npos)
            {
              sep = " \t";
              sepflag = true;
            }
          else
            {
              char ch = line.at (n);

              switch (line.at (n))
                {
                case ' ':
                case '\t':
                  sepflag = true;
                  sep = " \t";
                  break;

                default:
                  sep = ch;
                  break;
                }
            }
        }

      if (cmax == 0)
        {
          // Try to estimate the number of columns.  Skip leading
          // whitespace.
          size_t pos1 = line.find_first_not_of (" \t");
          do
            {
              size_t pos2 = line.find_first_of (sep, pos1);

              if (sepflag && pos2 != std::string::npos)
                // Treat consecutive separators as one.
                {
                  pos2 = line.find_first_not_of (sep, pos2);
                  if (pos2 != std::string::npos)
                    pos2 -= 1;
                  else
                    pos2 = line.length () - 1;
                }

              cmax++;

              if (pos2 != std::string::npos)
                pos1 = pos2 + 1;
              else
                pos1 = std::string::npos;

            }
          while (pos1 != std::string::npos);

          if (iscmplx)
            cdata.resize (rmax, cmax);
          else
            rdata.resize (rmax, cmax);
        }

      r = (r > i + 1 ? r : i + 1);
      j = 0;
      // Skip leading whitespace.
      size_t pos1 = line.find_first_not_of (" \t");
      do
        {
          octave_quit ();

          size_t pos2 = line.find_first_of (sep, pos1);
          std::string str = line.substr (pos1, pos2 - pos1);

          if (sepflag && pos2 != std::string::npos)
            // Treat consecutive separators as one.
            pos2 = line.find_first_not_of (sep, pos2) - 1;

          c = (c > j + 1 ? c : j + 1);
          if (r > rmax || c > cmax)
            {
              // Use resize_and_fill for the case of not-equal
              // length rows.
              rmax = 2*r;
              cmax = c;
              if (iscmplx)
                cdata.resize (rmax, cmax);
              else
                rdata.resize (rmax, cmax);
            }

          tmp_stream.str (str);
          tmp_stream.clear ();

          double x = octave_read_double (tmp_stream);
          if (tmp_stream)
            {
              if (tmp_stream.eof ())
                {
                  if (iscmplx)
                    cdata(i,j++) = x;
                  else
                    rdata(i,j++) = x;
                }
              else if (std::toupper (tmp_stream.peek ()) == 'I')
                {
                  // This is to allow pure imaginary numbers.
                  if (iscmplx)
                    cdata(i,j++) = x;
                  else
                    rdata(i,j++) = x;
                }
              else
                {
                  double y = octave_read_double (tmp_stream);

                  if (! iscmplx && y != 0.)
                    {
                      iscmplx = true;
                      cdata = ComplexMatrix (rdata);
                    }

                  if (iscmplx)
                    cdata(i,j++) = Complex (x, y);
                  else
                    rdata(i,j++) = x;
                }
            }
          else if (iscmplx)
            cdata(i,j++) = empty_value;
          else
            rdata(i,j++) = empty_value;

          if (pos2 != std::string::npos)
            pos1 = pos2 + 1;
          else
            pos1 = std::string::npos;

        }
      while (pos1 != std::string::npos);

      if (i == r1)
        break;  // Stop early if the desired range has been read.

      i++;
    }

  // Clip selection indices to actual size of data
  if (r1 >= r)
    r1 = r - 1;
  if (c1 >= c)
    c1 = c - 1;

  if (iscmplx)
    {
      if ((i == 0 && j == 0) || (c0 > c1))
        return ovl (ComplexMatrix (0,0));

      cdata = cdata.extract (0, c0, r1, c1);
      return ovl (cdata);
    }
  else
    {
      if ((i == 0 && j == 0) || (c0 > c1))
        return ovl (Matrix (0,0));

      rdata = rdata.extract (0, c0, r1, c1);
      return ovl (rdata);
    }
}

/*
%!test
%! file = tempname ();
%! fid = fopen (file, "wt");
%! fwrite (fid, "1, 2, 3\n4, 5, 6\n7, 8, 9\n10, 11, 12");
%! fclose (fid);
%! unwind_protect
%!   assert (dlmread (file), [1, 2, 3; 4, 5, 6; 7, 8, 9;10, 11, 12]);
%!   assert (dlmread (file, ","), [1, 2, 3; 4, 5, 6; 7, 8, 9; 10, 11, 12]);
%!   assert (dlmread (file, ",", [1, 0, 2, 1]), [4, 5; 7, 8]);
%!   assert (dlmread (file, ",", "B1..C2"), [2, 3; 5, 6]);
%!   assert (dlmread (file, ",", "B1:C2"), [2, 3; 5, 6]);
%!   assert (dlmread (file, ",", "..C2"), [1, 2, 3; 4, 5, 6]);
%!   assert (dlmread (file, ",", 0, 1), [2, 3; 5, 6; 8, 9; 11, 12]);
%!   assert (dlmread (file, ",", "B1.."), [2, 3; 5, 6; 8, 9; 11, 12]);
%!   assert (dlmread (file, ",", 10, 0), []);
%!   assert (dlmread (file, ",", 0, 10), []);
%!   fail ('dlmread (file, ",", [0 1])', "error parsing RANGE");
%! unwind_protect_cleanup
%!   unlink (file);
%! end_unwind_protect

%!test
%! file = tempname ();
%! fid = fopen (file, "wt");
%! fwrite (fid, "1, 2, 3\n4+4i, 5, 6\n7, 8, 9\n10, 11, 12");
%! fclose (fid);
%! unwind_protect
%!   assert (dlmread (file), [1, 2, 3; 4 + 4i, 5, 6; 7, 8, 9; 10, 11, 12]);
%!   assert (dlmread (file, ","), [1, 2, 3; 4 + 4i, 5, 6; 7, 8, 9; 10, 11, 12]);
%!   assert (dlmread (file, ",", [1, 0, 2, 1]), [4 + 4i, 5; 7, 8]);
%!   assert (dlmread (file, ",", "A2..B3"), [4 + 4i, 5; 7, 8]);
%!   assert (dlmread (file, ",", "A2:B3"), [4 + 4i, 5; 7, 8]);
%!   assert (dlmread (file, ",", "..B3"), [1, 2; 4 + 4i, 5; 7, 8]);
%!   assert (dlmread (file, ",", 1, 0), [4 + 4i, 5, 6; 7, 8, 9; 10, 11, 12]);
%!   assert (dlmread (file, ",", "A2.."), [4 + 4i, 5, 6; 7, 8, 9; 10, 11, 12]);
%!   assert (dlmread (file, ",", 10, 0), []);
%!   assert (dlmread (file, ",", 0, 10), []);
%!   assert (dlmread (file, ",", [0, 4, 0, Inf]), []);
%! unwind_protect_cleanup
%!   unlink (file);
%! end_unwind_protect

*/
