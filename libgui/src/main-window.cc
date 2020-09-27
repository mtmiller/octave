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

#include <utility>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleFactory>
#include <QTextBrowser>
#include <QTextCodec>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QToolBar>

#if defined (HAVE_QSCINTILLA)
#  include "file-editor.h"
#endif
#include "gui-preferences-cs.h"
#include "gui-preferences-dw.h"
#include "gui-preferences-ed.h"
#include "gui-preferences-global.h"
#include "gui-preferences-mw.h"
#include "gui-preferences-nr.h"
#include "gui-preferences-sc.h"
#include "gui-settings.h"
#include "interpreter-qobject.h"
#include "main-window.h"
#include "news-reader.h"
#include "octave-qobject.h"
#include "settings-dialog.h"
#include "shortcut-manager.h"
#include "welcome-wizard.h"

#include "Array.h"
#include "cmd-edit.h"
#include "oct-env.h"
#include "url-transfer.h"

#include "builtin-defun-decls.h"
#include "defaults.h"
#include "defun.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "load-path.h"
#include "oct-map.h"
#include "octave.h"
#include "parse.h"
#include "syminfo.h"
#include "symscope.h"
#include "utils.h"
#include "version.h"

namespace octave
{
  static file_editor_interface *
  create_default_editor (QWidget *p, base_qobject& oct_qobj)
  {
#if defined (HAVE_QSCINTILLA)
    return new file_editor (p, oct_qobj);
#else
    octave_unused_parameter (p);
    octave_unused_parameter (oct_qobj);

    return 0;
#endif
  }

  main_window::main_window (base_qobject& oct_qobj)
    : QMainWindow (), m_octave_qobj (oct_qobj),
      m_workspace_model (nullptr),
      m_status_bar (nullptr), m_command_window (nullptr),
      m_history_window (nullptr), m_file_browser_window (nullptr),
      m_doc_browser_window (nullptr), m_editor_window (nullptr),
      m_workspace_window (nullptr), m_variable_editor_window (nullptr),
      m_external_editor (new external_editor_interface (this, m_octave_qobj)),
      m_active_editor (m_external_editor), m_settings_dlg (nullptr),
      m_find_files_dlg (nullptr), m_set_path_dlg (nullptr),
      m_release_notes_window (nullptr), m_community_news_window (nullptr),
      m_clipboard (QApplication::clipboard ()),
      m_prevent_readline_conflicts (true), m_suppress_dbg_location (true),
      m_closing (false), m_file_encoding (QString ())
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();

    if (rmgr.is_first_run ())
      {
        // Before wizard.
        m_octave_qobj.config_translators ();

        welcome_wizard welcomeWizard (m_octave_qobj);

        if (welcomeWizard.exec () == QDialog::Rejected)
          exit (1);

        // Install settings file.
        rmgr.reload_settings ();
      }
    else
      {
        // Get settings file.
        rmgr.reload_settings ();

        // After settings.
        m_octave_qobj.config_translators ();
      }

    rmgr.update_network_settings ();

    // We provide specific terminal capabilities, so ensure that
    // TERM is always set appropriately.

#if defined (OCTAVE_USE_WINDOWS_API)
    sys::env::putenv ("TERM", "cygwin");
#else
    sys::env::putenv ("TERM", "xterm");
#endif

    // FIXME: can we do this job when creating the shortcut manager?
    // A quick look shows that it may require some coordination with the
    // resource manager.  Startup is complicated, but maybe we can make
    // it simpler?
    shortcut_manager& scmgr = m_octave_qobj.get_shortcut_manager ();
    scmgr.init_data ();

    m_workspace_model = m_octave_qobj.get_workspace_model ();

    construct_central_widget ();

    m_status_bar = new QStatusBar ();
    m_command_window = new terminal_dock_widget (this, m_octave_qobj);
    m_history_window = new history_dock_widget (this, m_octave_qobj);
    m_file_browser_window = new files_dock_widget (this, m_octave_qobj);
    m_doc_browser_window = new documentation_dock_widget (this, m_octave_qobj);
    m_editor_window = create_default_editor (this, m_octave_qobj);
    m_variable_editor_window = new variable_editor (this, m_octave_qobj);
    m_workspace_window = new workspace_view (this, m_octave_qobj);

    m_previous_dock = m_command_window;

    // Set active editor depending on editor window.  If the latter is
    // not initialized (qscintilla not present), use the external editor.
    if (m_editor_window)
      m_active_editor = m_editor_window;
    else
      m_active_editor = m_external_editor;

#if defined (HAVE_QGUIAPPLICATION_SETDESKTOPFILENAME)
    QGuiApplication::setDesktopFileName ("org.octave.Octave.desktop");
#endif

    QApplication *qapp = m_octave_qobj.qapplication ();

    m_default_style = qapp->style ()->objectName ();

    gui_settings *settings = rmgr.get_settings ();

    bool connect_to_web = true;
    QDateTime last_checked;
    int serial = 0;
    m_active_dock = nullptr;

    if (settings)
      {
        connect_to_web
          = settings->value (nr_allow_connection).toBool ();

        last_checked
          = settings->value (nr_last_time).toDateTime ();

        serial = settings->value (nr_last_news).toInt ();
        m_default_encoding = settings->value (ed_default_enc).toString ();
      }

    QDateTime current = QDateTime::currentDateTime ();
    QDateTime one_day_ago = current.addDays (-1);

    if (connect_to_web
        && (! last_checked.isValid () || one_day_ago > last_checked))
      load_and_display_community_news (serial);

    construct_octave_qt_link ();

    // We have to set up all our windows, before we finally launch
    // octave.

    construct ();

    read_settings ();

    init_terminal_size ();

    // Connect signals for changes in visibility now before window is
    // shown.

    connect_visibility_changed ();

