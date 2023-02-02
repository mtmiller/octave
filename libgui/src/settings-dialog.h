////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011-2023 The Octave Project Developers
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

#if ! defined (octave_settings_dialog_h)
#define octave_settings_dialog_h 1

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>

#include "color-picker.h"
#include "gui-preferences-ed.h"
#include "ui-settings-dialog.h"

class QsciLexer;

OCTAVE_BEGIN_NAMESPACE(octave)

// Ui::settings_dialog is a generated class.

class settings_dialog : public QDialog, private Ui::settings_dialog
{
  Q_OBJECT

public:

  explicit settings_dialog (QWidget *parent,
                            const QString& desired_tab = QString ());

  ~settings_dialog () = default;

  void show_tab (const QString&);

signals:

  void apply_new_settings ();

private slots:

  void get_octave_dir ();
  void get_file_browser_dir ();
  void get_dir (QLineEdit *, const QString&);
  void set_disabled_pref_file_browser_dir (bool disable);
  void proxy_items_update ();

  // slots updating colors depending on theme
  void update_terminal_colors (int def = 0);
  void update_workspace_colors (int def = 0);
  void update_varedit_colors (int def = 0);
  void update_editor_lexers (int def = 0);

  // slots for dialog's buttons
  void button_clicked (QAbstractButton *button);

  // slots for import/export-buttons of shortcut sets
  void import_shortcut_set ();
  void export_shortcut_set ();
  void default_shortcut_set ();

private:

  enum import_export_action
  {
    OSC_IMPORT,
    OSC_EXPORT
  };

#if defined (HAVE_QSCINTILLA)
  void update_lexer (QsciLexer *lexer, int mode, int def = 0);
  void get_lexer_settings (QsciLexer *lexer);
  void write_lexer_settings (QsciLexer *lexer);
#endif

  void write_changed_settings ();

  void read_workspace_colors ();
  void write_workspace_colors ();

  void read_terminal_colors ();
  void write_terminal_colors ();

  void read_varedit_colors ();
  void write_varedit_colors ();

  QString get_shortcuts_file_name (import_export_action action);

  bool overwrite_all_shortcuts ();

  color_picker *m_widget_title_bg_color;
  color_picker *m_widget_title_bg_color_active;
  color_picker *m_widget_title_fg_color;
  color_picker *m_widget_title_fg_color_active;

  QRadioButton *m_rb_comment_strings[ed_comment_strings_count];
  QCheckBox *m_rb_uncomment_strings[ed_comment_strings_count];

  QCheckBox *m_ws_enable_colors;
  QCheckBox *m_ws_hide_tool_tips;
};

OCTAVE_END_NAMESPACE(octave)

#endif
