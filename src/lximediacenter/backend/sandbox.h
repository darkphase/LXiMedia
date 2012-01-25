/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#ifndef SANDBOX_H
#define SANDBOX_H

#include <QtCore>
#include <LXiMediaCenter>

class Sandbox : public QObject,
                public SSandboxServer::Callback
{
Q_OBJECT
public:
                                Sandbox(void);
  virtual                       ~Sandbox();

  inline SSandboxServer       * server(void)                                    { return &sandboxServer; }

  void                          start(const QString &mode);

public: // From SSandboxServer::Callback
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);

private:
  SSandboxServer                sandboxServer;
  QString                       mode;

  QList<BackendSandbox *>       backendSandboxes;

  QTimer                        stopTimer;
};

#endif
