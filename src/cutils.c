/*

Copyright (C) 1999 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (__WIN32__) && ! defined (_POSIX_VERSION)

#include <windows.h>

#else

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <unistd.h>
#endif

#include "systime.h"

#ifdef HAVE_POLL_H
#include <poll.h>
#elif HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#endif

void
octave_sleep (unsigned int seconds)
{
#if defined (__WIN32__) && ! defined (_POSIX_VERSION)
  Sleep (1000 * seconds);
#else
  sleep (seconds);
#endif
}

void
octave_usleep (unsigned int useconds)
{
  unsigned int sec = useconds / 1000000;
  unsigned int usec = useconds % 1000000;

  if (sec > 0)
    octave_sleep (sec);

#if defined (__WIN32__) && ! defined (_POSIX_VERSION)

  /* Round to the nearest millisecond, with a minimum of 1 millisecond
     if usleep was called with a a non-zero value.  */

  if (usec > 500)
    Sleep ((usec+500)/1000);
  else if (usec > 0)
    Sleep (1);
  else
    Sleep (0);

#elif defined (HAVE_USLEEP)

  usleep (usec);

#elif defined (HAVE_SELECT)

  struct timeval delay;

  delay.tv_sec = 0;
  delay.tv_usec = usec;

  select (0, 0, 0, 0, &delay);

#elif defined (HAVE_POLL)

  struct pollfd pfd;
  int delay = usec / 1000;

  if (delay > 0)
    poll (&fd, 0, delay);

#endif
}

int
octave_strcasecmp (const char *s1, const char *s2)
{
  return strcasecmp (s1, s2);
}

int
octave_strncasecmp (const char *s1, const char *s2, size_t n)
{
  return strncasecmp (s1, s2, n);
}

// We manage storage.  User should not free it, and its contents are
// only valid until next call to vsnprintf.

#if defined __GNUC__ && __GNUC__ >= 3
#define HAVE_C99_VSNPRINTF 1
#endif

char *
octave_vsnprintf (const char *fmt, va_list args)
{
#if defined (HAVE_VSNPRINTF)
  static size_t size = 100;

  static char *buf = 0;

  int nchars;

  if (! buf)
    buf = malloc (size);

#if defined (HAVE_C99_VSNPRINTF)

  nchars = vsnprintf (buf, size, fmt, args);

  if (nchars >= size)
    {
      size = nchars + 1;
      buf = realloc (buf, size);

      if (buf)
	vsnprintf (buf, size, fmt, args);

      return buf;
    }

#else

  while (1)
    {
      nchars = vsnprintf (buf, size, fmt, args);

      if (nchars > -1)
	return buf;
      else
	{
	  size *= 2;

	  buf = realloc (buf, size);

	  if (! buf)
	    return 0;
	}
    }

#endif

#else
  return 0;
#endif
}

char *
octave_snprintf (const char *fmt, ...)
{
  char *retval = 0;

  va_list args;
  va_start (args, fmt);

  retval = octave_vsnprintf (fmt, args);

  va_end (args);

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