    focus_command_window ();
  }

  main_window::~main_window (void)
  {
    // Destroy the terminal first so that STDERR stream is redirected back
    // to its original pipe to capture error messages at exit.

    delete m_editor_window;     // first one for dialogs of modified editor-tabs
    delete m_external_editor;
    delete m_command_window;
    delete m_workspace_window;
    delete m_doc_browser_window;
    delete m_file_browser_window;
    delete m_history_window;
    delete m_status_bar;
    delete m_workspace_model;
    delete m_variable_editor_window;

    delete m_find_files_dlg;
    delete m_release_notes_window;
    delete m_community_news_window;
  }

  bool main_window::command_window_has_focus (void) const
  {
    return m_command_window->has_focus ();
  }

  void main_window::focus_command_window (void)
  {
    m_command_window->activate ();
  }

  void main_window::focus_window (const QString& win_name)
  {
    if (win_name == "command")
      m_command_window->activate ();
    else if (win_name == "history")
      m_history_window->activate ();
    else if (win_name == "workspace")
      m_workspace_window->activate ();
    else if (win_name == "filebrowser")
      m_file_browser_window->activate ();
  }

  bool main_window::confirm_shutdown (void)
  {
    bool closenow = true;

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    if (settings->value (global_prompt_to_exit.key,
                         global_prompt_to_exit.def).toBool ())
      {
        int ans = QMessageBox::question (this, tr ("Octave"),
                                         tr ("Are you sure you want to exit Octave?"),
                                         (QMessageBox::Ok
                                          | QMessageBox::Cancel),
                                         QMessageBox::Ok);

        if (ans != QMessageBox::Ok)
          closenow = false;
      }

#if defined (HAVE_QSCINTILLA)
    if (closenow)
      closenow = m_editor_window->check_closing ();
#endif

    return closenow;
  }

  // catch focus changes and determine the active dock widget
  void main_window::focus_changed (QWidget *, QWidget *new_widget)
  {
    // If there is no new widget (e.g., when pressing <alt> and the global
    // menu gets active), we can return immediately
    if (! new_widget)
      return;

    octave_dock_widget *dock = nullptr;
    QWidget *w_new = new_widget;  // get a copy of new focus widget
    QWidget *start = w_new;       // Save it as start of our search
    int count = 0;                // fallback to prevent endless loop

    QList<octave_dock_widget *> w_list = dock_widget_list ();

    while (w_new && w_new != m_main_tool_bar && count < 100)
      {
        // Go through all dock widgets and check whether the current widget
        // with focus is a child of one of them.
        for (auto w : w_list)
          {
            if (w->isAncestorOf (w_new))
              dock = w;
          }

        if (dock)
          break;

        // If not yet found (in case w_new is not a child of its dock widget),
        // test next widget in the focus chain
        w_new = qobject_cast<QWidget *> (w_new->previousInFocusChain ());

        // Measures preventing an endless loop
        if (w_new == start)
          break;  // We have arrived where we began ==> exit loop
        count++;  // Limited number of trials
      }

    // editor needs extra handling
    octave_dock_widget *edit_dock_widget
      = static_cast<octave_dock_widget *> (m_editor_window);
    // if new dock has focus, emit signal and store active focus
    // except editor changes to a dialog (dock=0)
    if ((dock || m_active_dock != edit_dock_widget) && (dock != m_active_dock))
      {
        // signal to all dock widgets for updating the style
        emit active_dock_changed (m_active_dock, dock);

        if (dock)
          {
            QList<QDockWidget *> tabbed = tabifiedDockWidgets (dock);
            if (tabbed.contains (m_active_dock))
              dock->set_predecessor_widget (m_active_dock);
          }

        if (edit_dock_widget == dock)
          emit editor_focus_changed (true);
        else if (edit_dock_widget == m_active_dock)
          emit editor_focus_changed (false);

        if (m_active_dock)
          m_previous_dock = m_active_dock;
        m_active_dock = dock;
      }
  }

  void main_window::request_reload_settings (void)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    if (settings)
      emit settings_changed (settings);
  }

  void main_window::report_status_message (const QString& statusMessage)
  {
    m_status_bar->showMessage (statusMessage, 1000);
  }

  void main_window::handle_save_workspace_request (void)
  {
    // FIXME: Remove, if for all common KDE versions (bug #54607) is resolved.
    int opts = 0;  // No options by default.
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (! settings->value (global_use_native_dialogs).toBool ())
      opts = QFileDialog::DontUseNativeDialog;

    QString file
      = QFileDialog::getSaveFileName (this, tr ("Save Workspace As"), ".",
                                      nullptr, nullptr, QFileDialog::Option (opts));

    if (! file.isEmpty ())
      {
        emit interpreter_event
          ([file] (interpreter& interp)
           {
             // INTERPRETER THREAD

             Fsave (interp, ovl (file.toStdString ()));
           });
      }
  }

  void main_window::handle_load_workspace_request (const QString& file_arg)
  {
    // FIXME: Remove, if for all common KDE versions (bug #54607) is resolved.
    int opts = 0;  // No options by default.
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (! settings->value (global_use_native_dialogs).toBool ())
      opts = QFileDialog::DontUseNativeDialog;

    QString file = file_arg;

    if (file.isEmpty ())
      file = QFileDialog::getOpenFileName (this, tr ("Load Workspace"), ".",
                                           nullptr, nullptr, QFileDialog::Option (opts));

    if (! file.isEmpty ())
      {
        emit interpreter_event
          ([file] (interpreter& interp)
           {
             // INTERPRETER THREAD

             Fload (interp, ovl (file.toStdString ()));

             tree_evaluator& tw = interp.get_evaluator ();

             event_manager& xevmgr = interp.get_event_manager ();

             xevmgr.set_workspace (true, tw.get_symbol_info ());
           });
      }
  }

  void main_window::handle_open_any_request (const QString& file_arg)
  {
    if (! file_arg.isEmpty ())
      {
        std::string file = file_arg.toStdString ();

        emit interpreter_event
          ([file] (interpreter& interp)
           {
             // INTERPRETER THREAD

             interp.feval ("open", ovl (file));

             // Update the workspace since open.m may have loaded new
             // variables.
             tree_evaluator& tw = interp.get_evaluator ();

             event_manager& xevmgr = interp.get_event_manager ();

             xevmgr.set_workspace (true, tw.get_symbol_info ());
           });
      }
  }

  void main_window::handle_clear_workspace_request (void)
  {
    emit interpreter_event
      ([] (interpreter& interp)
       {
         // INTERPRETER THREAD

         Fclear (interp);
       });
  }

  void main_window::handle_clear_command_window_request (void)
  {
    emit interpreter_event
      ([] (void)
       {
         // INTERPRETER THREAD

         command_editor::kill_full_line ();
         command_editor::clear_screen ();
       });
  }

  void main_window::handle_clear_history_request (void)
  {
    emit interpreter_event
      ([] (interpreter& interp)
       {
         // INTERPRETER THREAD

         history_system& history_sys = interp.get_history_system ();

         history_sys.do_history (ovl ("-c"));
       });
  }

  void main_window::handle_undo_request (void)
  {
    if (command_window_has_focus ())
      {
        emit interpreter_event
          ([] (void)
           {
             // INTERPRETER THREAD

             command_editor::undo ();
             command_editor::redisplay ();
           });
      }
    else
      emit undo_signal ();
  }

  void main_window::handle_rename_variable_request (const QString& old_name_arg,
                                                    const QString& new_name_arg)

  {
    std::string old_name = old_name_arg.toStdString ();
    std::string new_name = new_name_arg.toStdString ();

    emit interpreter_event
      ([old_name, new_name] (interpreter& interp)
       {
         // INTERPRETER THREAD

         symbol_scope scope = interp.get_current_scope ();

         if (scope)
           {
             scope.rename (old_name, new_name);

             tree_evaluator& tw = interp.get_evaluator ();

             event_manager& xevmgr = interp.get_event_manager ();

             xevmgr.set_workspace (true, tw.get_symbol_info ());
           }

         // FIXME: if this action fails, do we need a way to display that info
         // in the GUI?
       });
  }

  void main_window::modify_path (const octave_value_list& dir_list,
                                 bool rm, bool subdirs)
  {
    emit interpreter_event
      ([dir_list, rm, subdirs, this] (interpreter& interp)
      {
        // INTERPRETER THREAD

        octave_value_list paths = ovl ();

        if (subdirs)
          {
            // Loop over all directories in order to get all subdirs
            for (octave_idx_type i = 0; i < dir_list.length (); i++)
              paths.append (Fgenpath (dir_list(i)));
          }
        else
          paths = dir_list;

        if (rm)
          Frmpath (interp, paths);
        else
          Faddpath (interp, paths);
      });
  }

  void main_window::new_file (const QString& commands)
  {
    emit new_file_signal (commands);
  }

  void main_window::open_file (const QString& file_name, int line)
  {
    if (line < 0)
      emit open_file_signal (file_name);
    else
      emit open_file_signal (file_name, QString (), line);
  }

  void main_window::edit_mfile (const QString& name, int line)
  {
    handle_edit_mfile_request (name, QString (), QString (), line);
  }

  void main_window::file_remove_proxy (const QString& o, const QString& n)
  {
    interpreter_qobject *interp_qobj = m_octave_qobj.interpreter_qobj ();

    qt_interpreter_events *qt_link = interp_qobj->qt_link ();

    // Wait for worker to suspend
    qt_link->lock ();

    // Close the file if opened
#if defined (HAVE_QSCINTILLA)
    m_editor_window->handle_file_remove (o, n);
#else
    octave_unused_parameter (o);
    octave_unused_parameter (n);
#endif

    // We are done: Unlock and wake the worker thread
    qt_link->unlock ();
    qt_link->wake_all ();
  }

  void main_window::open_online_documentation_page (void)
  {
    QDesktopServices::openUrl
      (QUrl ("https://octave.org/doc/interpreter/index.html"));
  }

  void main_window::display_release_notes (void)
  {
    if (! m_release_notes_window)
      {
        std::string news_file = config::oct_etc_dir () + "/NEWS";

        QString news;

        QFile *file = new QFile (QString::fromStdString (news_file));
        if (file->open (QFile::ReadOnly))
          {
            QTextStream *stream = new QTextStream (file);
            news = stream->readAll ();
            if (! news.isEmpty ())
              {
                // Convert '<', '>' which would be interpreted as HTML
                news.replace ("<", "&lt;");
                news.replace (">", "&gt;");
                // Add HTML tags for pre-formatted text
                news.prepend ("<pre>");
                news.append ("</pre>");
              }
            else
              news = (tr ("The release notes file '%1' is empty.")
                      . arg (QString::fromStdString (news_file)));
          }
        else
          news = (tr ("The release notes file '%1' cannot be read.")
                  . arg (QString::fromStdString (news_file)));

        m_release_notes_window = new QWidget;

        QTextBrowser *browser = new QTextBrowser (m_release_notes_window);
        browser->setText (news);

        QVBoxLayout *vlayout = new QVBoxLayout;
        vlayout->addWidget (browser);

        m_release_notes_window->setLayout (vlayout);
        m_release_notes_window->setWindowTitle (tr ("Octave Release Notes"));

        browser->document ()->adjustSize ();

        // center the window on the screen where octave is running
        QDesktopWidget *m_desktop = QApplication::desktop ();
        QRect screen_geo = m_desktop->availableGeometry (this);

        int win_x = screen_geo.width ();        // width of the screen
        int win_y = screen_geo.height ();       // height of the screen

        int reln_x = win_x*2/5;  // desired width of release notes
        int reln_y = win_y*2/3;  // desired height of release notes

        m_release_notes_window->resize (reln_x, reln_y);  // set size
        m_release_notes_window->move (20, 20);     // move to the top left corner
      }

    if (! m_release_notes_window->isVisible ())
      m_release_notes_window->show ();
    else if (m_release_notes_window->isMinimized ())
      m_release_notes_window->showNormal ();

    m_release_notes_window->setWindowIcon (QIcon (m_release_notes_icon));

    m_release_notes_window->raise ();
    m_release_notes_window->activateWindow ();
  }

  void main_window::load_and_display_community_news (int serial)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    bool connect_to_web
      = (settings
         ? settings->value (nr_allow_connection).toBool ()
         : true);

    QString base_url = "https://octave.org";
    QString page = "community-news.html";

    QThread *worker_thread = new QThread;

    news_reader *reader = new news_reader (m_octave_qobj, base_url, page,
                                           serial, connect_to_web);

    reader->moveToThread (worker_thread);

    connect (reader, SIGNAL (display_news_signal (const QString&)),
             this, SLOT (display_community_news (const QString&)));

    connect (worker_thread, SIGNAL (started (void)),
             reader, SLOT (process (void)));

    connect (reader, SIGNAL (finished (void)), worker_thread, SLOT (quit (void)));

    connect (reader, SIGNAL (finished (void)), reader, SLOT (deleteLater (void)));

    connect (worker_thread, SIGNAL (finished (void)),
             worker_thread, SLOT (deleteLater (void)));

    worker_thread->start ();
  }

  void main_window::display_community_news (const QString& news)
  {
    if (! m_community_news_window)
      {
        m_community_news_window = new QWidget;

        QTextBrowser *browser = new QTextBrowser (m_community_news_window);

        browser->setHtml (news);
        browser->setObjectName ("OctaveNews");
        browser->setOpenExternalLinks (true);

        QVBoxLayout *vlayout = new QVBoxLayout;

        vlayout->addWidget (browser);

        m_community_news_window->setLayout (vlayout);
        m_community_news_window->setWindowTitle (tr ("Octave Community News"));

        // center the window on the screen where octave is running
        QDesktopWidget *m_desktop = QApplication::desktop ();
        QRect screen_geo = m_desktop->availableGeometry (this);

        int win_x = screen_geo.width ();        // width of the screen
        int win_y = screen_geo.height ();       // height of the screen

        int news_x = win_x/2;  // desired width of news window
        int news_y = win_y/2;  // desired height of news window

        m_community_news_window->resize (news_x, news_y);  // set size and center
        m_community_news_window->move ((win_x - m_community_news_window->width ())/2,
                                       (win_y - m_community_news_window->height ())/2);
      }
    else
      {
        // Window already exists, just update the browser contents
        QTextBrowser *browser

          = m_community_news_window->findChild<QTextBrowser *>("OctaveNews"
                                                               , Qt::FindDirectChildrenOnly
                                                              );
        if (browser)
          browser->setHtml (news);
      }

    if (! m_community_news_window->isVisible ())
      m_community_news_window->show ();
    else if (m_community_news_window->isMinimized ())
      m_community_news_window->showNormal ();

    // same icon as release notes
    m_community_news_window->setWindowIcon (QIcon (m_release_notes_icon));

    m_community_news_window->raise ();
    m_community_news_window->activateWindow ();
  }

  void main_window::open_bug_tracker_page (void)
  {
    QDesktopServices::openUrl (QUrl ("https://octave.org/bugs.html"));
  }

  void main_window::open_octave_packages_page (void)
  {
    QDesktopServices::openUrl (QUrl ("https://octave.org/packages.html"));
  }

  void main_window::open_contribute_page (void)
  {
    QDesktopServices::openUrl (QUrl ("https://octave.org/contribute.html"));
  }

  void main_window::open_donate_page (void)
  {
    QDesktopServices::openUrl (QUrl ("https://octave.org/donate.html"));
  }

  void main_window::process_settings_dialog_request (const QString& desired_tab)
  {
    if (m_settings_dlg)  // m_settings_dlg is a guarded pointer!
      {
        // here the dialog is still open and called once again
        if (! desired_tab.isEmpty ())
          m_settings_dlg->show_tab (desired_tab);
        return;
      }

    m_settings_dlg = new settings_dialog (this, m_octave_qobj, desired_tab);

    connect (m_settings_dlg, SIGNAL (apply_new_settings (void)),
             this, SLOT (request_reload_settings (void)));

    m_settings_dlg->setModal (false);
    m_settings_dlg->setAttribute (Qt::WA_DeleteOnClose);
    m_settings_dlg->show ();
  }

  void main_window::show_about_octave (void)
  {
    std::string message
      = octave_name_version_copyright_copying_warranty_and_bugs (true);

    QMessageBox::about (this, tr ("About Octave"),
                        QString::fromStdString (message));
  }

  void main_window::notice_settings (const gui_settings *settings)
  {
    if (! settings)
      return;

    // Get desired style from preferences or take the default one if
    // the desired one is not found
    QString preferred_style = settings->value (global_style).toString ();

    if (preferred_style == global_style.def.toString ())
      preferred_style = m_default_style;

    QStyle *new_style = QStyleFactory::create (preferred_style);
    if (new_style)
      {
        QApplication *qapp = m_octave_qobj.qapplication ();

        qapp->setStyle (new_style);
      }

    // the widget's icons (when floating)
    QString icon_set
      = settings->value (dw_icon_set).toString ();

    int count = 0;
    int icon_set_found = 0; // default

    while (! dw_icon_set_names[count].name.isEmpty ())
      {
        // while not end of data
        if (dw_icon_set_names[count].name == icon_set)
          {
            // data of desired icon set found
            icon_set_found = count;
            break;
          }
        count++;
      }

    QString icon;
    for (auto *widget : dock_widget_list ())
      {
        QString name = widget->objectName ();
        if (! name.isEmpty ())
          {
            // if child has a name
            icon = dw_icon_set_names[icon_set_found].path; // prefix | octave-logo
            if (dw_icon_set_names[icon_set_found].name != "NONE")
              icon += name + ".png"; // add widget name and ext.
            widget->setWindowIcon (QIcon (icon));
          }
      }
    if (dw_icon_set_names[icon_set_found].name != "NONE")
      m_release_notes_icon = dw_icon_set_names[icon_set_found].path
                             + "ReleaseWidget.png";
    else
      m_release_notes_icon = ":/actions/icons/logo.png";

    int size_idx = settings->value (global_icon_size).toInt ();
    size_idx = (size_idx > 0) - (size_idx < 0) + 1;  // Make valid index from 0 to 2

    QStyle *st = style ();
    int icon_size = st->pixelMetric (global_icon_sizes[size_idx]);
    m_main_tool_bar->setIconSize (QSize (icon_size,icon_size));

    if (settings->value (global_status_bar.key, global_status_bar.def).toBool ())
      m_status_bar->show ();
    else
      m_status_bar->hide ();

    m_prevent_readline_conflicts
      = settings->value (sc_prevent_rl_conflicts.key,
                         sc_prevent_rl_conflicts.def).toBool ();

    m_suppress_dbg_location
      = ! settings->value (cs_dbg_location).toBool ();

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    rmgr.update_network_settings ();

    emit active_dock_changed (nullptr, m_active_dock); // update dock widget styles

    configure_shortcuts ();
    set_global_shortcuts (m_active_dock == m_command_window);
    disable_menu_shortcuts (m_active_dock == m_editor_window);

    // Check whether some octave internal preferences have to be updated
    QString new_default_encoding
      = settings->value (ed_default_enc).toString ();
    if (new_default_encoding != m_default_encoding)
      update_default_encoding (new_default_encoding);

    // Set cursor blinking depending on the settings
    // Cursor blinking: consider old terminal related setting if not yet set
    // TODO: This pref. can be deprecated / removed if Qt adds support for
    //       getting the cursor blink preferences from all OS environments
    bool cursor_blinking;

    if (settings->contains (global_cursor_blinking.key))
      cursor_blinking = settings->value (global_cursor_blinking).toBool ();
    else
      cursor_blinking = settings->value (cs_cursor_blinking).toBool ();

    if (cursor_blinking)
      QApplication::setCursorFlashTime (1000);  // 1000 ms flash time
    else
      QApplication::setCursorFlashTime (0);  // no flashing

  }

  void main_window::prepare_to_exit (void)
  {
    // Find files dialog is constructed dynamically, not at time of main_window
    // construction.  Connecting it to qApp aboutToQuit signal would have
    // caused it to run after gui_settings is deleted.
    if (m_find_files_dlg)
      m_find_files_dlg->save_settings ();

    if (m_set_path_dlg)
      m_set_path_dlg->save_settings ();

    write_settings ();
  }

  void main_window::go_to_previous_widget (void)
  {
    m_previous_dock->activate ();
  }

  void main_window::reset_windows (void)
  {
    hide ();
    set_window_layout (nullptr);  // do not use the settings file
    showNormal ();  // make sure main window is not minimized
    focus_command_window ();
  }

  void main_window::update_octave_directory (const QString& dir)
  {
    // Remove existing entry, if any, then add new directory at top and
    // mark it as the current directory.  Finally, update the file list
    // widget.

    int index = m_current_directory_combo_box->findText (dir);

    if (index >= 0)
      m_current_directory_combo_box->removeItem (index);

    m_current_directory_combo_box->insertItem (0, dir);
    m_current_directory_combo_box->setCurrentIndex (0);
  }

  void main_window::browse_for_directory (void)
  {
    // FIXME: Remove, if for all common KDE versions (bug #54607) is resolved.
    int opts = QFileDialog::ShowDirsOnly;
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (! settings->value (global_use_native_dialogs).toBool ())
      opts = QFileDialog::DontUseNativeDialog;

    QString dir
      = QFileDialog::getExistingDirectory (this, tr ("Browse directories"), nullptr,
                                           QFileDialog::Option (opts));

    set_current_working_directory (dir);

    // FIXME: on Windows systems, the command window freezes after the
    // previous actions.  Forcing the focus appears to unstick it.

    focus_command_window ();
  }

  void main_window::set_current_working_directory (const QString& dir)
  {
    // Change to dir if it is an existing directory.

    QString xdir = (dir.isEmpty () ? "." : dir);

    QFileInfo fileInfo (xdir);

    if (fileInfo.exists () && fileInfo.isDir ())
      {
        emit interpreter_event
          ([xdir] (interpreter& interp)
           {
             // INTERPRETER THREAD

             interp.chdir (xdir.toStdString ());
           });
      }
  }

  void main_window::change_directory_up (void)
  {
    set_current_working_directory ("..");
  }

  // Slot that is called if return is pressed in the line edit of the
  // combobox to change to a new directory or a directory that is already
  // in the drop down list.

  void main_window::accept_directory_line_edit (void)
  {
    // Get new directory name, and change to it if it is new.  Otherwise,
    // the combo box will trigger the "activated" signal to change to the
    // directory.

    QString dir = m_current_directory_combo_box->currentText ();

    int index = m_current_directory_combo_box->findText (dir);

    if (index < 0)
      set_current_working_directory (dir);
  }

  void main_window::execute_command_in_terminal (const QString& command)
  {
    emit interpreter_event
      ([command] (void)
       {
         // INTERPRETER THREAD

         std::string pending_input = command_editor::get_current_line ();

         command_editor::set_initial_input (pending_input);
         command_editor::replace_line (command.toStdString ());
         command_editor::redisplay ();
         command_editor::interrupt_event_loop ();
         command_editor::accept_line ();
       });

    focus_console_after_command ();
  }

  void main_window::run_file_in_terminal (const QFileInfo& info)
  {
    emit interpreter_event
      ([info] (interpreter& interp)
       {
         // INTERPRETER THREAD

         QString function_name = info.fileName ();
         function_name.chop (info.suffix ().length () + 1);
         std::string file_path = info.absoluteFilePath ().toStdString ();

         std::string pending_input = command_editor::get_current_line ();

         if (valid_identifier (function_name.toStdString ()))
           {
             // Valid identifier: call as function with possibility to
             // debug.

             load_path& lp = interp.get_load_path ();

             std::string path = info.absolutePath ().toStdString ();

             if (lp.contains_file_in_dir (file_path, path))
               command_editor::replace_line (function_name.toStdString ());
           }
         else
           {
             // No valid identifier: use equivalent of Fsource (), no
             // debug possible.

             interp.source_file (file_path);

             command_editor::replace_line ("");
           }

         command_editor::set_initial_input (pending_input);
         command_editor::redisplay ();
         command_editor::interrupt_event_loop ();
         command_editor::accept_line ();
       });

    focus_console_after_command ();
  }

  void main_window::handle_new_figure_request (void)
  {
    emit interpreter_event
      ([] (interpreter& interp)
       {
         // INTERPRETER THREAD

         Fbuiltin (interp, ovl ("figure"));
         Fdrawnow (interp);
       });
  }

  void main_window::handle_enter_debugger (void)
  {
    setWindowTitle ("Octave (Debugging)");

    m_debug_continue->setEnabled (true);
    m_debug_step_into->setEnabled (true);
    m_debug_step_over->setEnabled (true);
    m_debug_step_out->setEnabled (true);
    m_debug_quit->setEnabled (true);
  }

  void main_window::handle_exit_debugger (void)
  {
    setWindowTitle ("Octave");

    m_debug_continue->setEnabled (false);
    m_debug_step_into->setEnabled (false);
    m_debug_step_over->setEnabled (m_editor_has_tabs);
    m_debug_step_out->setEnabled (false);
    m_debug_quit->setEnabled (false);
  }

  void main_window::debug_continue (void)
  {
    emit interpreter_event
      ([this] (interpreter& interp)
       {
         // INTERPRETER THREAD

         F__db_next_breakpoint_quiet__ (interp, ovl (m_suppress_dbg_location));
         Fdbcont (interp);

         command_editor::interrupt (true);
       });
  }

  void main_window::debug_step_into (void)
  {
    emit interpreter_event
      ([this] (interpreter& interp)
       {
         // INTERPRETER THREAD

         F__db_next_breakpoint_quiet__ (interp, ovl (m_suppress_dbg_location));
         Fdbstep (interp, ovl ("in"));

         command_editor::interrupt (true);
       });
  }

  void main_window::debug_step_over (void)
  {
    if (m_debug_quit->isEnabled ())
      {
        // We are in debug mode, just call dbstep.

        emit interpreter_event
          ([this] (interpreter& interp)
           {
             // INTERPRETER THREAD

             F__db_next_breakpoint_quiet__ (interp,
                                            ovl (m_suppress_dbg_location));
             Fdbstep (interp);

             command_editor::interrupt (true);
           });
      }
    else
      {
        // Not in debug mode: "step into" the current editor file
        emit step_into_file_signal ();
      }
  }

  void main_window::debug_step_out (void)
  {
    emit interpreter_event
      ([this] (interpreter& interp)
       {
         // INTERPRETER THREAD

         F__db_next_breakpoint_quiet__ (interp, ovl (m_suppress_dbg_location));
         Fdbstep (interp, ovl ("out"));

         command_editor::interrupt (true);
       });
  }

  void main_window::debug_quit (void)
  {
    emit interpreter_event
      ([] (interpreter& interp)
       {
         // INTERPRETER THREAD

         Fdbquit (interp);

         command_editor::interrupt (true);
       });
  }

  //
  // Functions related to file editing
  //
  // These are moved from editor to here for also using them when octave
  // is built without qscintilla
  //
  void main_window::request_open_file (void)
  {
    // Open file isn't a file_editor_tab or editor function since the file
    // might be opened in an external editor.  Hence, functionality is here.

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    bool is_internal = m_editor_window
                       && ! settings->value (global_use_custom_editor.key,
                                             global_use_custom_editor.def).toBool ();

    // Create a NonModal message.
    QWidget *p = this;
    if (is_internal)
      p = m_editor_window;
    QFileDialog *fileDialog = new QFileDialog (p);
    fileDialog->setNameFilter (tr ("Octave Files (*.m);;All Files (*)"));

    fileDialog->setAcceptMode (QFileDialog::AcceptOpen);
    fileDialog->setViewMode (QFileDialog::Detail);
    fileDialog->setFileMode (QFileDialog::ExistingFiles);
    fileDialog->setDirectory (m_current_directory_combo_box->itemText (0));

    // FIXME: Remove, if for all common KDE versions (bug #54607) is resolved.
    if (! settings->value (global_use_native_dialogs).toBool ())
      fileDialog->setOption(QFileDialog::DontUseNativeDialog);

    connect (fileDialog, SIGNAL (filesSelected (const QStringList&)),
             this, SLOT (request_open_files (const QStringList&)));

    fileDialog->setWindowModality (Qt::NonModal);
    fileDialog->setAttribute (Qt::WA_DeleteOnClose);
    fileDialog->show ();
  }

  // Create a new script
  void main_window::request_new_script (const QString& commands)
  {
    emit new_file_signal (commands);
  }

  // Create a new function and open it
  void main_window::request_new_function (bool)
  {
    bool ok;
    // Get the name of the new function: Parent of the input dialog is the
    // editor window or the main window.  The latter is chosen, if a custom
    // editor is used or qscintilla is not available
    QWidget *p = m_editor_window;
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (! p || settings->value (global_use_custom_editor.key,
                                global_use_custom_editor.def).toBool ())
      p = this;
    QString new_name = QInputDialog::getText (p, tr ("New Function"),
                                              tr ("New function name:\n"), QLineEdit::Normal, "", &ok);

    if (ok && new_name.length () > 0)
      {
        // append suffix if it does not already exist
        if (new_name.rightRef (2) != ".m")
          new_name.append (".m");
        // check whether new files are created without prompt
        if (! settings->value (ed_create_new_file).toBool ())
          {
            // no, so enable this settings and wait for end of new file loading
            settings->setValue (ed_create_new_file.key, true);
            connect (m_editor_window, SIGNAL (file_loaded_signal (void)),
                     this, SLOT (restore_create_file_setting (void)));
          }
        // start the edit command
        execute_command_in_terminal ("edit " + new_name);
      }
  }

  void main_window::handle_edit_mfile_request (const QString& fname,
                                               const QString& ffile,
                                               const QString& curr_dir,
                                               int line)
  {
    emit interpreter_event
      ([this, fname, ffile, curr_dir, line] (interpreter& interp)
       {
         // INTERPRETER THREAD

         // Split possible subfunctions
         QStringList fcn_list = fname.split ('>');
         QString fcn_name = fcn_list.at (0) + ".m";

         // FIXME: could use symbol_exist directly, but we may also want
         // to fix that to be a member function in the interpreter
         // class?

         // Is it a regular function within the search path? (Call Fexist)
         octave_value_list fct = Fexist (interp, ovl (fname.toStdString ()),0);
         int type = fct (0).int_value ();

         QString message = QString ();
         QString filename = QString ();

         switch (type)
           {
           case 3:
           case 5:
           case 103:
             message = tr ("%1 is a built-in, compiled or inline\n"
                           "function and can not be edited.");
             break;

           case 2:
             // FIXME: could use a load_path function directly.
             octave_value_list file_path
               = Ffile_in_loadpath (interp, ovl (fcn_name.toStdString ()), 0);
             if (file_path.length () > 0)
               filename = QString::fromStdString (file_path (0).string_value ());
             break;
           }

         if (filename.isEmpty () && message.isEmpty ())
           {
             // No error so far, but function still not known
             // -> try directory of edited file
             // get directory
             QDir dir;
             if (ffile.isEmpty ())
               {
                 if (curr_dir.isEmpty ())
                   dir = QDir (m_current_directory_combo_box->itemText (0));
                 else
                   dir = QDir (curr_dir);
               }
             else
               dir = QDir (QFileInfo (ffile).canonicalPath ());

             QFileInfo file = QFileInfo (dir, fcn_name);
             if (file.exists ())
               filename = file.canonicalFilePath (); // local file exists
             else
               {
                 // local file does not exist -> try private directory
                 file = QFileInfo (ffile);
                 file = QFileInfo (QDir (file.canonicalPath () + "/private"),
                                   fcn_name);
                 if (file.exists ())
                   filename = file.canonicalFilePath ();  // private function exists
                 else
                   message = tr ("Can not find function %1");  // no file found

               }
           }

         if (! message.isEmpty ())
           {
             emit warning_function_not_found_signal (message.arg (fname));
             return;
           }

         if (! filename.endsWith (".m"))
           filename.append (".m");

         // default encoding
         emit open_file_signal (filename, QString (), line);
       });
  }

  void main_window::warning_function_not_found (const QString& message)
  {
    QMessageBox *msgBox = new QMessageBox (QMessageBox::Critical,
                                           tr ("Octave Editor"),
                                           message, QMessageBox::Ok, this);
    msgBox->setWindowModality (Qt::NonModal);
    msgBox->setAttribute (Qt::WA_DeleteOnClose);
    msgBox->show ();
  }

  void main_window::handle_insert_debugger_pointer_request (const QString& file,
                                                            int line)
  {
    bool cmd_focus = command_window_has_focus ();

    emit insert_debugger_pointer_signal (file, line);

    if (cmd_focus)
      focus_command_window ();
  }

  void main_window::handle_delete_debugger_pointer_request (const QString& file,
                                                            int line)
  {
    bool cmd_focus = command_window_has_focus ();

    emit delete_debugger_pointer_signal (file, line);

    if (cmd_focus)
      focus_command_window ();
  }

  void main_window::handle_update_breakpoint_marker_request (bool insert,
                                                             const QString& file,
                                                             int line,
                                                             const QString& cond)
  {
    bool cmd_focus = command_window_has_focus ();

    emit update_breakpoint_marker_signal (insert, file, line, cond);

    if (cmd_focus)
      focus_command_window ();
  }

  void main_window::read_settings (void)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    if (! settings)
      {
        qDebug ("Error: gui_settings pointer from resource manager is NULL.");
        return;
      }

    set_window_layout (settings);

    // restore the list of the last directories
    QStringList curr_dirs = settings->value (mw_dir_list).toStringList ();
    for (int i=0; i < curr_dirs.size (); i++)
      {
        m_current_directory_combo_box->addItem (curr_dirs.at (i));
      }
    emit settings_changed (settings);
  }

  void main_window::init_terminal_size (void)
  {
    emit init_terminal_size_signal ();
  }

  void main_window::set_window_layout (gui_settings *settings)
  {
    // Restore main window state and geometry from settings file or, in case
    // of an error, from the default layout.
    if (settings)
      {
        if (! restoreState (settings->value (mw_state).toByteArray ()))
          restoreState (mw_state.def.toByteArray ());

        if (! restoreGeometry (settings->value (mw_geometry).toByteArray ()))
          restoreGeometry (mw_geometry.def.toByteArray ());
      }

    // Restore the geometry of all dock-widgets
    for (auto *widget : dock_widget_list ())
      {
        QString name = widget->objectName ();

        if (! name.isEmpty ())
          {
            bool floating = false;
            bool visible = true;
            if (settings)
              {
                floating = settings->value
                  (dw_is_floating.key.arg (name), dw_is_floating.def).toBool ();
                visible = settings->value
                  (dw_is_visible.key.arg (name), dw_is_visible.def).toBool ();
              }

            // If floating, make window from widget.
            if (floating)
              {
                widget->make_window ();

                if (visible)
                  {
                    if (settings
                        && settings->value (dw_is_minimized.key.arg (name),
                                            dw_is_minimized.def).toBool ())
                      widget->showMinimized ();
                    else
                      widget->setVisible (true);
                  }
              }
            else  // not floating
              {
                if (! widget->parent ())        // should not be floating but is
                  widget->make_widget (false);  // no docking, just reparent

                widget->make_widget ();
                widget->setVisible (visible);   // not floating -> show
              }
          }
      }

    if (! settings)
      {
        restoreGeometry (mw_geometry.def.toByteArray ());
        restoreState (mw_state.def.toByteArray ());

        QDesktopWidget *m_desktop = QApplication::desktop ();
        QRect screen_geo = m_desktop->availableGeometry (this);

        int win_x = screen_geo.width ();        // width of the screen
        int win_y = screen_geo.height ();       // height of the screen

        resize (std::max (width (), 2*win_x/3), std::max (height (), 7*win_y/8));
      }

    show ();
  }

  void main_window::write_settings (void)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (! settings)
      {
        qDebug ("Error: gui_settings pointer from resource manager is NULL.");
        return;
      }

    settings->setValue (mw_geometry.key, saveGeometry ());
    settings->setValue (mw_state.key, saveState ());
    // write the list of recently used directories
    QStringList curr_dirs;
    for (int i=0; i<m_current_directory_combo_box->count (); i++)
      {
        curr_dirs.append (m_current_directory_combo_box->itemText (i));
      }
    settings->setValue (mw_dir_list.key, curr_dirs);
    settings->sync ();
  }

  // Connecting the signals emitted when the visibility of a widget changes.
  // This has to be done after the window is shown (see octave-gui.cc)
  void main_window::connect_visibility_changed (void)
  {
    for (auto *widget : dock_widget_list ())
      widget->connect_visibility_changed ();

#if defined (HAVE_QSCINTILLA)
    m_editor_window->enable_menu_shortcuts (false);
#endif
  }

  void main_window::copyClipboard (void)
  {
    if (m_current_directory_combo_box->hasFocus ())
      {
        QLineEdit *edit = m_current_directory_combo_box->lineEdit ();
        if (edit && edit->hasSelectedText ())
          {
            QClipboard *clipboard = QApplication::clipboard ();
            clipboard->setText (edit->selectedText ());
          }
      }
    else
      emit copyClipboard_signal ();
  }

  void main_window::pasteClipboard (void)
  {
    if (m_current_directory_combo_box->hasFocus ())
      {
        QLineEdit *edit = m_current_directory_combo_box->lineEdit ();
        QClipboard *clipboard = QApplication::clipboard ();
        QString str = clipboard->text ();
        if (edit && str.length () > 0)
          {
            edit->insert (str);
          }
      }
    else
      emit pasteClipboard_signal ();
  }

  void main_window::selectAll (void)
  {
    if (m_current_directory_combo_box->hasFocus ())
      {
        QLineEdit *edit = m_current_directory_combo_box->lineEdit ();
        if (edit)
          {
            edit->selectAll ();
          }
      }
    else
      emit selectAll_signal ();
  }

  void main_window::handle_show_doc (const QString& file)
  {
    m_doc_browser_window->setVisible (true);
    emit show_doc_signal (file);
  }

  void main_window::handle_register_doc (const QString& file)
  {
    emit register_doc_signal (file);
  }

  void main_window::handle_unregister_doc (const QString& file)
  {
    emit unregister_doc_signal (file);
  }

  void main_window::handle_octave_ready (void)
  {
    // actions after the startup files are executed
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    QDir startup_dir = QDir ();    // current octave dir after startup

    if (settings)
      {
        if (settings->value (global_restore_ov_dir.key,
                             global_restore_ov_dir.def).toBool ())
          {
            // restore last dir from previous session
            QStringList curr_dirs
              = settings->value (mw_dir_list).toStringList ();
            startup_dir
              = QDir (curr_dirs.at (0));  // last dir in previous session
          }
        else if (! settings->value (global_ov_startup_dir.key,
                                    global_ov_startup_dir.def).toString ().isEmpty ())
          {
            // do not restore but there is a startup dir configured
            startup_dir
              = QDir (settings->value (global_ov_startup_dir.key,
                                       global_ov_startup_dir.def).toString ());
          }

        update_default_encoding (settings->value (ed_default_enc).toString ());
      }

    if (! startup_dir.exists ())
      {
        // the configured startup dir does not exist, take actual one
        startup_dir = QDir ();
      }

    set_current_working_directory (startup_dir.absolutePath ());

    if (m_editor_window)
      {
#if defined (HAVE_QSCINTILLA)
        // Octave ready, determine whether to create an empty script.
        // This can not be done when the editor is created because all functions
        // must be known for the lexer's auto completion information
        m_editor_window->empty_script (true, false);
        m_editor_window->restore_session (settings);
#endif
      }

    focus_command_window ();  // make sure that the command window has focus
  }

  void main_window::handle_set_path_dialog_request (void)
  {
    if (m_set_path_dlg)  // m_set_path_dlg is a guarded pointer!
      return;

    m_set_path_dlg = new set_path_dialog (this, m_octave_qobj);

    m_set_path_dlg->setModal (false);
    m_set_path_dlg->setAttribute (Qt::WA_DeleteOnClose);
    m_set_path_dlg->show ();

    // Any interpreter_event signal from a set_path_dialog object is
    // handled the same as for the main_window object.

    connect (m_set_path_dlg, SIGNAL (interpreter_event (const fcn_callback&)),
             this, SIGNAL (interpreter_event (const fcn_callback&)));

    connect (m_set_path_dlg, SIGNAL (interpreter_event (const meth_callback&)),
             this, SIGNAL (interpreter_event (const meth_callback&)));

    connect (m_set_path_dlg,
             SIGNAL (modify_path_signal (const octave_value_list&, bool, bool)),
             this, SLOT (modify_path (const octave_value_list&, bool, bool)));

    interpreter_qobject *interp_qobj = m_octave_qobj.interpreter_qobj ();

    qt_interpreter_events *qt_link = interp_qobj->qt_link ();

    connect (qt_link, SIGNAL (update_path_dialog_signal (void)),
             m_set_path_dlg, SLOT (update_model (void)));

    // Now that all the signal connections are in place for the dialog
    // we can set the initial value of the path in the model.

    m_set_path_dlg->update_model ();
  }

  void main_window::find_files (const QString& start_dir)
  {

    if (! m_find_files_dlg)
      {
        m_find_files_dlg = new find_files_dialog (this, m_octave_qobj);

        connect (m_find_files_dlg, SIGNAL (finished (int)),
                 this, SLOT (find_files_finished (int)));

        connect (m_find_files_dlg, SIGNAL (dir_selected (const QString &)),
                 m_file_browser_window,
                 SLOT (set_current_directory (const QString&)));

        connect (m_find_files_dlg, SIGNAL (file_selected (const QString &)),
                 this, SLOT (open_file (const QString &)));

        m_find_files_dlg->setWindowModality (Qt::NonModal);
      }

    if (! m_find_files_dlg->isVisible ())
      {
        m_find_files_dlg->show ();
      }

    m_find_files_dlg->set_search_dir (start_dir);

    m_find_files_dlg->activateWindow ();

  }

  void main_window::set_global_shortcuts (bool set_shortcuts)
  {
    // this slot is called when the terminal gets/loses focus

    // return if the user doesn't want to use readline shortcuts
    if (! m_prevent_readline_conflicts)
      return;

    if (set_shortcuts)
      {
        // terminal loses focus: set the global shortcuts
        configure_shortcuts ();
      }
    else
      {
        // terminal gets focus: disable some shortcuts
        QKeySequence no_key = QKeySequence ();

        // file menu
        m_open_action->setShortcut (no_key);
        m_new_script_action->setShortcut (no_key);
        m_new_function_action->setShortcut (no_key);
        m_new_figure_action->setShortcut (no_key);
        m_load_workspace_action->setShortcut (no_key);
        m_save_workspace_action->setShortcut (no_key);
        m_preferences_action->setShortcut (no_key);
        m_set_path_action->setShortcut (no_key);
        m_exit_action->setShortcut (no_key);

        // edit menu
        m_select_all_action->setShortcut (no_key);
        m_clear_clipboard_action->setShortcut (no_key);
        m_find_files_action->setShortcut (no_key);
        m_clear_command_history_action->setShortcut (no_key);
        m_clear_command_window_action->setShortcut (no_key);
        m_clear_workspace_action->setShortcut (no_key);

        // window menu
        m_reset_windows_action->setShortcut (no_key);

        // help menu
        m_ondisk_doc_action->setShortcut (no_key);
        m_online_doc_action->setShortcut (no_key);
        m_report_bug_action->setShortcut (no_key);
        m_octave_packages_action->setShortcut (no_key);
        m_contribute_action->setShortcut (no_key);
        m_developer_action->setShortcut (no_key);
        m_about_octave_action->setShortcut (no_key);

        // news menu
        m_release_notes_action->setShortcut (no_key);
        m_current_news_action->setShortcut (no_key);
      }
  }

  void main_window::set_screen_size (int ht, int wd)
  {
    emit interpreter_event
      ([ht, wd] (void)
       {
         // INTERPRETER THREAD

         command_editor::set_screen_size (ht, wd);
       });
  }

  void main_window::clipboard_has_changed (void)
  {
    if (m_clipboard->text ().isEmpty ())
      {
        m_paste_action->setEnabled (false);
        m_clear_clipboard_action->setEnabled (false);
      }
    else
      {
        m_paste_action->setEnabled (true);
        m_clear_clipboard_action->setEnabled (true);
      }
  }

  void main_window::clear_clipboard (void)
  {
    m_clipboard->clear (QClipboard::Clipboard);
  }

  void main_window::disable_menu_shortcuts (bool disable)
  {
    QHash<QMenu*, QStringList>::const_iterator i = m_hash_menu_text.constBegin ();

    while (i != m_hash_menu_text.constEnd ())
      {
        i.key ()->setTitle (i.value ().at (disable));
        ++i;
      }
  }

  void main_window::restore_create_file_setting (void)
  {
    // restore the new files creation setting
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    settings->setValue (ed_create_new_file.key, false);
    disconnect (m_editor_window, SIGNAL (file_loaded_signal (void)),
                this, SLOT (restore_create_file_setting (void)));
  }

  void main_window::set_file_encoding (const QString& new_encoding)
  {
    m_file_encoding = new_encoding;
  }

  // The following slot is called after files have been selected in the
  // open file dialog, possibly with a new selected encoding stored in
  // m_file_encoding
  void main_window::request_open_files (const QStringList& open_file_names)
  {
    for (int i = 0; i < open_file_names.count (); i++)
      emit open_file_signal (open_file_names.at (i), m_file_encoding, -1);
  }

  void main_window::edit_variable (const QString &expr, const octave_value& val)
  {
    m_variable_editor_window->edit_variable (expr, val);

    if (! m_variable_editor_window->isVisible ())
      {
        m_variable_editor_window->show ();
        m_variable_editor_window->raise ();
      }

  }

  void main_window::refresh_variable_editor (void)
  {
    m_variable_editor_window->refresh ();
  }

  void main_window::handle_variable_editor_update (void)
  {
    // Called when the variable editor emits the updated signal.  The size
    // of a variable may have changed, so we refresh the workspace in the
    // interpreter.  That will eventually cause the workspace view in the
    // GUI to be updated.

    emit interpreter_event
      ([] (interpreter& interp)
       {
         // INTERPRETER THREAD

         tree_evaluator& tw = interp.get_evaluator ();

         event_manager& xevmgr = interp.get_event_manager ();

         xevmgr.set_workspace (true, tw.get_symbol_info (), false);
       });
  }

  void main_window::closeEvent (QCloseEvent *e)
  {
    if (confirm_shutdown ())
      {
        // FIXME: Instead of ignoring the event and posting an
        // interpreter event, should we just accept the event and
        // shutdown and clean up the interprter as part of closing the
        // GUI?  Going that route might make it easier to close the GUI
        // without having to stop the interpreter, for example, if the
        // GUI is started from the interpreter command line.

        e->ignore ();

        emit interpreter_event
          ([] (interpreter& interp)
           {
             // INTERPRETER THREAD

             interp.quit (0, false, false);
           });
      }
    else
      e->ignore ();
  }

  void main_window::construct_central_widget (void)
  {
    // Create and set the central widget.  QMainWindow takes ownership of
    // the widget (pointer) so there is no need to delete the object upon
    // destroying this main_window.

    QWidget *dummyWidget = new QWidget ();
    dummyWidget->setObjectName ("CentralDummyWidget");
    dummyWidget->resize (10, 10);
    dummyWidget->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
    dummyWidget->hide ();
    setCentralWidget (dummyWidget);
  }

