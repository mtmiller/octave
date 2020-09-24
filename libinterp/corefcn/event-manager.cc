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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "builtin-defun-decls.h"
#include "cmd-edit.h"
#include "defun.h"
#include "event-manager.h"
#include "interpreter.h"
#include "interpreter-private.h"
#include "oct-env.h"
#include "oct-mutex.h"
#include "ovl.h"
#include "pager.h"
#include "syminfo.h"

namespace octave
{
  static int readline_event_hook (void)
  {
    event_manager& evmgr = __get_event_manager__ ("octave_readline_hook");

    evmgr.process_events ();

    return 0;
  }

  event_manager::event_manager (interpreter& interp)
    : m_interpreter (interp), instance (nullptr),
      event_queue_mutex (new mutex ()), gui_event_queue (),
      debugging (false), link_enabled (false)
  {
    command_editor::add_event_hook (readline_event_hook);
  }

  event_manager::~event_manager (void)
  {
    delete event_queue_mutex;
  }

  // Programming Note: It is possible to disable the link without deleting
  // the connection.  This allows it to be temporarily disabled.  But if
  // the link is removed, we also set the link_enabled flag to false
  // because if there is no link, it can't be enabled.  Also, access to
  // instance is only protected by a check on the link_enabled flag.

  void
  event_manager::connect_link (const std::shared_ptr<interpreter_events>& obj)
  {
    if (! obj)
      disable ();

    instance = obj;
  }

  bool event_manager::enable (void)
  {
    bool retval = link_enabled;

    if (instance)
      link_enabled = true;
    else
      warning ("event_manager: must have connected link to enable");

    return retval;
  }

  void event_manager::process_events (bool disable_flag)
  {
    if (enabled ())
      {
        if (disable_flag)
          disable ();

        event_queue_mutex->lock ();

        gui_event_queue.run ();

        event_queue_mutex->unlock ();
      }
  }

  void event_manager::discard_events (void)
  {
    if (enabled ())
      {
        event_queue_mutex->lock ();

        gui_event_queue.discard ();

        event_queue_mutex->unlock ();
      }
  }

  void event_manager::set_workspace (void)
  {
    if (enabled ())
      {
        tree_evaluator& tw = m_interpreter.get_evaluator ();

        instance->set_workspace (tw.at_top_level (), debugging,
                                 tw.get_symbol_info (), true);
      }
  }
}

DEFMETHOD (__event_manager_enabled__, interp, , ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_enabled__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.enabled ());
}

DEFMETHOD (__event_manager_edit_file__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_edit_file__ (@var{file})
Undocumented internal function.
@end deftypefn */)
{
  octave_value retval;

  octave::event_manager& evmgr = interp.get_event_manager ();

  if (args.length () == 1)
    {
      std::string file
        = args(0).xstring_value ("first argument must be filename");

      octave::flush_stdout ();

      retval = evmgr.edit_file (file);
    }
  else if (args.length () == 2)
    {
      std::string file
        = args(0).xstring_value ("first argument must be filename");

      octave::flush_stdout ();

      retval = evmgr.prompt_new_edit_file (file);
    }

  return retval;
}

DEFMETHOD (__event_manager_question_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_question_dialog__ (@var{msg}, @var{title}, @var{btn1}, @var{btn2}, @var{btn3}, @var{default})
Undocumented internal function.
@end deftypefn */)
{
  octave_value retval;

  if (args.length () == 6)
    {
      std::string msg = args(0).xstring_value ("invalid arguments");
      std::string title = args(1).xstring_value ("invalid arguments");
      std::string btn1 = args(2).xstring_value ("invalid arguments");
      std::string btn2 = args(3).xstring_value ("invalid arguments");
      std::string btn3 = args(4).xstring_value ("invalid arguments");
      std::string btndef = args(5).xstring_value ("invalid arguments");

      octave::flush_stdout ();

      octave::event_manager& evmgr = interp.get_event_manager ();

      retval = evmgr.question_dialog (msg, title, btn1, btn2, btn3, btndef);
    }

  return retval;
}

