/*

Copyright (C) 2012-2019 Michael Goffioul.
Copyright (C) 2012-2019 Jacob Dawid.

This file is part of QTerminal.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not,
see <https://www.gnu.org/licenses/>.

*/

#include "gui-preferences-cs.h"
#include "gui-preferences-sc.h"
#include "gui-preferences-global.h"

#include "QTerminal.h"
#if defined (Q_OS_WIN32)
# include "win32/QWinTerminalImpl.h"
#else
# include "unix/QUnixTerminalImpl.h"
#endif

QTerminal *
QTerminal::create (QWidget *xparent)
{
#if defined (Q_OS_WIN32)
  return new QWinTerminalImpl (xparent);
#else
  return new QUnixTerminalImpl (xparent);
#endif
}

// slot for disabling the interrupt action when terminal loses focus
void
QTerminal::set_global_shortcuts (bool focus_out)
  {
    if (focus_out)
      {
        _interrupt_action->setShortcut (QKeySequence ());
        _nop_action->setShortcut (QKeySequence ());
      }
    else
      {
        _interrupt_action->setShortcut
          (QKeySequence (Qt::ControlModifier | Qt::Key_C));

        _nop_action->setShortcut
          (QKeySequence (Qt::ControlModifier | Qt::Key_D));
      }
  }

// slot for the terminal's context menu
void
QTerminal::handleCustomContextMenuRequested (const QPoint& at)
  {
    QClipboard * cb = QApplication::clipboard ();
    QString selected_text = selectedText();
    bool has_selected_text = ! selected_text.isEmpty ();

    _edit_action->setVisible (false);
    m_edit_selected_action->setVisible (false);
    m_help_selected_action->setVisible (false);
    m_doc_selected_action->setVisible (false);

#if defined (Q_OS_WIN32)
    // include this when in windows because there is no filter for
    // detecting links and error messages yet
    if (has_selected_text)
      {
        QRegExp file ("(?:[ \\t]+)(\\S+) at line (\\d+) column (?:\\d+)");

        int pos = file.indexIn (selected_text);

        if (pos > -1)
          {
            QString file_name = file.cap (1);
            QString line = file.cap (2);

            _edit_action->setVisible (true);
            _edit_action->setText (tr ("Edit %1 at line %2")
                                   .arg (file_name).arg (line));

            QStringList data;
            data << file_name << line;
            _edit_action->setData (data);
          }
      }
#endif

    if (has_selected_text)
      {
        QRegExp expr (".*\b*(\\w+)\b*.*");

        int pos = expr.indexIn (selected_text);

        if (pos > -1)
          {
            QString expr_found = expr.cap (1);

            m_edit_selected_action->setVisible (true);
            m_edit_selected_action->setText (tr ("Edit %1").arg (expr_found));
            m_edit_selected_action->setData (expr_found);

            m_help_selected_action->setVisible (true);
            m_help_selected_action->setText (tr ("Help on %1").arg (expr_found));
            m_help_selected_action->setData (expr_found);

            m_doc_selected_action->setVisible (true);
            m_doc_selected_action->setText (tr ("Documentation on %1")
                                            .arg (expr_found));
            m_doc_selected_action->setData (expr_found);
          }
      }

    _paste_action->setEnabled (cb->text().length() > 0);
    _copy_action->setEnabled (has_selected_text);
    _run_selection_action->setVisible (has_selected_text);

    // Get the actions of any hotspots the filters may have found
    QList<QAction*> actions = get_hotspot_actions (at);
    if (actions.length ())
      _contextMenu->addSeparator ();
    for (int i = 0; i < actions.length (); i++)
      _contextMenu->addAction (actions.at(i));

    // Finally, show the context menu
    _contextMenu->exec (mapToGlobal (at));

    // Cleaning up, remove actions of the hotspot
    for (int i = 0; i < actions.length (); i++)
      _contextMenu->removeAction (actions.at(i));
  }