// Main subroutine of the constructor

  void main_window::construct (void)
  {
    setWindowIcon (QIcon (":/actions/icons/logo.png"));

    m_workspace_window->setModel (m_workspace_model);

    connect (m_workspace_model, SIGNAL (model_changed (void)),
             m_workspace_window, SLOT (handle_model_changed (void)));

    interpreter_qobject *interp_qobj = m_octave_qobj.interpreter_qobj ();

    qt_interpreter_events *qt_link = interp_qobj->qt_link ();

    connect (qt_link,
             SIGNAL (edit_variable_signal (const QString&,
                                           const octave_value&)),
             this,
             SLOT (edit_variable (const QString&, const octave_value&)));

    connect (qt_link, SIGNAL (refresh_variable_editor_signal (void)),
             this, SLOT (refresh_variable_editor (void)));

    connect (m_workspace_window,
             SIGNAL (rename_variable_signal (const QString&, const QString&)),
             this,
             SLOT (handle_rename_variable_request (const QString&,
                                                   const QString&)));

    connect (m_variable_editor_window, SIGNAL (updated (void)),
             this, SLOT (handle_variable_editor_update (void)));

    construct_menu_bar ();

    construct_tool_bar ();

    // Order is important.  Deleting gui_settings must be last.
    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_command_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_history_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_file_browser_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_doc_browser_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_workspace_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_editor_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             m_variable_editor_window, SLOT (save_settings (void)));

    connect (qApp, SIGNAL (aboutToQuit (void)),
             this, SLOT (prepare_to_exit (void)));

    connect (qApp, SIGNAL (focusChanged (QWidget*, QWidget*)),
             this, SLOT (focus_changed (QWidget*, QWidget*)));

    connect (this, SIGNAL (settings_changed (const gui_settings *)),
             this, SLOT (notice_settings (const gui_settings *)));

    connect (this, SIGNAL (editor_focus_changed (bool)),
             this, SLOT (disable_menu_shortcuts (bool)));

    connect (this, SIGNAL (editor_focus_changed (bool)),
             m_editor_window, SLOT (enable_menu_shortcuts (bool)));

    connect (this, SIGNAL (step_into_file_signal (void)),
             m_editor_window, SLOT (request_step_into_file (void)));

    connect (m_editor_window, SIGNAL (editor_tabs_changed_signal (bool)),
             this, SLOT (editor_tabs_changed (bool)));

    connect (m_editor_window,
             SIGNAL (request_open_file_external (const QString&, int)),
             m_external_editor,
             SLOT (call_custom_editor (const QString&, int)));

    connect (m_external_editor,
             SIGNAL (request_settings_dialog (const QString&)),
             this, SLOT (process_settings_dialog_request (const QString&)));

    connect (m_file_browser_window, SIGNAL (load_file_signal (const QString&)),
             this, SLOT (handle_load_workspace_request (const QString&)));

    connect (m_file_browser_window, SIGNAL (open_any_signal (const QString&)),
             this, SLOT (handle_open_any_request (const QString&)));

    connect (m_file_browser_window, SIGNAL (find_files_signal (const QString&)),
             this, SLOT (find_files (const QString&)));

    // Connections for signals from the interpreter thread where the slot
    // should be executed by the gui thread

    connect (this, SIGNAL (warning_function_not_found_signal (const QString&)),
             this, SLOT (warning_function_not_found (const QString&)));

    // Build the window with widgets

    setWindowTitle ("Octave");

    // See Octave bug #53409 and https://bugreports.qt.io/browse/QTBUG-55357
