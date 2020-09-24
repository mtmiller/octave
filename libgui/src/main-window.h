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

#if ! defined (octave_main_window_h)
#define octave_main_window_h 1

// Qt includes
#include <QCloseEvent>
#include <QComboBox>
#include <QMainWindow>
#include <QPointer>
#include <QQueue>
#include <QStatusBar>
#include <QTabWidget>
#include <QThread>
#include <QToolBar>
#include <QToolButton>
#include <QTranslator>

// Editor includes
#include "external-editor-interface.h"
#include "file-editor-interface.h"

// QTerminal includes
#include "QTerminal.h"

// Own includes
#include "dialog.h"
#include "documentation-dock-widget.h"
#include "files-dock-widget.h"
#include "find-files-dialog.h"
#include "history-dock-widget.h"
#include "interpreter-qobject.h"
#include "octave-dock-widget.h"
#include "octave-qobject.h"
#include "qt-interpreter-events.h"
#include "set-path-dialog.h"
#include "terminal-dock-widget.h"
#include "variable-editor.h"
#include "workspace-model.h"
#include "workspace-view.h"

class octave_value;

namespace octave
{
  class interpreter;

  class settings_dialog;

  //! Represents the main window.

  class main_window : public QMainWindow
  {
    Q_OBJECT

  public:

    typedef std::pair <std::string, std::string> name_pair;
    typedef std::pair <int, int> int_pair;

    main_window (base_qobject& oct_qobj);

    ~main_window (void);

    bool command_window_has_focus (void) const;

    void focus_command_window (void);

    bool confirm_shutdown (void);

  signals:

    void active_dock_changed (octave_dock_widget *, octave_dock_widget *);
    void editor_focus_changed (bool);

    void settings_changed (const gui_settings *);
    void init_terminal_size_signal (void);
    void new_file_signal (const QString&);
    void open_file_signal (const QString&);
    void open_file_signal (const QString& file, const QString& enc, int line);
    void step_into_file_signal (void);

    void show_doc_signal (const QString&);
    void register_doc_signal (const QString&);
    void unregister_doc_signal (const QString&);
    void update_gui_lexer_signal (bool);

    void insert_debugger_pointer_signal (const QString& file, int line);
    void delete_debugger_pointer_signal (const QString& file, int line);
    void update_breakpoint_marker_signal (bool insert, const QString& file,
                                          int line, const QString& cond);

    void copyClipboard_signal (void);
    void pasteClipboard_signal (void);
    void selectAll_signal (void);
    void undo_signal (void);

    void add_actions_signal (QList <QAction *> action_list);

    void warning_function_not_found_signal (const QString& message);

    void interpreter_event (const fcn_callback& fcn);
    void interpreter_event (const meth_callback& meth);

  public slots:

    void focus_changed (QWidget *w_old, QWidget *w_new);
    void focus_window (const QString& win_name);
    void request_reload_settings (void);

    void report_status_message (const QString& statusMessage);
    void handle_save_workspace_request (void);
    void handle_load_workspace_request (const QString& file = QString ());
    void handle_open_any_request (const QString& file = QString ());
    void handle_clear_workspace_request (void);
    void handle_clear_command_window_request (void);
    void handle_clear_history_request (void);
    void handle_undo_request (void);
    void handle_rename_variable_request (const QString& old_name,
                                         const QString& new_name);
    void modify_path (const octave_value_list& dir_list, bool rm, bool subdirs);
    void new_file (const QString& commands = QString ());
    void open_file (const QString& file_name = QString (), int line = -1);
    void edit_mfile (const QString&, int);
    void file_remove_proxy (const QString& o, const QString& n);
    void open_online_documentation_page (void);
    void display_release_notes (void);
    void load_and_display_community_news (int serial = -1);
    void display_community_news (const QString& news);
    void open_bug_tracker_page (void);
    void open_octave_packages_page (void);
    void open_contribute_page (void);
    void open_donate_page (void);
    void process_settings_dialog_request (const QString& desired_tab
                                          = QString ());

