////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011-2020 The Octave Project Developers
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

#if ! defined (octave_event_manager_h)
#define octave_event_manager_h 1

#include "octave-config.h"

#include <functional>
#include <list>
#include <memory>
#include <string>

#include "oct-mutex.h"
#include "octave.h"
#include "event-queue.h"
#include "uint8NDArray.h"

class octave_value;
class string_vector;

namespace octave
{
  typedef std::function<void (void)> fcn_callback;
  typedef std::function<void (octave::interpreter&)> meth_callback;

  class symbol_info_list;

  // The methods in this class provide a way to pass signals to the GUI
  // thread.  A GUI that wishes to act on these events should derive
  // from this class and perform actions in a thread-safe way.  In
  // Octave's Qt-based GUI, for example, these functions are all
  // implemented as wrappers around Qt signals that trigger actions in
  // the GUI.  The Qt signal/slot mechanism ensures that the actions are
  // properly queued for execution when the objects corresponding to the
  // signal and slot belong to different threads.
  //
  // These functions should not be called directly.  Instead all
  // requests from the interpreter for GUI actions should be done
  // through the event_manager class.  That class checks to ensure that
  // the GUI is connected and enabled before calling these virtual
  // functions.

  // FIXME: it would be nice if instead of requiring the GUI to derive
  // from this class, it could subscribe to individual events, possibly
  // multiple times.  In that way, it would be more flexible and
  // decentralized, similar to the Qt signal/slot connection mechanism
  // and would allow the GUI to connect multiple signals to a single
  // action or multiple actions to a single signal.

  // FIXME: audit this list of functions and determine whether they are
  // all necessary and whether there might be better names for them.

  class interpreter_events
  {
  public:

    interpreter_events (void) = default;

    interpreter_events (const interpreter_events&) = default;

    interpreter_events& operator = (const interpreter_events&) = default;

    virtual ~interpreter_events (void) = default;

    // Dialogs.

    typedef std::list<std::pair<std::string, std::string>> filter_list;

    virtual std::list<std::string>
    file_dialog (const filter_list& /*filter*/,
                 const std::string& /*title*/,
                 const std::string& /*filename*/,
                 const std::string& /*dirname*/,
                 const std::string& /*multimode*/)
    {
      return std::list<std::string> ();
    }

    virtual std::list<std::string>
    input_dialog (const std::list<std::string>& /*prompt*/,
                  const std::string& /*title*/,
                  const std::list<float>& /*nr*/,
                  const std::list<float>& /*nc*/,
                  const std::list<std::string>& /*defaults*/)
    {
      return std::list<std::string> ();
    }

    virtual std::pair<std::list<int>, int>
    list_dialog (const std::list<std::string>& /*list*/,
                 const std::string& /*mode*/, int /*width*/, int /*height*/,
                 const std::list<int>& /*initial_value*/,
                 const std::string& /*name*/,
                 const std::list<std::string>& /*prompt*/,
                 const std::string& /*ok_string*/,
                 const std::string& /*cancel_string*/)
    {
      return std::pair<std::list<int>, int> ();
    }

    virtual std::string
    question_dialog (const std::string& /*msg*/, const std::string& /*title*/,
                     const std::string& /*btn1*/, const std::string& /*btn2*/,
                     const std::string& /*btn3*/, const std::string& /*btndef*/)
    {
      return "";
    }

    virtual void update_path_dialog (void) {  }

    virtual void show_preferences (void) { }

    virtual void apply_preferences (void) { }

    virtual void show_doc (const std::string& /*file*/) { }

    virtual bool edit_file (const std::string& /*file*/) { return false; }

    virtual void
    edit_variable (const std::string& /*name*/, const octave_value& /*val*/)
    { }

    // Other requests for user interaction, usually some kind of
    // confirmation before another action.  Could these be reformulated
    // using the question_dialog action?

    virtual bool confirm_shutdown (void) { return false; }

    virtual bool prompt_new_edit_file (const std::string& /*file*/)
    {
      return false;
    }

    virtual int
    debug_cd_or_addpath_error (const std::string& /*file*/,
                               const std::string& /*dir*/,
                               bool /*addpath_option*/)
    {
      return -1;
    }

    // Requests for information normally stored in the GUI.

    virtual uint8NDArray get_named_icon (const std::string& /*icon_name*/)
    {
      return uint8NDArray ();
    }

    virtual std::string gui_preference (const std::string& /*key*/,
                                        const std::string& /*value*/)
    {
      return "";
    }

    // Requests for GUI action that do not require user interaction.
    // These are different from other notifications in that they are not
    // associated with changes in the interpreter state (like a change
    // in the current working directory or command history).

    virtual bool copy_image_to_clipboard (const std::string& /*file*/)
    {
      return false;
    }

    virtual void focus_window (const std::string /*win_name*/)
    { }

    virtual void
    execute_command_in_terminal (const std::string& /*command*/) { }

    virtual void register_doc (const std::string& /*file*/) { }

