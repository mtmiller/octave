////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011-2021 The Octave Project Developers
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

#include <QDesktopWidget>

#include "gui-preferences-cs.h"
#include "gui-preferences-global.h"
#include "octave-qobject.h"
#include "terminal-dock-widget.h"

#include "quit.h"
#include "signal-wrappers.h"

#include "sighandlers.h"

namespace octave
{
  terminal_dock_widget::terminal_dock_widget (QWidget *p,
                                              base_qobject& oct_qobj)
    : octave_dock_widget ("TerminalDockWidget", p, oct_qobj),
      m_terminal (QTerminal::create (oct_qobj, this, p))
  {
    m_terminal->setObjectName ("OctaveTerminal");
    m_terminal->setFocusPolicy (Qt::StrongFocus);

    setWindowIcon (QIcon (":/actions/icons/logo.png"));
    set_title (tr ("Command Window"));

    setWidget (m_terminal);
    setFocusProxy (m_terminal);

    connect (m_terminal, SIGNAL (interrupt_signal (void)),
             &oct_qobj, SLOT (interpreter_interrupt (void)));

    // Connect the visibility signal to the terminal for dis-/enabling timers
    connect (this, SIGNAL (visibilityChanged (bool)),
             m_terminal, SLOT (handle_visibility_changed (bool)));

    // Chose a reasonable size at startup in order to avoid truncated
    // startup messages
    resource_manager& rmgr = m_octave_qobj.get_resource_manager ();
    gui_settings *settings = rmgr.get_settings ();

    QFont font = QFont ();
    font.setStyleHint (QFont::TypeWriter);
    QString default_font = settings->value (global_mono_font).toString ();
    font.setFamily
      (settings->value (cs_font.key, default_font).toString ());
    font.setPointSize
      (settings->value (cs_font_size).toInt ());

    QFontMetrics metrics(font);

    int win_x =  metrics.maxWidth()*80;
    int win_y =  metrics.height()*25;

    int max_x = QApplication::desktop ()->screenGeometry (this).width ();
    int max_y = QApplication::desktop ()->screenGeometry (this).height ();

    if (win_x > max_x)
      win_x = max_x;
    if (win_y > max_y)
      win_y = max_y;

    setGeometry (0, 0, win_x, win_y);
  }

  terminal_dock_widget::~terminal_dock_widget (void) { }

  bool terminal_dock_widget::has_focus (void) const
  {
    QWidget *w = widget ();

    return w->hasFocus ();
  }
}