#if (QT_VERSION < 0x050601) || (QT_VERSION >= 0x050701)
    setDockOptions (QMainWindow::AnimatedDocks
                    | QMainWindow::AllowNestedDocks
                    | QMainWindow::AllowTabbedDocks);
#else
    setDockNestingEnabled (true);
#endif

    addDockWidget (Qt::RightDockWidgetArea, m_command_window);
    addDockWidget (Qt::RightDockWidgetArea, m_doc_browser_window);
    tabifyDockWidget (m_command_window, m_doc_browser_window);

#if defined (HAVE_QSCINTILLA)
    addDockWidget (Qt::RightDockWidgetArea, m_editor_window);
    tabifyDockWidget (m_command_window, m_editor_window);
#endif
    addDockWidget (Qt::RightDockWidgetArea, m_variable_editor_window);
    tabifyDockWidget (m_command_window, m_variable_editor_window);

    addDockWidget (Qt::LeftDockWidgetArea, m_file_browser_window);
    addDockWidget (Qt::LeftDockWidgetArea, m_workspace_window);
    addDockWidget (Qt::LeftDockWidgetArea, m_history_window);

    int win_x = QApplication::desktop ()->width ();
    int win_y = QApplication::desktop ()->height ();

    if (win_x > 960)
      win_x = 960;

    if (win_y > 720)
      win_y = 720;

    setGeometry (0, 0, win_x, win_y);   // excluding frame geometry
    move (0, 0);                        // including frame geometry

    setStatusBar (m_status_bar);