DEFMETHOD (__event_manager_file_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_file_dialog__ (@var{filterlist}, @var{title}, @var{filename}, @var{size} @var{multiselect}, @var{pathname})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 6)
    return ovl ();

  octave_value_list retval (3);

  const Array<std::string> flist = args(0).cellstr_value ();
  std::string title = args(1).string_value ();
  std::string filename = args(2).string_value ();
  Matrix pos = args(3).matrix_value ();
  std::string multi_on = args(4).string_value (); // on, off, create
  std::string pathname = args(5).string_value ();

  octave_idx_type nel;

  octave::event_manager::filter_list filter_lst;

  for (octave_idx_type i = 0; i < flist.rows (); i++)
    filter_lst.push_back (std::make_pair (flist(i,0),
                                          (flist.columns () > 1
                                           ? flist(i,1) : "")));

  octave::flush_stdout ();

  octave::event_manager& evmgr = interp.get_event_manager ();

  std::list<std::string> items_lst
    = evmgr.file_dialog (filter_lst, title, filename, pathname, multi_on);

  nel = items_lst.size ();

  // If 3, then retval is filename, directory, and selected index.
  if (nel <= 3)
    {
      if (items_lst.front ().empty ())
        retval = ovl (octave_value (0.), octave_value (0.), octave_value (0.));
      else
        {
          int idx = 0;
          for (auto& str : items_lst)
            {
              if (idx != 2)
                retval(idx++) = str;
              else
                retval(idx++) = atoi (str.c_str ());
            }
        }
    }
  else
    {
      // Multiple files.
      nel -= 2;
      Cell items (dim_vector (1, nel));

      auto it = items_lst.begin ();

      for (int idx = 0; idx < nel; idx++, it++)
        items.xelem (idx) = *it;

      retval = ovl (items, *it++, atoi (it->c_str ()));
    }

  return retval;
}

DEFMETHOD (__event_manager_list_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_list_dialog__ (@var{list}, @var{mode}, @var{size}, @var{initial}, @var{name}, @var{prompt}, @var{ok_string}, @var{cancel_string})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 8)
    return ovl ();

  Cell list = args(0).cell_value ();
  const Array<std::string> tlist = list.cellstr_value ();
  octave_idx_type nel = tlist.numel ();
  std::list<std::string> list_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    list_lst.push_back (tlist(i));

  std::string mode = args(1).string_value ();

  Matrix size_matrix = args(2).matrix_value ();
  int width = size_matrix(0);
  int height = size_matrix(1);

  Matrix initial_matrix = args(3).matrix_value ();
  nel = initial_matrix.numel ();
  std::list<int> initial_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    initial_lst.push_back (initial_matrix(i));

  std::string name = args(4).string_value ();
  list = args(5).cell_value ();
  const Array<std::string> plist = list.cellstr_value ();
  nel = plist.numel ();
  std::list<std::string> prompt_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    prompt_lst.push_back (plist(i));
  std::string ok_string = args(6).string_value ();
  std::string cancel_string = args(7).string_value ();

  octave::flush_stdout ();

  octave::event_manager& evmgr = interp.get_event_manager ();

  std::pair<std::list<int>, int> result
    = evmgr.list_dialog (list_lst, mode, width, height, initial_lst,
                         name, prompt_lst, ok_string, cancel_string);

  std::list<int> items_lst = result.first;
  nel = items_lst.size ();
  Matrix items (dim_vector (1, nel));
  octave_idx_type i = 0;
  for (const auto& int_el : items_lst)
    items.xelem(i++) = int_el;

  return ovl (items, result.second);
}

DEFMETHOD (__event_manager_input_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_input_dialog__ (@var{prompt}, @var{title}, @var{rowscols}, @var{defaults})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 4)
    return ovl ();

  Cell prompt = args(0).cell_value ();
  Array<std::string> tmp = prompt.cellstr_value ();
  octave_idx_type nel = tmp.numel ();
  std::list<std::string> prompt_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    prompt_lst.push_back (tmp(i));

  std::string title = args(1).string_value ();

  Matrix rc = args(2).matrix_value ();
  nel = rc.rows ();
  std::list<float> nr;
  std::list<float> nc;
  for (octave_idx_type i = 0; i < nel; i++)
    {
      nr.push_back (rc(i,0));
      nc.push_back (rc(i,1));
    }

  Cell defaults = args(3).cell_value ();
  tmp = defaults.cellstr_value ();
  nel = tmp.numel ();
  std::list<std::string> defaults_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    defaults_lst.push_back (tmp(i));

  octave::flush_stdout ();

  octave::event_manager& evmgr = interp.get_event_manager ();

  std::list<std::string> items_lst
    = evmgr.input_dialog (prompt_lst, title, nr, nc, defaults_lst);

  nel = items_lst.size ();
  Cell items (dim_vector (nel, 1));
  octave_idx_type i = 0;
  for (const auto& str_el : items_lst)
    items.xelem(i++) = str_el;

  return ovl (items);
}


DEFMETHOD (__event_manager_named_icon__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_dialog_icons__ (@var{icon_name})
Undocumented internal function.
@end deftypefn */)
{
  uint8NDArray retval;

  if (args.length () > 0)
    {
      std::string icon_name = args(0).xstring_value ("invalid arguments");

      octave::event_manager& evmgr = interp.get_event_manager ();

      retval = evmgr.get_named_icon (icon_name);
    }

  return ovl (retval);
}

