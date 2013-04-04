/*

Copyright (C) 2013 John W. Eaton
Copyright (C) 2011-2012 Jacob Dawid
Copyright (C) 2011-2012 John P. Swensen

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

#ifndef OCTAVE_QT_LINK_H
#define OCTAVE_QT_LINK_H

#include <string>

class octave_mutex;

#include "oct-obj.h"

#include "event-queue.h"

#include "octave-link.h"
#include "octave-main-thread.h"
#include "octave-event-listener.h"

// \class OctaveLink
// \brief Provides threadsafe access to octave.
// \author Jacob Dawid
//
// This class is a wrapper around octave and provides thread safety by
// buffering access operations to octave and executing them in the
// readline event hook, which lives in the octave thread.

class octave_qt_link : public octave_link
{
public:

  octave_qt_link (void);

  ~octave_qt_link (void) { }

  void do_update_workspace (void);

  void do_update_history (void);

  void do_insert_debugger_pointer (const std::string& file, int line);
  void do_delete_debugger_pointer (const std::string& file, int line);

  void do_pre_input_event (void);
  void do_post_input_event (void);

  void do_enter_debugger_event (const std::string& file, int line);
  void do_exit_debugger_event (const std::string& file, int line);

  void do_update_breakpoint (bool insert, const std::string& file, int line);

  void do_edit_file (const octave_value_list& args);

private:

  // No copying!

  octave_qt_link (const octave_qt_link&);

  octave_qt_link& operator = (const octave_qt_link&);

  // Thread running octave_main.
  octave_main_thread *main_thread;
};

#endif // OCTAVELINK_H
