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

#if ! defined (octave_GenericEventNotify_h)
#define octave_GenericEventNotify_h 1

#include <QSet>

class QEvent;
class QObject;
class QWidget;

OCTAVE_BEGIN_NAMESPACE(octave)

class GenericEventNotifyReceiver;

class GenericEventNotifySender
{
public:
  GenericEventNotifySender (void) : m_receivers () { }
  virtual ~GenericEventNotifySender (void) = default;

  void addReceiver (GenericEventNotifyReceiver *r)
  { m_receivers.insert (r); }

  void removeReceiver (GenericEventNotifyReceiver *r)
  { m_receivers.remove (r); }

protected:
  bool notifyReceiversBefore (QObject *obj, QEvent *evt);
  void notifyReceiversAfter (QObject *obj, QEvent *evt);

private:
  QSet<GenericEventNotifyReceiver *> m_receivers;
};

class GenericEventNotifyReceiver
{
public:
  GenericEventNotifyReceiver (void) { }
  virtual ~GenericEventNotifyReceiver (void) = default;

  virtual bool eventNotifyBefore (QObject *obj, QEvent *evt) = 0;
  virtual void eventNotifyAfter (QObject *obj, QEvent *evt) = 0;
};

inline
bool GenericEventNotifySender::notifyReceiversBefore (QObject *obj,
                                                      QEvent *evt)
{
  for (auto *r : m_receivers)
    if (r->eventNotifyBefore (obj, evt))
      return true;

  return false;
}

inline
void GenericEventNotifySender::notifyReceiversAfter (QObject *obj,
                                                     QEvent *evt)
{
  for (auto *r : m_receivers)
    r->eventNotifyAfter (obj, evt);
}

OCTAVE_END_NAMESPACE(octave)

#define DECLARE_GENERICEVENTNOTIFY_SENDER(T,B) \
class T : public B, public GenericEventNotifySender \
{ \
public: \
  T (QWidget *xparent) : B (xparent), GenericEventNotifySender () { } \
  ~ T (void) = default; \
\
  bool event (QEvent *evt) \
    { \
      bool result = true; \
      if (! notifyReceiversBefore (this, evt)) \
        result = B::event (evt); \
      notifyReceiversAfter (this, evt); \
      return result; \
    } \
}

#endif