#if defined (HAVE_QSCINTILLA)
    connect (this,
             SIGNAL (insert_debugger_pointer_signal (const QString&, int)),
             m_editor_window,
             SLOT (handle_insert_debugger_pointer_request (const QString&,
                                                           int)));

    connect (this,
             SIGNAL (delete_debugger_pointer_signal (const QString&, int)),
             m_editor_window,
             SLOT (handle_delete_debugger_pointer_request (const QString&,
                                                           int)));

    connect (this,
             SIGNAL (update_breakpoint_marker_signal (bool, const QString&,
                                                      int, const QString&)),
             m_editor_window,
             SLOT (handle_update_breakpoint_marker_request (bool,
                                                            const QString&,
                                                            int,
                                                            const QString&)));

    // Signals for removing/renaming files/dirs in the file browser
    connect (m_file_browser_window,
             SIGNAL (file_remove_signal (const QString&, const QString&)),
             m_editor_window,
             SLOT (handle_file_remove (const QString&, const QString&)));

    connect (m_file_browser_window, SIGNAL (file_renamed_signal (bool)),
             m_editor_window, SLOT (handle_file_renamed (bool)));

    // Signals for removing/renaming files/dirs in the terminal window
    connect (qt_link, SIGNAL (file_renamed_signal (bool)),
             m_editor_window, SLOT (handle_file_renamed (bool)));

    // Signals for entering/exiting debug mode
    connect (qt_link, SIGNAL (enter_debugger_signal (void)),
             m_editor_window, SLOT (handle_enter_debug_mode (void)));

    connect (qt_link, SIGNAL (exit_debugger_signal (void)),
             m_editor_window, SLOT (handle_exit_debug_mode (void)));
