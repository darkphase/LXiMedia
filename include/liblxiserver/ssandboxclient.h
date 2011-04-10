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

class LXISERVER_PUBLIC SSandboxClient : public SHttpClientEngine
{
Q_OBJECT
public:
  enum Mode
  {
    Mode_Normal               = 0,
    Mode_Nice
  };

public:
                                SSandboxClient(const QString &application, Mode, QObject * = NULL);
  virtual                       ~SSandboxClient();

  Mode                          mode(void) const;

public: // From HttpClientEngine
  virtual void                  openRequest(const RequestMessage &header, QObject *receiver, const char *slot);
  virtual void                  closeRequest(QAbstractSocket *, bool canReuse = false);

signals:
  /*! This signal is emitted when a line is written to stderr that starts with
      '#'. Other lines written to stderr are routed to
      SApplication::logLineToActiveLogFile().
   */
  void                          consoleLine(const QString &);

private slots:
  __internal void               processStarted(const QHostAddress &, quint16);
  __internal void               openSockets(void);
  __internal void               stop(void);
  __internal void               finished(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
