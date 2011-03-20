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

#ifndef LXISERVER_SANDBOXCLIENT_H
#define LXISERVER_SANDBOXCLIENT_H

#include <QtCore>
#include "httpengine.h"

namespace LXiServer {

class SandboxClient : public HttpClientEngine
{
Q_OBJECT
public:
  enum Mode
  {
    Mode_Normal               = 0,
    Mode_Nice
  };

private:
  class StartServerEvent : public QEvent
  {
  public:
    inline StartServerEvent(QSemaphore *sem, int timeout)
      : QEvent(startServerEventType), sem(sem), timeout(timeout)
    {
    }

    QSemaphore          * const sem;
    int                         timeout;
  };

public:
  explicit                      SandboxClient(Mode, QObject * = NULL);
  virtual                       ~SandboxClient();

  inline ResponseMessage        sendRequest(const RequestMessage &request, int timeout = maxTTL) { return HttpClientEngine::sendRequest(request, timeout); }
  QByteArray                    sendRequest(const QByteArray &, int timeout = maxTTL);

signals:
  void                          consoleLine(const QString &);

protected:
  virtual void                  customEvent(QEvent *);

protected: // From HttpClientEngine
  virtual QIODevice           * openSocket(const QString &host, int timeout);
  virtual void                  closeSocket(QIODevice *, bool canReuse, int timeout);

private slots:
  void                          startServer(int timeout);
  void                          readConsole(void);
  void                          serverFinished(int, QProcess::ExitStatus);

private:
  static const QEvent::Type     startServerEventType;

  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