#endif

    // Signals for removing/renaming files/dirs in the temrinal window
    connect (qt_link,
             SIGNAL (file_remove_signal (const QString&, const QString&)),
             this, SLOT (file_remove_proxy (const QString&, const QString&)));

    connect (this, SIGNAL (interpreter_event (const fcn_callback&)),
             &m_octave_qobj, SLOT (interpreter_event (const fcn_callback&)));

    connect (this, SIGNAL (interpreter_event (const meth_callback&)),
             &m_octave_qobj, SLOT (interpreter_event (const meth_callback&)));

    configure_shortcuts ();
  }

  void main_window::construct_octave_qt_link (void)
  {
    interpreter_qobject *interp_qobj = m_octave_qobj.interpreter_qobj ();

    qt_interpreter_events *qt_link = interp_qobj->qt_link ();

    connect (qt_link, SIGNAL (settings_changed (const gui_settings *)),
             this, SLOT (notice_settings (const gui_settings *)));

    connect (qt_link, SIGNAL (apply_new_settings (void)),
             this, SLOT (request_reload_settings (void)));

    connect (qt_link,
             SIGNAL (set_workspace_signal (bool, bool, const symbol_info_list&)),
             m_workspace_model,
             SLOT (set_workspace (bool, bool, const symbol_info_list&)));

    connect (qt_link, SIGNAL (clear_workspace_signal (void)),
             m_workspace_model, SLOT (clear_workspace (void)));

    connect (qt_link, SIGNAL (directory_changed_signal (QString)),
             this, SLOT (update_octave_directory (QString)));

    connect (qt_link, SIGNAL (directory_changed_signal (QString)),
             m_file_browser_window, SLOT (update_octave_directory (QString)));

    connect (qt_link, SIGNAL (directory_changed_signal (QString)),
             m_editor_window, SLOT (update_octave_directory (QString)));

    connect (qt_link,
             SIGNAL (execute_command_in_terminal_signal (QString)),
             this, SLOT (execute_command_in_terminal (QString)));

    connect (qt_link,
             SIGNAL (set_history_signal (const QStringList&)),
             m_history_window, SLOT (set_history (const QStringList&)));

    connect (qt_link,
             SIGNAL (append_history_signal (const QString&)),
             m_history_window, SLOT (append_history (const QString&)));

    connect (qt_link,
             SIGNAL (clear_history_signal (void)),
             m_history_window, SLOT (clear_history (void)));

    connect (qt_link, SIGNAL (enter_debugger_signal (void)),
             this, SLOT (handle_enter_debugger (void)));

    connect (qt_link, SIGNAL (exit_debugger_signal (void)),
             this, SLOT (handle_exit_debugger (void)));

    connect (qt_link,
             SIGNAL (show_preferences_signal (void)),
             this, SLOT (process_settings_dialog_request (void)));

    connect (qt_link,
             SIGNAL (edit_file_signal (const QString&)),
             m_active_editor,
             SLOT (handle_edit_file_request (const QString&)));

    connect (qt_link,
             SIGNAL (insert_debugger_pointer_signal (const QString&, int)),
             this,
             SLOT (handle_insert_debugger_pointer_request (const QString&,
                                                           int)));

    connect (qt_link,
             SIGNAL (delete_debugger_pointer_signal (const QString&, int)),
             this,
             SLOT (handle_delete_debugger_pointer_request (const QString&,
                                                           int)));

    connect (qt_link,
             SIGNAL (update_breakpoint_marker_signal (bool, const QString&,
                                                      int, const QString&)),
             this,
             SLOT (handle_update_breakpoint_marker_request (bool, const QString&,
                                                            int, const QString&)));

    connect (qt_link,
             SIGNAL (show_doc_signal (const QString &)),
             this, SLOT (handle_show_doc (const QString &)));

    connect (qt_link,
             SIGNAL (register_doc_signal (const QString &)),
             this, SLOT (handle_register_doc (const QString &)));

    connect (qt_link,
             SIGNAL (unregister_doc_signal (const QString &)),
             this, SLOT (handle_unregister_doc (const QString &)));

    connect (qt_link, SIGNAL (update_gui_lexer_signal (bool)),
             this, SIGNAL (update_gui_lexer_signal (bool)));
  }

  QAction* main_window::add_action (QMenu *menu, const QIcon& icon,
                                    const QString& text, const char *member,
                                    const QWidget *receiver)
  {
    QAction *a;

    if (receiver)
      a = menu->addAction (icon, text, receiver, member);
    else
      a = menu->addAction (icon, text, this, member);

    addAction (a);  // important for shortcut context
    a->setShortcutContext (Qt::ApplicationShortcut);
    return a;
  }

  QMenu* main_window::m_add_menu (QMenuBar *p, QString name)
  {
    QMenu *menu = p->addMenu (name);

    QString base_name = name;  // get a copy
    // replace intended '&' ("&&") by a temp. string
    base_name.replace ("&&", "___octave_amp_replacement___");
    // remove single '&' (shortcut)
    base_name.remove ("&");
    // restore intended '&'
    base_name.replace ("___octave_amp_replacement___", "&&");

    // remember names with and without shortcut
    m_hash_menu_text[menu] = QStringList () << name << base_name;

    return menu;
  }

  void main_window::construct_menu_bar (void)
  {
    QMenuBar *menu_bar = menuBar ();

    construct_file_menu (menu_bar);

    construct_edit_menu (menu_bar);

    construct_debug_menu (menu_bar);

    construct_window_menu (menu_bar);

    construct_help_menu (menu_bar);

    construct_news_menu (menu_bar);

#if defined (HAVE_QSCINTILLA)
    // call the editor to add actions which should also be available in the
    // editor's menu and tool bar
    QList<QAction*> shared_actions;
    shared_actions << m_new_script_action
                   << m_new_function_action
                   << m_open_action
                   << m_find_files_action
                   << m_undo_action
                   << m_copy_action
                   << m_paste_action
                   <<m_select_all_action;
    m_editor_window->insert_global_actions (shared_actions);
#endif
  }

  void main_window::construct_file_menu (QMenuBar *p)
  {
    QMenu *file_menu = m_add_menu (p, tr ("&File"));

    construct_new_menu (file_menu);

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    m_open_action
      = file_menu->addAction (rmgr.icon ("document-open"), tr ("Open..."));
    m_open_action->setShortcutContext (Qt::ApplicationShortcut);
    m_open_action->setToolTip (tr ("Open an existing file in editor"));

#if defined (HAVE_QSCINTILLA)
    file_menu->addMenu (m_editor_window->get_mru_menu ());
#endif

    file_menu->addSeparator ();

    m_load_workspace_action
      = file_menu->addAction (tr ("Load Workspace..."));

    m_save_workspace_action
      = file_menu->addAction (tr ("Save Workspace As..."));

    file_menu->addSeparator ();

    m_exit_action = file_menu->addAction (tr ("Exit"));
    m_exit_action->setMenuRole (QAction::QuitRole);
    m_exit_action->setShortcutContext (Qt::ApplicationShortcut);

    connect (m_open_action, SIGNAL (triggered (void)),
             this, SLOT (request_open_file (void)));

    connect (m_load_workspace_action, SIGNAL (triggered (void)),
             this, SLOT (handle_load_workspace_request (void)));

    connect (m_save_workspace_action, SIGNAL (triggered (void)),
             this, SLOT (handle_save_workspace_request (void)));

    connect (m_exit_action, SIGNAL (triggered (void)),
             this, SLOT (close (void)));
  }

  void main_window::construct_new_menu (QMenu *p)
  {
    QMenu *new_menu = p->addMenu (tr ("New"));

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    m_new_script_action
      = new_menu->addAction (rmgr.icon ("document-new"), tr ("New Script"));
    m_new_script_action->setShortcutContext (Qt::ApplicationShortcut);

    m_new_function_action = new_menu->addAction (tr ("New Function..."));
    m_new_function_action->setEnabled (true);
    m_new_function_action->setShortcutContext (Qt::ApplicationShortcut);

    m_new_figure_action = new_menu->addAction (tr ("New Figure"));
    m_new_figure_action->setEnabled (true);

    connect (m_new_script_action, SIGNAL (triggered (void)),
             this, SLOT (request_new_script (void)));

    connect (m_new_function_action, SIGNAL (triggered (void)),
             this, SLOT (request_new_function (void)));

    connect (this, SIGNAL (new_file_signal (const QString&)),
             m_active_editor, SLOT (request_new_file (const QString&)));

    connect (this, SIGNAL (open_file_signal (const QString&)),
             m_active_editor, SLOT (request_open_file (const QString&)));

    connect (this,
             SIGNAL (open_file_signal (const QString&, const QString&, int)),
             m_active_editor,
             SLOT (request_open_file (const QString&, const QString&, int)));

    connect (m_new_figure_action, SIGNAL (triggered (void)),
             this, SLOT (handle_new_figure_request (void)));
  }

  void main_window::construct_edit_menu (QMenuBar *p)
  {
    QMenu *edit_menu = m_add_menu (p, tr ("&Edit"));

    QKeySequence ctrl_shift = Qt::ControlModifier + Qt::ShiftModifier;

    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    m_undo_action
      = edit_menu->addAction (rmgr.icon ("edit-undo"), tr ("Undo"));
    m_undo_action->setShortcutContext (Qt::ApplicationShortcut);

    edit_menu->addSeparator ();

    m_copy_action
      = edit_menu->addAction (rmgr.icon ("edit-copy"), tr ("Copy"), this,
                              SLOT (copyClipboard (void)));
    m_copy_action->setShortcutContext (Qt::ApplicationShortcut);

    m_paste_action
      = edit_menu->addAction (rmgr.icon ("edit-paste"), tr ("Paste"), this,
                              SLOT (pasteClipboard (void)));
    m_paste_action->setShortcutContext (Qt::ApplicationShortcut);

    m_select_all_action
      = edit_menu->addAction (tr ("Select All"), this, SLOT (selectAll (void)));
    m_select_all_action->setShortcutContext (Qt::ApplicationShortcut);

    m_clear_clipboard_action
      = edit_menu->addAction (tr ("Clear Clipboard"), this,
                              SLOT (clear_clipboard (void)));

    edit_menu->addSeparator ();

    m_find_files_action
      = edit_menu->addAction (rmgr.icon ("edit-find"), tr ("Find Files..."));

    edit_menu->addSeparator ();

    m_clear_command_window_action
      = edit_menu->addAction (tr ("Clear Command Window"));

    m_clear_command_history_action
      = edit_menu->addAction (tr ("Clear Command History"));

    m_clear_workspace_action
      = edit_menu->addAction (tr ("Clear Workspace"));

    edit_menu->addSeparator ();

    m_set_path_action
      = edit_menu->addAction (tr ("Set Path"));

    m_preferences_action
      = edit_menu->addAction (rmgr.icon ("preferences-system"),
                              tr ("Preferences..."));

    connect (m_find_files_action, SIGNAL (triggered (void)),
             this, SLOT (find_files (void)));

    connect (m_clear_command_window_action, SIGNAL (triggered (void)),
             this, SLOT (handle_clear_command_window_request (void)));

    connect (m_clear_command_history_action, SIGNAL (triggered (void)),
             this, SLOT (handle_clear_history_request (void)));

    connect (m_clear_workspace_action, SIGNAL (triggered (void)),
             this, SLOT (handle_clear_workspace_request (void)));

    connect (m_clipboard, SIGNAL (dataChanged (void)),
             this, SLOT (clipboard_has_changed (void)));
    clipboard_has_changed ();
#if defined (Q_OS_WIN32)
    // Always enable paste action (unreliable clipboard signals in windows)
    // FIXME: This has to be removed, when the clipboard signals in windows
    //        are working again
    m_paste_action->setEnabled (true);
    m_clear_clipboard_action->setEnabled (true);
#endif

    connect (m_preferences_action, SIGNAL (triggered (void)),
             this, SLOT (process_settings_dialog_request (void)));

    connect (m_set_path_action, SIGNAL (triggered (void)),
             this, SLOT (handle_set_path_dialog_request (void)));

  }

  QAction * main_window::construct_debug_menu_item (const char *icon,
                                                    const QString& item,
                                                    const char *member)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    QAction *action = add_action (m_debug_menu, rmgr.icon (QString (icon)),
                                  item, member);

    action->setEnabled (false);