    virtual void unregister_doc (const std::string& /*file*/) { }

    virtual void update_gui_lexer (void) { }

    // Notifications of events in the interpreter that a GUI will
    // normally wish to respond to.

    virtual void directory_changed (const std::string& /*dir*/) { }

    virtual void
    file_remove (const std::string& /*old_nm*/, const std::string& /*new_nm*/)
    { }

    virtual void file_renamed (bool) { }

    virtual void
    set_workspace (bool /*top_level*/, bool /*debug*/,
                   const octave::symbol_info_list& /*syminfo*/,
                   bool /*update_variable_editor*/)
    { }

    virtual void clear_workspace (void) { }

    virtual void set_history (const string_vector& /*hist*/) { }

    virtual void append_history (const std::string& /*hist_entry*/) { }

    virtual void clear_history (void) { }

    virtual void pre_input_event (void) { }

    virtual void post_input_event (void) { }

    virtual void
    enter_debugger_event (const std::string& /*fcn_name*/,
                          const std::string& /*fcn_file_name*/,
                          int /*line*/)
    { }

    virtual void
    execute_in_debugger_event (const std::string& /*file*/, int /*line*/) { }

    virtual void exit_debugger_event (void) { }

    virtual void
    update_breakpoint (bool /*insert*/, const std::string& /*file*/,
                       int /*line*/, const std::string& /*cond*/)
    { }
  };

  //! Provides threadsafe access to octave.
  //!
  //! This class provides thread-safe communication between the
  //! interpreter and a GUI.

  class event_manager
  {
  public:

    event_manager (interpreter& interp);

    // No copying!

    event_manager (const event_manager&) = delete;

    event_manager&
    operator = (const event_manager&) = delete;

    virtual ~event_manager (void);

    // OBJ should be an object of a class that is derived from the base
    // class interpreter_events, or nullptr to disconnect and delete the
    // previous link.

    void connect_link (const std::shared_ptr<interpreter_events>& obj);

    bool enable (void);

    bool disable (void)
    {
      bool retval = link_enabled;
      link_enabled = false;
      return retval;
    }

    bool enabled (void) const
    {
      return link_enabled;
    }

    // If disable is TRUE, then no additional events will be processed
    // other than exit.

    void process_events (bool disable = false);

    void discard_events (void);

    // The post_event and post_exception functions provide a thread-safe
    // way for the GUI to queue interpreter functions for execution.
    // The queued functions are executed when the interpreter is
    // otherwise idle.

    void post_event (const fcn_callback& fcn)
    {
      if (enabled ())
        gui_event_queue.add (fcn);
    }

    void post_event (const meth_callback& meth)
    {
      if (enabled ())
        gui_event_queue.add (std::bind (meth, std::ref (m_interpreter)));
    }

    // The following functions correspond to the virtual fuunctions in
    // the interpreter_events class.  They provide a way for the
    // interpreter to notify the GUI that some event has occurred
    // (directory or workspace changed, for example) or to request the
    // GUI to perform some action (display a dialog, for example).

    // Please keep this list of declarations in the same order as the
    // ones above in the interpreter_events class.

    typedef std::list<std::pair<std::string, std::string>> filter_list;

    std::list<std::string>
    file_dialog (const filter_list& filter, const std::string& title,
                 const std::string& filename, const std::string& dirname,
                 const std::string& multimode)
    {
      return (enabled ()
              ? instance->file_dialog (filter, title, filename, dirname,
                                       multimode)
              : std::list<std::string> ());
    }

    std::list<std::string>
    input_dialog (const std::list<std::string>& prompt,
                  const std::string& title,
                  const std::list<float>& nr,
                  const std::list<float>& nc,
                  const std::list<std::string>& defaults)
    {
      return (enabled ()
              ? instance->input_dialog (prompt, title, nr, nc, defaults)
              : std::list<std::string> ());
    }

    std::pair<std::list<int>, int>
    list_dialog (const std::list<std::string>& list,
                 const std::string& mode,
                 int width, int height,
                 const std::list<int>& initial_value,
                 const std::string& name,
                 const std::list<std::string>& prompt,
                 const std::string& ok_string,
                 const std::string& cancel_string)
    {
      return (enabled ()
              ? instance->list_dialog (list, mode, width, height,
                                       initial_value, name, prompt,
                                       ok_string, cancel_string)
              : std::pair<std::list<int>, int> ());
    }

    std::string
    question_dialog (const std::string& msg, const std::string& title,
                     const std::string& btn1, const std::string& btn2,
                     const std::string& btn3, const std::string& btndef)
    {
      return (enabled ()
              ? instance->question_dialog (msg, title, btn1,
                                           btn2, btn3, btndef)
              : "");
    }

    void update_path_dialog (void)
    {
      if (octave::application::is_gui_running () && enabled ())
        instance->update_path_dialog ();
    }

    bool show_preferences (void)
    {
      if (enabled ())
        {
          instance->show_preferences ();
          return true;
        }
      else
        return false;
    }