    void show_about_octave (void);
    void notice_settings (const gui_settings *settings);
    void prepare_to_exit (void);
    void go_to_previous_widget (void);
    void reset_windows (void);

    void update_octave_directory (const QString& dir);
    void browse_for_directory (void);
    void set_current_working_directory (const QString& dir);
    void change_directory_up (void);
    void accept_directory_line_edit (void);

    void execute_command_in_terminal (const QString& dir);
    void run_file_in_terminal (const QFileInfo& info);

    void handle_new_figure_request (void);

    void handle_enter_debugger (void);
    void handle_exit_debugger (void);
    void debug_continue (void);
    void debug_step_into (void);
    void debug_step_over (void);
    void debug_step_out (void);
    void debug_quit (void);
    void editor_tabs_changed (bool);

    void request_open_file (void);
    void request_new_script (const QString& commands = QString ());
    void request_new_function (bool triggered = true);
    void handle_edit_mfile_request (const QString& name, const QString& file,
                                    const QString& curr_dir, int line);

    void handle_insert_debugger_pointer_request (const QString& file, int line);
    void handle_delete_debugger_pointer_request (const QString& file, int line);
    void handle_update_breakpoint_marker_request (bool insert,
                                                  const QString& file, int line,
                                                  const QString& cond);

    void read_settings (void);
    void init_terminal_size (void);
    void set_window_layout (gui_settings *settings);
    void write_settings (void);
    void connect_visibility_changed (void);

    void copyClipboard (void);
    void pasteClipboard (void);
    void selectAll (void);

    void focus_console_after_command (void);
    void handle_show_doc (const QString& file);
    void handle_register_doc (const QString& file);
    void handle_unregister_doc (const QString& file);

    void handle_octave_ready ();

    void handle_set_path_dialog_request (void);

    //! Find files dialog.
    //!@{
    void find_files (const QString& startdir = QDir::currentPath ());
    void find_files_finished (int) { }
    //!@}

    //! Setting global shortcuts.

    void set_global_shortcuts (bool enable);

    void set_screen_size (int ht, int wd);

    //! Handling the clipboard.
    //!@{
    void clipboard_has_changed (void);
    void clear_clipboard ();
    //!@}

    //! Returns a list of dock widgets.

    QList<octave_dock_widget *> get_dock_widget_list (void)
    {
      return dock_widget_list ();
    }

  private slots:

    void disable_menu_shortcuts (bool disable);
    void restore_create_file_setting (void);
    void set_file_encoding (const QString& new_encoding);
    void request_open_files (const QStringList& open_file_names);

    void warning_function_not_found (const QString& message);

    //! Opens the variable editor for @p name.

    void edit_variable (const QString &name, const octave_value&);

    void refresh_variable_editor (void);

    void handle_variable_editor_update (void);

  protected:

    void closeEvent (QCloseEvent *closeEvent);

  private:

    void construct_central_widget (void);

    void construct (void);

    void construct_octave_qt_link (void);

    QAction * add_action (QMenu *menu, const QIcon& icon,
                          const QString& text, const char *member,
                          const QWidget *receiver = nullptr);

    QMenu * m_add_menu (QMenuBar *p, QString text);
    void construct_menu_bar (void);
    void construct_file_menu (QMenuBar *p);
    void construct_new_menu (QMenu *p);
    void construct_edit_menu (QMenuBar *p);
    QAction * construct_debug_menu_item (const char *icon, const QString& item,
                                         const char *member);
    void construct_debug_menu (QMenuBar *p);
    QAction * construct_window_menu_item (QMenu *p, const QString& item,
                                          bool checkable, QWidget*);
    void construct_window_menu (QMenuBar *p);
    void construct_help_menu (QMenuBar *p);
    void construct_documentation_menu (QMenu *p);

    void construct_news_menu (QMenuBar *p);