#if defined (HAVE_QSCINTILLA)
    m_editor_window->debug_menu ()->addAction (action);
    m_editor_window->toolbar ()->addAction (action);
#endif

    return action;
  }

  void main_window::construct_debug_menu (QMenuBar *p)
  {
    m_debug_menu = m_add_menu (p, tr ("De&bug"));

    m_debug_step_over
      = construct_debug_menu_item ("db-step", tr ("Step"),
                                   SLOT (debug_step_over (void)));

    m_debug_step_into
      = construct_debug_menu_item ("db-step-in", tr ("Step In"),
                                   SLOT (debug_step_into (void)));

    m_debug_step_out
      = construct_debug_menu_item ("db-step-out", tr ("Step Out"),
                                   SLOT (debug_step_out (void)));

    m_debug_continue
      = construct_debug_menu_item ("db-cont", tr ("Continue"),
                                   SLOT (debug_continue (void)));

    m_debug_menu->addSeparator ();
#if defined (HAVE_QSCINTILLA)
    m_editor_window->debug_menu ()->addSeparator ();
#endif

    m_debug_quit
      = construct_debug_menu_item ("db-stop", tr ("Quit Debug Mode"),
                                   SLOT (debug_quit (void)));
  }

  void main_window::editor_tabs_changed (bool have_tabs)
  {
    // Set state of actions which depend on the existence of editor tabs
    m_editor_has_tabs = have_tabs;
    m_debug_step_over->setEnabled (have_tabs);
  }

  QAction * main_window::construct_window_menu_item (QMenu *p,
                                                     const QString& item,
                                                     bool checkable,
                                                     QWidget *widget)
  {
    QAction *action = p->addAction (QIcon (), item);

    addAction (action);  // important for shortcut context
    action->setCheckable (checkable);
    action->setShortcutContext (Qt::ApplicationShortcut);

    if (widget)  // might be zero for m_editor_window
      {
        if (checkable)
          {
            // action for visibility of dock widget
            connect (action, SIGNAL (toggled (bool)),
                     widget, SLOT (setVisible (bool)));

            connect (widget, SIGNAL (active_changed (bool)),
                     action, SLOT (setChecked (bool)));
          }
        else
          {
            // action for focus of dock widget
            connect (action, SIGNAL (triggered (void)), widget, SLOT (activate (void)));
          }
      }
    else
      {
        action->setEnabled (false);
      }

    return action;
  }

  void main_window::construct_window_menu (QMenuBar *p)
  {
    QMenu *window_menu = m_add_menu (p, tr ("&Window"));

    m_show_command_window_action = construct_window_menu_item
      (window_menu, tr ("Show Command Window"), true, m_command_window);

    m_show_history_action = construct_window_menu_item
      (window_menu, tr ("Show Command History"), true, m_history_window);

    m_show_file_browser_action = construct_window_menu_item
      (window_menu, tr ("Show File Browser"), true, m_file_browser_window);

    m_show_workspace_action = construct_window_menu_item
      (window_menu, tr ("Show Workspace"), true, m_workspace_window);

    m_show_editor_action = construct_window_menu_item
      (window_menu, tr ("Show Editor"), true, m_editor_window);

    m_show_documentation_action = construct_window_menu_item
      (window_menu, tr ("Show Documentation"), true, m_doc_browser_window);

    m_show_variable_editor_action = construct_window_menu_item
      (window_menu, tr ("Show Variable Editor"), true, m_variable_editor_window);

    window_menu->addSeparator ();

    m_command_window_action = construct_window_menu_item
      (window_menu, tr ("Command Window"), false, m_command_window);

    m_history_action = construct_window_menu_item
      (window_menu, tr ("Command History"), false, m_history_window);

    m_file_browser_action = construct_window_menu_item
      (window_menu, tr ("File Browser"), false, m_file_browser_window);

    m_workspace_action = construct_window_menu_item
      (window_menu, tr ("Workspace"), false, m_workspace_window);

    m_editor_action = construct_window_menu_item
      (window_menu, tr ("Editor"), false, m_editor_window);

    m_documentation_action = construct_window_menu_item
      (window_menu, tr ("Documentation"), false, m_doc_browser_window);

    m_variable_editor_action = construct_window_menu_item
      (window_menu, tr ("Variable Editor"), false, m_variable_editor_window);

    window_menu->addSeparator ();

    m_previous_dock_action = add_action (window_menu, QIcon (),
                                           tr ("Previous Widget"), SLOT (go_to_previous_widget (void)));

    window_menu->addSeparator ();

    m_reset_windows_action = add_action (window_menu, QIcon (),
                                         tr ("Reset Default Window Layout"), SLOT (reset_windows (void)));
  }

  void main_window::construct_help_menu (QMenuBar *p)
  {
    QMenu *help_menu = m_add_menu (p, tr ("&Help"));

    construct_documentation_menu (help_menu);

    help_menu->addSeparator ();

    m_report_bug_action = add_action (help_menu, QIcon (),
                                      tr ("Report Bug"), SLOT (open_bug_tracker_page ()));

    m_octave_packages_action = add_action (help_menu, QIcon (),
                                           tr ("Octave Packages"), SLOT (open_octave_packages_page ()));

    m_contribute_action = add_action (help_menu, QIcon (),
                                      tr ("Contribute"), SLOT (open_contribute_page ()));

    m_developer_action = add_action (help_menu, QIcon (),
                                     tr ("Donate to Octave"), SLOT (open_donate_page ()));

    help_menu->addSeparator ();

    m_about_octave_action = add_action (help_menu, QIcon (),
                                        tr ("About Octave"), SLOT (show_about_octave ()));
  }

  void main_window::construct_documentation_menu (QMenu *p)
  {
    QMenu *doc_menu = p->addMenu (tr ("Documentation"));

    m_ondisk_doc_action = add_action (doc_menu, QIcon (),
                                      tr ("On Disk"), SLOT (activate ()), m_doc_browser_window);

    m_online_doc_action = add_action (doc_menu, QIcon (),
                                      tr ("Online"), SLOT (open_online_documentation_page ()));
  }

  void main_window::construct_news_menu (QMenuBar *p)
  {
    QMenu *news_menu = m_add_menu (p, tr ("&News"));

    m_release_notes_action = add_action (news_menu, QIcon (),
                                         tr ("Release Notes"), SLOT (display_release_notes ()));

    m_current_news_action = add_action (news_menu, QIcon (),
                                        tr ("Community News"), SLOT (load_and_display_community_news ()));
  }

  void main_window::construct_tool_bar (void)
  {
    m_main_tool_bar = addToolBar (tr ("Toolbar"));
    m_main_tool_bar->setStyleSheet (m_main_tool_bar->styleSheet ()
                                    + global_toolbar_style);

    m_main_tool_bar->setObjectName ("MainToolBar");
    m_main_tool_bar->addAction (m_new_script_action);
    m_main_tool_bar->addAction (m_open_action);

    m_main_tool_bar->addSeparator ();

    m_main_tool_bar->addAction (m_copy_action);
    m_main_tool_bar->addAction (m_paste_action);
    m_main_tool_bar->addAction (m_undo_action);

    m_main_tool_bar->addSeparator ();

    m_current_directory_combo_box = new QComboBox (this);
    QFontMetrics fm = m_current_directory_combo_box->fontMetrics ();
    m_current_directory_combo_box->setFixedWidth (48*fm.averageCharWidth ());
    m_current_directory_combo_box->setEditable (true);
    m_current_directory_combo_box->setInsertPolicy (QComboBox::NoInsert);
    m_current_directory_combo_box->setToolTip (tr ("Enter directory name"));
    m_current_directory_combo_box->setMaxVisibleItems (current_directory_max_visible);
    m_current_directory_combo_box->setMaxCount (current_directory_max_count);
    QSizePolicy sizePol (QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_current_directory_combo_box->setSizePolicy (sizePol);

    // addWidget takes ownership of the objects so there is no
    // need to delete these upon destroying this main_window.
    m_main_tool_bar->addWidget (new QLabel (tr ("Current Directory: ")));
    m_main_tool_bar->addWidget (m_current_directory_combo_box);
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    QAction *current_dir_up
      = m_main_tool_bar->addAction (rmgr.icon ("go-up"),
                                    tr ("One directory up"));
    QAction *current_dir_search
      = m_main_tool_bar->addAction (rmgr.icon ("folder"),
                                    tr ("Browse directories"));

    connect (m_current_directory_combo_box, SIGNAL (activated (QString)),
             this, SLOT (set_current_working_directory (QString)));

    connect (m_current_directory_combo_box->lineEdit (), SIGNAL (returnPressed (void)),
             this, SLOT (accept_directory_line_edit (void)));

    connect (current_dir_search, SIGNAL (triggered (void)),
             this, SLOT (browse_for_directory (void)));

    connect (current_dir_up, SIGNAL (triggered (void)),
             this, SLOT (change_directory_up (void)));

    connect (m_undo_action, SIGNAL (triggered (void)),
             this, SLOT (handle_undo_request (void)));
  }

  void main_window::focus_console_after_command (void)
  {
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();
    if (settings->value (cs_focus_cmd).toBool ())
      focus_command_window ();
  }

  void main_window::configure_shortcuts (void)
  {
    shortcut_manager& scmgr = m_octave_qobj.get_shortcut_manager ();

    // file menu
    scmgr.set_shortcut (m_open_action, sc_main_file_open_file);
    scmgr.set_shortcut (m_new_script_action, sc_main_file_new_file);
    scmgr.set_shortcut (m_new_function_action, sc_main_file_new_function);
    scmgr.set_shortcut (m_new_figure_action, sc_main_file_new_figure);
    scmgr.set_shortcut (m_load_workspace_action, sc_main_file_load_workspace);
    scmgr.set_shortcut (m_save_workspace_action, sc_main_file_save_workspace);
    scmgr.set_shortcut (m_exit_action, sc_main_file_exit);

    // edit menu
    scmgr.set_shortcut (m_copy_action, sc_main_edit_copy);
    scmgr.set_shortcut (m_paste_action, sc_main_edit_paste);
    scmgr.set_shortcut (m_undo_action, sc_main_edit_undo);
    scmgr.set_shortcut (m_select_all_action, sc_main_edit_select_all);
    scmgr.set_shortcut (m_clear_clipboard_action, sc_main_edit_clear_clipboard);
    scmgr.set_shortcut (m_find_files_action, sc_main_edit_find_in_files);
    scmgr.set_shortcut (m_clear_command_history_action, sc_main_edit_clear_history);
    scmgr.set_shortcut (m_clear_command_window_action, sc_main_edit_clear_command_window);
    scmgr.set_shortcut (m_clear_workspace_action, sc_main_edit_clear_workspace);
    scmgr.set_shortcut (m_set_path_action, sc_main_edit_set_path);
    scmgr.set_shortcut (m_preferences_action, sc_main_edit_preferences);

    // debug menu
    scmgr.set_shortcut (m_debug_step_over, sc_main_debug_step_over);
    scmgr.set_shortcut (m_debug_step_into, sc_main_debug_step_into);
    scmgr.set_shortcut (m_debug_step_out, sc_main_debug_step_out);
    scmgr.set_shortcut (m_debug_continue, sc_main_debug_continue);
    scmgr.set_shortcut (m_debug_quit, sc_main_debug_quit);

    // window menu
    scmgr.set_shortcut (m_show_command_window_action, sc_main_window_show_command);
    scmgr.set_shortcut (m_show_history_action, sc_main_window_show_history);
    scmgr.set_shortcut (m_show_workspace_action, sc_main_window_show_workspace);
    scmgr.set_shortcut (m_show_file_browser_action, sc_main_window_show_file_browser);
    scmgr.set_shortcut (m_show_editor_action, sc_main_window_show_editor);
    scmgr.set_shortcut (m_show_documentation_action, sc_main_window_show_doc);
    scmgr.set_shortcut (m_show_variable_editor_action, sc_main_window_show_variable_editor);
    scmgr.set_shortcut (m_command_window_action, sc_main_window_command);
    scmgr.set_shortcut (m_history_action, sc_main_window_history);
    scmgr.set_shortcut (m_workspace_action, sc_main_window_workspace);
    scmgr.set_shortcut (m_file_browser_action, sc_main_window_file_browser);
    scmgr.set_shortcut (m_editor_action, sc_main_window_editor);
    scmgr.set_shortcut (m_documentation_action, sc_main_window_doc);
    scmgr.set_shortcut (m_variable_editor_action, sc_main_window_variable_editor);
    scmgr.set_shortcut (m_previous_dock_action, sc_main_window_previous_dock);
    scmgr.set_shortcut (m_reset_windows_action, sc_main_window_reset);

    // help menu
    scmgr.set_shortcut (m_ondisk_doc_action, sc_main_help_ondisk_doc);
    scmgr.set_shortcut (m_online_doc_action, sc_main_help_online_doc);
    scmgr.set_shortcut (m_report_bug_action, sc_main_help_report_bug);
    scmgr.set_shortcut (m_octave_packages_action, sc_main_help_packages);
    scmgr.set_shortcut (m_contribute_action, sc_main_help_contribute);
    scmgr.set_shortcut (m_developer_action, sc_main_help_developer);
    scmgr.set_shortcut (m_about_octave_action, sc_main_help_about);

    // news menu
    scmgr.set_shortcut (m_release_notes_action, sc_main_news_release_notes);
    scmgr.set_shortcut (m_current_news_action, sc_main_news_community_news);
  }

  QList<octave_dock_widget *> main_window::dock_widget_list (void)
  {
    QList<octave_dock_widget *> list = QList<octave_dock_widget *> ();
    list.append (static_cast<octave_dock_widget *> (m_command_window));
    list.append (static_cast<octave_dock_widget *> (m_history_window));
    list.append (static_cast<octave_dock_widget *> (m_file_browser_window));
    list.append (static_cast<octave_dock_widget *> (m_doc_browser_window));
#if defined (HAVE_QSCINTILLA)
    list.append (static_cast<octave_dock_widget *> (m_editor_window));
#endif
    list.append (static_cast<octave_dock_widget *> (m_workspace_window));
    list.append (static_cast<octave_dock_widget *> (m_variable_editor_window));
    return list;
  }

  void main_window::update_default_encoding (const QString& default_encoding)
  {
    m_default_encoding = default_encoding;
    std::string mfile_encoding = m_default_encoding.toStdString ();
    if (m_default_encoding.startsWith ("SYSTEM", Qt::CaseInsensitive))
      mfile_encoding = "SYSTEM";

    emit interpreter_event
      ([mfile_encoding] (interpreter& interp)
       {
         // INTERPRETER THREAD

         F__mfile_encoding__ (interp, ovl (mfile_encoding));
       });
  }
}
