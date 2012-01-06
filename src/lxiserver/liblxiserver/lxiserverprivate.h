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

#ifndef LXISERVERPRIVATE_H
#define LXISERVERPRIVATE_H

#include <QtCore>
#include <QtNetwork>
#include "shttpengine.h"
#include "shttpclient.h"
#include "ssandboxclient.h"

// Qt uses names pipes for local sockets under Windows, which are not suitable
// for using HTTP and therefore normal TCP sockets need to be used.
#ifndef Q_OS_WIN
#define SANDBOX_USE_LOCALSERVER
#endif

using namespace LXiServer;

class HttpClientRequest : public QObject
{
Q_OBJECT
private:
  class HandleResponseEvent : public QEvent
  {
  public:
    inline HandleResponseEvent(const SHttpEngine::ResponseMessage &response)
      : QEvent(handleResponseEventType), response(response)
    {
    }

    SHttpEngine::ResponseMessage response;
  };

public:
  explicit                      HttpClientRequest(SHttpClientEngine *, bool reuse);
  virtual                       ~HttpClientRequest();

public slots:
  void                          start(QIODevice *);

signals:
  void                          response(const SHttpEngine::ResponseMessage &);

protected:
  virtual void                  customEvent(QEvent *);

private slots:
  void                          readyRead();
  void                          close();

private:
  static const QEvent::Type     handleResponseEventType;

  SHttpClientEngine     * const parent;
  const bool                    reuse;
  const SHttpEngine::RequestMessage message;
  QPointer<QIODevice>           socket;
  QByteArray                    data;
  QTimer                        closeTimer;
  bool                          responded;
};

class HttpServerRequest : public QObject
{
Q_OBJECT
private:
  class HandleRequestEvent : public QEvent
  {
  public:
    inline HandleRequestEvent(const SHttpEngine::RequestMessage &request)
      : QEvent(handleRequestEventType), request(request)
    {
    }

    SHttpEngine::RequestMessage request;
  };

public:
  explicit                      HttpServerRequest(SHttpServerEngine *, quint16 serverPort);
  virtual                       ~HttpServerRequest();

public slots:
  void                          start(QIODevice *);

protected:
  virtual void                  customEvent(QEvent *);

private slots:
  void                          readyRead();
  void                          close();

private:
  static const QEvent::Type     handleRequestEventType;

  const QPointer<SHttpServerEngine> parent;
  const quint16                 serverPort;
  QPointer<QIODevice>           socket;
  QByteArray                    data;
  bool                          headerReceived;
  QByteArray                    content;
  QTimer                        closeTimer;
};

class HttpSocketRequest : public QObject
{
Q_OBJECT
public:
                                HttpSocketRequest(SHttpClientEngine *, QAbstractSocket *, quint16 port, const QByteArray &message = QByteArray());
                                HttpSocketRequest(SHttpClientEngine *, QLocalSocket *, const QByteArray &message = QByteArray());
  virtual                       ~HttpSocketRequest();

signals:
  void                          connected(QIODevice *, SHttpEngine *);

private slots:
  void                          connectToHost(const QHostInfo &);
  void                          connected(void);
  void                          bytesWritten(void);
  void                          failed(void);

private:
  static const int              maxTTL = 15000;
  SHttpClientEngine     * const parent;
  const quint16                 port;
  QByteArray                    message;
  QPointer<QIODevice>           socket;
  QTimer                        failTimer;
};

class SandboxProcess : public QObject
{
Q_OBJECT
public:
  explicit                      SandboxProcess(SSandboxClient *, const QString &);
  virtual                       ~SandboxProcess();

  bool                          waitForStarted(int msecs = 30000);
  bool                          waitForReadyRead(int msecs = 30000);
  bool                          isRunning(void) const;

public slots:
  void                          kill(void);

signals:
  void                          ready(const QString &);
  void                          stop(void);
  void                          finished(QProcess::ExitStatus);
  void                          consoleLine(const QString &);

private slots:
  void                          finished(int, QProcess::ExitStatus);
  void                          readyRead();

private:
  const QPointer<SSandboxClient> parent;
  QProcess              * const process;
  bool                          started;
};

#endif
