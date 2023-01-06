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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <QAbstractButton>

#include "ButtonControl.h"
#include "ButtonGroup.h"
#include "Container.h"
#include "QtHandlesUtils.h"

#include "graphics.h"
#include "interpreter.h"

OCTAVE_BEGIN_NAMESPACE(octave)

ButtonControl::ButtonControl (octave::base_qobject& oct_qobj,
                              octave::interpreter& interp,
                              const graphics_object& go,
                              QAbstractButton *btn)
: BaseControl (oct_qobj, interp, go, btn), m_blockCallback (false)
{
  uicontrol::properties& up = properties<uicontrol> ();

  QString str = Utils::fromStdString (up.get_string_string ());
  str.replace ("&", "&&");
  btn->setText (str);
  if (btn->isCheckable () || up.style_is ("togglebutton"))
    {
      btn->setCheckable (true);

      Matrix value = up.get_value ().matrix_value ();

      if (value.numel () > 0 && value(0) == up.get_max ())
        btn->setChecked (true);
    }

  connect (btn, &QAbstractButton::clicked, this, &ButtonControl::clicked);
  connect (btn, &QAbstractButton::toggled, this, &ButtonControl::toggled);
}

ButtonControl::~ButtonControl (void)
{ }

void
ButtonControl::update (int pId)
{
  uicontrol::properties& up = properties<uicontrol> ();
  QAbstractButton *btn = qWidget<QAbstractButton> ();

  switch (pId)
    {
    case uicontrol::properties::ID_STRING:
      {
        QString str = Utils::fromStdString (up.get_string_string ());
        str.replace ("&", "&&");
        btn->setText (str);
        break;
      }

    case uicontrol::properties::ID_VALUE:
      m_blockCallback = true;
      if (btn->isCheckable ())
        {
          Matrix value = up.get_value ().matrix_value ();

          if (value.numel () > 0)
            {
              double dValue = value(0);

              if (dValue != 0.0 && dValue != 1.0)
                warning ("button value not within valid display range");
              else if (dValue == up.get_min () && btn->isChecked ())
                {
                  btn->setChecked (false);
                  if (up.style_is ("radiobutton") || up.style_is ("togglebutton"))
                    {
                      gh_manager& gh_mgr = m_interpreter.get_gh_manager ();

                      Object *parent = Object::parentObject (m_interpreter, gh_mgr.get_object (up.get___myhandle__ ()));
                      ButtonGroup *btnGroup = dynamic_cast<ButtonGroup *>(parent);
                      if (btnGroup)
                        btnGroup->selectNothing ();
                    }
                }
              else if (dValue == up.get_max () && ! btn->isChecked ())
                btn->setChecked (true);
            }
        }
      m_blockCallback = false;
      break;

    default:
      BaseControl::update (pId);
      break;
    }
}

void
ButtonControl::toggled (bool checked)
{
  QAbstractButton *btn = qWidget<QAbstractButton> ();

  if (! m_blockCallback && btn->isCheckable ())
    {
      gh_manager& gh_mgr = m_interpreter.get_gh_manager ();

      octave::autolock guard (gh_mgr.graphics_lock ());

      uicontrol::properties& up = properties<uicontrol> ();

      Matrix oldValue = up.get_value ().matrix_value ();
      double newValue = (checked ? up.get_max () : up.get_min ());

      if (oldValue.numel () != 1 || (newValue != oldValue(0)))
        emit gh_set_event (m_handle, "value", newValue, false);
      emit gh_callback_event (m_handle, "callback");
    }
}

void
ButtonControl::clicked (void)
{
  QAbstractButton *btn = qWidget<QAbstractButton> ();

  if (! btn->isCheckable ())
    emit gh_callback_event (m_handle, "callback");
}

OCTAVE_END_NAMESPACE(octave);