// slot for running the selected code
void
QTerminal::run_selection ()
{
  QStringList commands = selectedText ().split (QRegExp ("[\r\n]"),
                                                QString::SkipEmptyParts);
  for (int i = 0; i < commands.size (); i++)
    emit execute_command_in_terminal_signal (commands.at (i));

}

// slot for edit files in error messages
void
QTerminal::edit_file ()
{
  QString file = _edit_action->data ().toStringList ().at (0);
  int line = _edit_action->data ().toStringList ().at (1).toInt ();

  emit edit_mfile_request (file,line);
}

// slot for edit selected function names
void QTerminal::edit_selected ()
{
  QString file = m_edit_selected_action->data ().toString ();

  emit edit_mfile_request (file,0);
}

// slot for showing help on selected epxression
void QTerminal::help_on_expression ()
{
  QString expr = m_help_selected_action->data ().toString ();

  emit execute_command_in_terminal_signal ("help " + expr);
}

// slot for showing documentation on selected epxression
void QTerminal::doc_on_expression ()
{
  QString expr = m_doc_selected_action->data ().toString ();

  emit show_doc_signal (expr);
}

void
QTerminal::notice_settings (const QSettings *settings)
{
  // QSettings pointer is checked before emitting.

  // Set terminal font:
  QFont term_font = QFont ();
  term_font.setStyleHint (QFont::TypeWriter);
  QString default_font = settings->value (global_mono_font.key, global_mono_font.def).toString ();
  term_font.setFamily
    (settings->value (cs_font.key, default_font).toString ());
  term_font.setPointSize
    (settings->value (cs_font_size.key, cs_font_size.def).toInt ());
  setTerminalFont (term_font);

  QFontMetrics metrics (term_font);
  setMinimumSize (metrics.maxWidth ()*16, metrics.height ()*3);

  QString cursor_type
    = settings->value (cs_cursor.key, cs_cursor.def).toString ();

  bool cursor_blinking;
  if (settings->contains (global_cursor_blinking.key))
    cursor_blinking = settings->value (global_cursor_blinking.key,
                                       global_cursor_blinking.def).toBool ();
  else
    cursor_blinking = settings->value (cs_cursor_blinking.key,
                                       cs_cursor_blinking.def).toBool ();

  for (int ct = IBeamCursor; ct <= UnderlineCursor; ct++)
    {
      if (cursor_type.toStdString () == cs_cursor_types[ct])
        {
          setCursorType ((CursorType) ct, cursor_blinking);
          break;
        }
    }

  bool cursorUseForegroundColor
    = settings->value (cs_cursor_use_fgcol.key, cs_cursor_use_fgcol.def).toBool ();

  setForegroundColor
    (settings->value (cs_colors[0].key, cs_colors[0].def).value<QColor> ());

  setBackgroundColor
    (settings->value (cs_colors[1].key, cs_colors[1].def).value<QColor> ());

  setSelectionColor
    (settings->value (cs_colors[2].key, cs_colors[2].def).value<QColor> ());

  setCursorColor (cursorUseForegroundColor,
     settings->value (cs_colors[3].key, cs_colors[3].def).value<QColor> ());

  setScrollBufferSize (settings->value (cs_hist_buffer.key,
                                        cs_hist_buffer.def).toInt ());

  // check whether Copy shortcut is Ctrl-C
  QKeySequence sc;
  sc = QKeySequence (settings->value (sc_main_edit_copy.key,
                                      sc_main_edit_copy.def).toString ());

  // if sc is empty, shortcuts are not yet in the settings (take the default)
  if (sc.isEmpty ())         // QKeySequence::Copy as second argument in
    sc = QKeySequence::Copy; // settings->value () does not work!

  //  dis- or enable extra interrupt action
  bool extra_ir_action = (sc != QKeySequence (Qt::ControlModifier | Qt::Key_C));
  _interrupt_action->setEnabled (extra_ir_action);
  has_extra_interrupt (extra_ir_action);

  // check whether shortcut Ctrl-D is in use by the main-window
  bool ctrld = settings->value (sc_main_ctrld.key, sc_main_ctrld.def).toBool ();
  _nop_action->setEnabled (! ctrld);
}
