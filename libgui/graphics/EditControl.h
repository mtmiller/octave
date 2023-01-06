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

#if ! defined (octave_EditControl_h)
#define octave_EditControl_h 1

#include "BaseControl.h"

class QLineEdit;
class QWidget;

OCTAVE_BEGIN_NAMESPACE(octave)

class base_qobject;
class interpreter;

class TextEdit;

class EditControl : public BaseControl
{
  Q_OBJECT

public:
  EditControl (octave::base_qobject& oct_qobj, octave::interpreter& interp,
               const graphics_object& go, QLineEdit *edit);

  EditControl (octave::base_qobject& oct_qobj, octave::interpreter& interp,
               const graphics_object& go, TextEdit *edit);

  ~EditControl (void);

  static EditControl *
  create (octave::base_qobject& oct_qobj, octave::interpreter& interp,
          const graphics_object& go);

protected:
  void update (int pId);

private:
  void init (QLineEdit *edit, bool callBase = false);
  void init (TextEdit *edit, bool callBase = false);
  void initCommon (QWidget *widget);
  bool updateSingleLine (int pId);
  bool updateMultiLine (int pId);

private slots:
  void textChanged (void);
  void editingFinished (void);
  void returnPressed (void);

private:
  bool m_multiLine;
  bool m_textChanged;
};

OCTAVE_END_NAMESPACE(octave)

#endif
