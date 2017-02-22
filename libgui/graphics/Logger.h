/*

Copyright (C) 2011-2017 Michael Goffioul

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

#if ! defined (octave_Logger_h)
#define octave_Logger_h 1

#include <cstdarg>

class QMutex;

namespace QtHandles
{

  class Logger
  {
  public:
    static void debug (const char* fmt, ...);

  private:
    bool m_debugEnabled;

    static Logger* s_instance;
    static QMutex* s_mutex;

  private:
    Logger (void);
    ~Logger (void);

    static Logger* instance (void);

    void debugV (const char* fmt, va_list arg);
  };

}

#endif
