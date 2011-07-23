/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef LXMEDIACENTER_BACKENDSANDBOX_H
#define LXMEDIACENTER_BACKENDSANDBOX_H

#include <QtCore>
#include <LXiServer>
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC BackendSandbox : public QObject,
                                             public SSandboxServer::Callback
{
Q_OBJECT
S_FACTORIZABLE(BackendSandbox)
public:
  /*! Creates all registred BackendSandboxes.
      \param parent   The parent object, or NULL if none.
   */
  static QList<BackendSandbox *> create(QObject *parent);

public:
  explicit                      BackendSandbox(QObject * = NULL);
  virtual                       ~BackendSandbox();

  virtual void                  initialize(SSandboxServer *);
  virtual void                  close(void);
};

} // End of namespace

#endif