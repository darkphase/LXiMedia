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

#ifndef LXISERVER_SSANDBOXCLIENT_H
#define LXISERVER_SSANDBOXCLIENT_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpengine.h"
#include "export.h"

namespace LXiServer {

class SSandboxServer;

class LXISERVER_PUBLIC SSandboxClient : public SHttpClientEngine
{
Q_OBJECT
public:
  enum Priority
  {
    Priority_Low              = -1,
    Priority_Normal           = 0,
    Priority_High             = 1
  };

public:
                                SSandboxClient(const QString &application, Priority, QObject * = NULL);
                                SSandboxClient(SSandboxServer *, Priority, QObject * = NULL);
  virtual                       ~SSandboxClient();

  Priority                      priority(void) const;

  void                          ensureStarted(void);

public: // From HttpClientEngine
  virtual void                  openRequest(const RequestMessage &header, QObject *receiver, const char *slot);
  
  virtual ResponseMessage       blockingRequest(const RequestMessage &, int timeout = 30000);

signals:
  /*! This signal is emitted when a line is written to stderr that starts with
      '#'. Other lines written to stderr are routed to
      SApplication::logLine().
   */
  void                          consoleLine(const QString &);

protected:
  virtual void                  socketDestroyed(void);

private slots:
  void                          startProcess(void);
  void                          processStarted(const QString &);
  void                          openRequest(void);
  void                          stop(void);
  void                          finished(QProcess::ExitStatus);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
