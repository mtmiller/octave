// utils.h                                               -*- C++ -*-
/*

Copyright (C) 1996 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if !defined (octave_utils_h)
#define octave_utils_h 1

class istream;
class ostrstream;

#include <string>

class Octave_object;
class string_vector;

extern char *strsave (const char *);

extern string search_path_for_file (const string&, const string&);
extern string file_in_path (const string&, const string&);
extern string fcn_file_in_path (const string&);
extern string oct_file_in_path (const string&);

extern "C" void jump_to_top_level (void) NORETURN;

extern int almost_match (const string& std, const string& s,
			 int min_match_len = 1, int case_sens = 1);

extern int
keyword_almost_match (const char **std, int *min_len,
		      const string& s, int min_toks_to_match,
		      int max_toks);

extern string_vector get_fcn_file_names (const string&, int = 0);

extern string_vector get_fcn_file_names (int = 0);

extern int NINT (double x);
extern double D_NINT (double x);

extern string_vector make_argv (const Octave_object&, const string&);

extern int empty_arg (const char *name, int nr, int nc);

extern string undo_string_escapes (const string& s);

extern void oct_putenv (const char *, const char *);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
