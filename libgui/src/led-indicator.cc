////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2013-2020 The Octave Project Developers
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

#include <QColor>
#include "led-indicator.h"

namespace octave
{

  led_indicator::led_indicator (led_state initial_state, QWidget *p)
    : QLabel (p)
  {
    setFixedSize(12,12);
    set_state (initial_state);
  }

  void led_indicator::set_state (led_state state)
  {
    QColor col (Qt::gray);

    switch (state)
      {
        case LED_STATE_NO:
          break;

        case LED_STATE_INACTIVE:
          col = QColor (Qt::darkRed);
          break;

        case LED_STATE_ACTIVE:
          col = QColor (Qt::darkGreen);
          break;
      }

    setStyleSheet (style_sheet (col));
  }

  QString led_indicator::style_sheet (const QColor& col)
  {
    int h, s, v;

    col.getHsv (&h, &s, &v);
    s = s/4;
    v = 232;

    QColor col_light = QColor::fromHsv (h,s,v);

    const QString style = QString (
        "border-radius: %1; background-color: "
        "qlineargradient(spread:pad, x1:0.2, y1:0.2, x2:1, y2:1, stop:0 "
        "%2, stop:1 %3);"
      ).arg (width ()/2).arg (col_light.name ()).arg (col.name ());

    return style;
  }

}