    bool apply_preferences (void)
    {
      if (enabled ())
        {
          instance->apply_preferences ();
          return true;
        }
      else
        return false;
    }

    bool show_doc (const std::string& file)
    {
      if (enabled ())
        {
          instance->show_doc (file);
          return true;
        }
      else
        return false;
    }

    bool edit_file (const std::string& file)
    {
      return enabled () ? instance->edit_file (file) : false;
    }

    bool edit_variable (const std::string& name, const octave_value& val)
    {
      if (enabled ())
        {
          instance->edit_variable (name, val);
          return true;
        }
      else
        return false;
    }

    bool confirm_shutdown (void)
    {
      bool retval = true;

      if (enabled ())
        retval = instance->confirm_shutdown ();

      return retval;
    }

    bool prompt_new_edit_file (const std::string& file)
    {
      return enabled () ? instance->prompt_new_edit_file (file) : false;
    }

    int debug_cd_or_addpath_error (const std::string& file,
                                   const std::string& dir, bool addpath_option)
    {
      return (enabled ()
              ? instance->debug_cd_or_addpath_error (file, dir, addpath_option)
              : 0);
    }

    uint8NDArray get_named_icon (const std::string& icon_name)
    {
      return (enabled ()
              ? instance->get_named_icon (icon_name) : uint8NDArray ());
    }

    std::string gui_preference (const std::string& key,
                                const std::string& value)
    {
      return enabled () ? instance->gui_preference (key, value) : "";
    }

    bool copy_image_to_clipboard (const std::string& file)
    {
      return enabled () ? instance->copy_image_to_clipboard (file) : false;
    }

    virtual void focus_window (const std::string win_name)
    {
      if (enabled ())
        instance->focus_window (win_name);
    }

    // Preserves pending input.
    void execute_command_in_terminal (const std::string& command)
    {
      if (enabled ())
        instance->execute_command_in_terminal (command);
    }

    bool register_doc (const std::string& file)
    {
      if (enabled ())
        {
          instance->register_doc (file);
          return true;
        }
      else
        return false;
    }

    bool unregister_doc (const std::string& file)
    {
      if (enabled ())
        {
          instance->unregister_doc (file);
          return true;
        }
      else
        return false;
    }

    bool update_gui_lexer (void)
    {
      if (enabled ())
        {
          instance->update_gui_lexer ();
          return true;
        }
      else
        return false;
    }

    void directory_changed (const std::string& dir)
    {
      if (enabled ())
        instance->directory_changed (dir);
    }

    // Methods for removing/renaming files which might be open in editor
    void file_remove (const std::string& old_name, const std::string& new_name)
    {
      if (octave::application::is_gui_running () && enabled ())
        instance->file_remove (old_name, new_name);
    }

    void file_renamed (bool load_new)
    {
      if (octave::application::is_gui_running () && enabled ())
        instance->file_renamed (load_new);
    }

    void set_workspace (void);

    void set_workspace (bool top_level, const octave::symbol_info_list& syminfo,
                        bool update_variable_editor = true)
    {
      if (enabled ())
        instance->set_workspace (top_level, debugging, syminfo,
                                 update_variable_editor);
    }

    void clear_workspace (void)
    {
      if (enabled ())
        instance->clear_workspace ();
    }

    void set_history (const string_vector& hist)
    {
      if (enabled ())
        instance->set_history (hist);
    }

    void append_history (const std::string& hist_entry)
    {
      if (enabled ())
        instance->append_history (hist_entry);
    }

    void clear_history (void)
    {
      if (enabled ())
        instance->clear_history ();
    }

    void pre_input_event (void)
    {
      if (enabled ())
        instance->pre_input_event ();
    }

    void post_input_event (void)
    {
      if (enabled ())
        instance->post_input_event ();
    }

    void enter_debugger_event (const std::string& fcn_name,
                               const std::string& fcn_file_name, int line)
    {
      if (enabled ())
        {
          debugging = true;

          instance->enter_debugger_event (fcn_name, fcn_file_name, line);
        }
    }

    void execute_in_debugger_event (const std::string& file, int line)
    {
      if (enabled ())
        instance->execute_in_debugger_event (file, line);
    }

    void exit_debugger_event (void)
    {
      if (enabled () && debugging)
        {
          debugging = false;

          instance->exit_debugger_event ();
        }
    }

    void update_breakpoint (bool insert, const std::string& file,
                            int line, const std::string& cond = "")
    {
      if (enabled ())
        instance->update_breakpoint (insert, file, line, cond);
    }

  private:

    interpreter& m_interpreter;

    // Using a shared_ptr to manage the link_events object ensures that it
    // will be valid until it is no longer needed.

    std::shared_ptr<interpreter_events> instance;

  protected:

    // Semaphore to lock access to the event queue.
    octave::mutex *event_queue_mutex;

    // Event Queue.
    octave::event_queue gui_event_queue;

    bool debugging;
    bool link_enabled;
  };
}

#endif