    void construct_tool_bar (void);

    void configure_shortcuts (void);

    QList<octave_dock_widget *> dock_widget_list (void);

    void update_default_encoding (const QString& default_encoding);

    base_qobject& m_octave_qobj;

    workspace_model *m_workspace_model;

    QHash<QMenu*, QStringList> m_hash_menu_text;

    QString m_default_encoding;

    QString m_default_style;

    //! Toolbar.

    QStatusBar *m_status_bar;

    //! Dock widgets.
    //!@{
    terminal_dock_widget *m_command_window;
    history_dock_widget *m_history_window;
    files_dock_widget *m_file_browser_window;
    documentation_dock_widget *m_doc_browser_window;
    file_editor_interface *m_editor_window;
    workspace_view *m_workspace_window;
    variable_editor *m_variable_editor_window;
    //!@}

    external_editor_interface *m_external_editor;
    QWidget *m_active_editor;

    octave_dock_widget *m_previous_dock;
    octave_dock_widget *m_active_dock;

    QString m_release_notes_icon;

    QToolBar *m_main_tool_bar;

    QMenu *m_debug_menu;

    QAction *m_debug_continue;
    QAction *m_debug_step_into;
    QAction *m_debug_step_over;
    QAction *m_debug_step_out;
    QAction *m_debug_quit;

    QAction *m_new_script_action;
    QAction *m_new_function_action;
    QAction *m_open_action;
    QAction *m_new_figure_action;
    QAction *m_load_workspace_action;
    QAction *m_save_workspace_action;
    QAction *m_set_path_action;
    QAction *m_preferences_action;
    QAction *m_exit_action;

    QAction *m_copy_action;
    QAction *m_paste_action;
    QAction *m_clear_clipboard_action;
    QAction *m_undo_action;
    QAction *m_clear_command_window_action;
    QAction *m_clear_command_history_action;
    QAction *m_clear_workspace_action;
    QAction *m_find_files_action;
    QAction *m_select_all_action;

    QAction *m_show_command_window_action;
    QAction *m_show_history_action;
    QAction *m_show_workspace_action;
    QAction *m_show_file_browser_action;
    QAction *m_show_editor_action;
    QAction *m_show_documentation_action;
    QAction *m_show_variable_editor_action;
    QAction *m_command_window_action;
    QAction *m_history_action;
    QAction *m_workspace_action;
    QAction *m_file_browser_action;
    QAction *m_editor_action;
    QAction *m_documentation_action;
    QAction *m_variable_editor_action;
    QAction *m_previous_dock_action;
    QAction *m_reset_windows_action;

    QAction *m_ondisk_doc_action;
    QAction *m_online_doc_action;
    QAction *m_report_bug_action;
    QAction *m_octave_packages_action;
    QAction *m_contribute_action;
    QAction *m_developer_action;
    QAction *m_about_octave_action;

    QAction *m_release_notes_action;
    QAction *m_current_news_action;

    //! For Toolbars.
    //!@{
    QComboBox *m_current_directory_combo_box;
    static const int current_directory_max_visible = 16;
    static const int current_directory_max_count = 16;
    QLineEdit *m_current_directory_line_edit;
    //!@}

    //! Settings dialog as guarded pointer (set to 0 when deleted).

    QPointer<settings_dialog> m_settings_dlg;

    //! Find files dialog.

    find_files_dialog *m_find_files_dlg;

    //! Set path dialog
    QPointer<set_path_dialog> m_set_path_dlg;

    //! Release notes window.

    QWidget *m_release_notes_window;

    QWidget *m_community_news_window;

    QClipboard *m_clipboard;

    //! Some class global flags.
    //!@{
    bool m_prevent_readline_conflicts;
    bool m_suppress_dbg_location;
    bool m_editor_has_tabs;

    //! Flag for closing the whole application.

    bool m_closing;
    //!@}

    QString m_file_encoding;
  };
}

#endif
