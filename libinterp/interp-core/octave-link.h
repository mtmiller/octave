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

#if !defined (octave_link_h)
#define octave_link_h 1

#include <string>

class octave_mutex;

#include "oct-obj.h"

#include "event-queue.h"

#include "octave-event-listener.h"

// \class OctaveLink
// \brief Provides threadsafe access to octave.
// \author Jacob Dawid
//
// This class is a wrapper around octave and provides thread safety by
// buffering access operations to octave and executing them in the
// readline event hook, which lives in the octave thread.

class octave_link
{
protected:

  octave_link (void);

public:

  virtual ~octave_link (void) { }

  static void register_event_listener (octave_event_listener *el)
  {
    if (instance_ok ())
      instance->do_register_event_listener (el);
  }

  static void generate_events (void)
  {
    if (instance_ok ())
      instance->do_generate_events ();
  }

  static void process_events (void)
  {
    if (instance_ok ())
      instance->do_process_events ();
  }

  template <class T>
  static void post_event (T *obj, void (T::*method) (void))
  {
    if (instance_ok ())
      instance->do_post_event (obj, method);
  }

  template <class T, class A>
  static void post_event (T *obj, void (T::*method) (A), A arg)
  {
    if (instance_ok ())
      instance->do_post_event (obj, method, arg);
  }

  template <class T, class A>
  static void post_event (T *obj, void (T::*method) (const A&), const A& arg)
  {
    if (instance_ok ())
      instance->do_post_event (obj, method, arg);
  }

  static void about_to_exit (void)
  {
    if (instance_ok ())
      instance->do_about_to_exit ();
  }

  static void entered_readline_hook (void)
  {
    if (instance_ok ())
      instance->do_entered_readline_hook ();
  }

  static void finished_readline_hook (void)
  {
    if (instance_ok ())
      instance->do_finished_readline_hook ();
  }

  static std::string last_working_directory (void)
  {
    return instance_ok ()
      ? instance->do_last_working_directory () : std::string ();
  }

  static void update_workspace (void)
  {
    if (instance_ok ())
      instance->do_update_workspace ();
  }

  static void update_history (void)
  {
    if (instance_ok ())
      instance->do_update_history ();
  }

  static void pre_input_event (void)
  {
    if (instance_ok ())
      instance->do_pre_input_event ();
  }

  static void post_input_event (void)
  {
    if (instance_ok ())
      instance->do_post_input_event ();
  }

  static void enter_debugger_event (const std::string& file, int line)
  {
    if (instance_ok ())
      instance->do_enter_debugger_event (file, line);
  }

  static void exit_debugger_event (const std::string& file, int line)
  {
    if (instance_ok ())
      instance->do_exit_debugger_event (file, line);
  }

  static void
  update_breakpoint (bool insert, const std::string& file, int line)
  {
    if (instance_ok ())
      instance->do_update_breakpoint (insert, file, line);
  }

  static void
  edit_file (const octave_value_list& args)
  {
    if (instance_ok ())
      instance->do_edit_file (args);
  }

  static void connect (octave_link *);

private:

  static octave_link *instance;

  static void cleanup_instance (void) { delete instance; instance = 0; }

  // No copying!

  octave_link (const octave_link&);

  octave_link& operator = (const octave_link&);

  static bool instance_ok (void);

protected:

  octave_event_listener *event_listener;

  // Semaphore to lock access to the event queue.
  octave_mutex *event_queue_mutex;

  // Event Queue.
  event_queue gui_event_queue;

  // Stores the last known current working directory of octave.
  std::string last_cwd;

  bool debugging;

  void do_register_event_listener (octave_event_listener *oel);

  void do_generate_events (void);
  void do_process_events (void);

  template <class T>
  void do_post_event (T *obj, void (T::*method) (void))
  {
    gui_event_queue.add_method (obj, method);
  }

  template <class T, class A>
  void do_post_event (T *obj, void (T::*method) (A), A arg)
  {
    gui_event_queue.add_method (obj, method, arg);
  }

  template <class T, class A>
  void do_post_event (T *obj, void (T::*method) (const A&), const A& arg)
  {
    gui_event_queue.add_method (obj, method, arg);
  }

  void do_about_to_exit (void);

  void do_entered_readline_hook (void) { }
  void do_finished_readline_hook (void) { }

  std::string do_last_working_directory (void);

  virtual void do_update_workspace (void) = 0;

  virtual void do_update_history (void) = 0;

  virtual void
  do_insert_debugger_pointer (const std::string& file, int line) = 0;

  virtual void
  do_delete_debugger_pointer (const std::string& file, int line) = 0;

  virtual void do_pre_input_event (void) = 0;
  virtual void do_post_input_event (void) = 0;

  virtual void do_enter_debugger_event (const std::string& file, int line) = 0;
  virtual void do_exit_debugger_event (const std::string& file, int line) = 0;

  virtual void do_update_breakpoint (bool insert,
                                     const std::string& file, int line) = 0;

  virtual void do_edit_file (const octave_value_list& args) = 0;
};

#endif // OCTAVELINK_H