DEFMETHOD (__event_manager_show_preferences__, interp, , ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_show_preferences__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.show_preferences ());
}

DEFMETHOD (__event_manager_apply_preferences__, interp, , ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_apply_preferences__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.apply_preferences ());
}

DEFMETHOD (__event_manager_gui_preference__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_gui_preference__ ()
Undocumented internal function.
@end deftypefn */)
{
  std::string key;
  std::string value = "";

  if (args.length () >= 1)
    key = args(0).string_value();
  else
    error ("__event_manager_gui_preference__: "
           "first argument must be the preference key");

  if (args.length () >= 2)
    value = args(1).string_value();

  if (octave::application::is_gui_running ())
    {
      octave::event_manager& evmgr = interp.get_event_manager ();

      return ovl (evmgr.gui_preference (key, value));
    }
  else
    return ovl (value);
}

DEFMETHOD (__event_manager_file_remove__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_file_remove__ ()
Undocumented internal function.
@end deftypefn */)
{
  std::string old_name, new_name;

  if (args.length () == 2)
    {
      old_name = args(0).string_value();
      new_name = args(1).string_value();
    }
  else
    error ("__event_manager_file_remove__: "
           "old and new name expected as arguments");

  octave::event_manager& evmgr = interp.get_event_manager ();

  evmgr.file_remove (old_name, new_name);

  return ovl ();
}

DEFMETHOD (__event_manager_file_renamed__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_file_renamed__ ()
Undocumented internal function.
@end deftypefn */)
{
  bool load_new;

  if (args.length () == 1)
    load_new = args(0).bool_value();
  else
    error ("__event_manager_file_renamed__: "
           "first argument must be boolean for reload new named file");

  octave::event_manager& evmgr = interp.get_event_manager ();

  evmgr.file_renamed (load_new);

  return ovl ();
}

DEFMETHOD (openvar, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} openvar (@var{name})
Open the variable @var{name} in the graphical Variable Editor.
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  if (! args(0).is_string ())
    error ("openvar: NAME must be a string");

  std::string name = args(0).string_value ();

  if (! (Fisguirunning ())(0).is_true ())
    warning ("openvar: GUI is not running, can't start Variable Editor");
  else
    {
      octave_value val = interp.varval (name);

      if (val.is_undefined ())
        error ("openvar: '%s' is not a variable", name.c_str ());

      octave::event_manager& evmgr = interp.get_event_manager ();

      evmgr.edit_variable (name, val);
    }

  return ovl ();
}

/*
%!error openvar ()
%!error openvar ("a", "b")
%!error <NAME must be a string> openvar (1:10)
*/

DEFMETHOD (__event_manager_show_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_show_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.show_doc (file));
}

DEFMETHOD (__event_manager_register_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_register_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.register_doc (file));
}

DEFMETHOD (__event_manager_unregister_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_unregister_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.unregister_doc (file));
}

DEFMETHOD (__event_manager_update_gui_lexer__, interp, , ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_update_gui_lexer__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave::event_manager& evmgr = interp.get_event_manager ();

  return ovl (evmgr.update_gui_lexer ());
}

DEFMETHOD (__event_manager_copy_image_to_clipboard__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __event_manager_copy_image_to_clipboard__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave::event_manager& evmgr = interp.get_event_manager ();
  evmgr.copy_image_to_clipboard (file);
  return ovl ();
}

DEFMETHOD (commandhistory, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} commandhistory ()
Show the GUI command history window and give it the keyboard focus.
@seealso{commandwindow, filebrowser, workspace}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  octave::event_manager& evmgr = interp.get_event_manager ();
  evmgr.focus_window ("history");
  return ovl ();
}

DEFMETHOD (commandwindow, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} commandwindow ()
Show the GUI command window and give it the keyboard focus.
@seealso{commandhistory, filebrowser, workspace}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  octave::event_manager& evmgr = interp.get_event_manager ();
  evmgr.focus_window ("command");
  return ovl ();
}

DEFMETHOD (filebrowser, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} filebrowser ()
Show the GUI file browser window and give it the keyboard focus.
@seealso{commandwindow, commandhistory, workspace}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  octave::event_manager& evmgr = interp.get_event_manager ();
  evmgr.focus_window ("filebrowser");
  return ovl ();
}

DEFMETHOD (workspace, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} workspace ()
Show the GUI workspace window and give it the keyboard focus.
@seealso{commandwindow, commandhistory, filebrowser}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  octave::event_manager& evmgr = interp.get_event_manager ();
  evmgr.focus_window ("workspace");
  return ovl ();
}
