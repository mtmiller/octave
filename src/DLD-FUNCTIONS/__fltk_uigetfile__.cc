/*

Copyright (C) 2010-2011 Kai Habel

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined (HAVE_FLTK)

#include <FL/Fl.H>
#include <Fl/Fl_File_Chooser.H>
#include "defun-dld.h"
#include "file-ops.h"


DEFUN_DLD (__fltk_uigetfile__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __fltk_uigetfile__ (@dots{})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  
  // Expected argument list
  // args(0) ... FileFilter in fltk format
  // args(1) ... Title
  // args(2) ... Default Filename
  // args(3) ... PostionValue [x,y]
  // args(4) ... SelectValue "on"/"off"/"dir"/"create"

  octave_value_list fargs, retval;

  std::string file_filter = args(0).string_value();
  std::string title = args(1).string_value();
  std::string default_name = args(2).string_value();
  Matrix pos = args(3).matrix_value();

  int multi_type = Fl_File_Chooser::SINGLE;
  std::string flabel = "Filename:";
  
  std::string multi = args(4).string_value();
  if (multi == "on")
    multi_type = Fl_File_Chooser::MULTI;
  else if (multi == "dir")
    {
      multi_type = Fl_File_Chooser::DIRECTORY;
      flabel = "Directory:";
    }
  else if (multi == "create")
    multi_type = Fl_File_Chooser::CREATE;

  Fl_File_Chooser::filename_label = flabel.c_str ();
  Fl_File_Chooser *fc = new Fl_File_Chooser (default_name.c_str (), file_filter.c_str (), multi_type, title.c_str ());
  fc->preview (0);

  if (multi_type == Fl_File_Chooser::CREATE)
    fc->ok_label ("Save");

  fc->show ();

  while (fc->shown ())
    Fl::wait ();

  retval(0) = octave_value(0);
  retval(1) = octave_value(0);
  retval(2) = octave_value(0);  

  if (fc->value())
    {
      int file_count = fc->count ();
      std::string fname;
      std::string sep = file_ops::dir_sep_str ();
      std::size_t idx;

      if (file_count == 1 && multi_type != Fl_File_Chooser::DIRECTORY)
        {
          fname = fc->value ();
          idx = fname.find_last_of (sep);
          retval(0) = fname.substr (idx + 1);
        }
      else
        {
          Cell file_cell = Cell(file_count, 1);
          for (octave_idx_type n = 1; n <= file_count; n++)
            {
              fname = fc->value (n);
              idx = fname.find_last_of (sep);
              file_cell(n - 1) = fname.substr (idx + 1);
            }
          retval(0) = file_cell;
        }

      if (multi_type == Fl_File_Chooser::DIRECTORY)
        retval(0) = std::string (fc->value ());
      else
        {
          retval(1) = std::string (fc->directory ()) + sep;
          retval(2) = fc->filter_value ();
        }
    }

  fc->hide ();
  Fl::flush ();
  delete fc;

  return retval;
}

#endif
