// sysdep.h                                              -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

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

#if !defined (octave_sysdep_h)
#define octave_sysdep_h 1

extern void sysdep_init (void);

extern void raw_mode (int);
extern int kbhit (void);

extern char *octave_getcwd (char *, int);
extern int octave_chdir (const char *);

extern "C"
{
extern int gethostname ();
}

// Octave's idea of infinity.
extern double octave_Inf;

// Octave's idea of not a number.
extern double octave_NaN;

enum floating_point_format
  {
    OCTAVE_IEEE_LITTLE,
    OCTAVE_IEEE_BIG,
    OCTAVE_VAX_D,
    OCTAVE_VAX_G,
    OCTAVE_CRAY,
    OCTAVE_UNKNOWN_FLT_FMT,
  };

// The floating point format on this system.
extern floating_point_format native_float_format;

// Nonzero if the machine we are running on is big-endian.
extern int octave_words_big_endian;

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
