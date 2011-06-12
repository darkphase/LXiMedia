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

#include "ssandboxclient.h"
#include "lxiserverprivate.h"
#include <QtNetwork>

namespace LXiServer {

struct SSandboxClient::Data
{
  struct Request
  {
    inline Request(const QByteArray &message, QObject *receiver, const char *slot)
      : message(message), receiver(receiver), slot(slot)
    {
    }

    QByteArray                  message;
    QObject                   * receiver;
    const char                * slot;
  };

  QString                       application;
  SSandboxServer              * localServer;
  Priority                      priority;
  QString                       modeText;

  QHostAddress                  address;
  quint16                       port;

  SandboxProcess              * serverProcess;
  bool                          processStarted;
  QList<Request>                requests;
};

SSandboxClient::SSandboxClient(const QString &application, Priority priority, QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->application = application;
  d->localServer = NULL;
  d->priority = priority;

  switch(priority)
  {
  case Priority_Low:    d->modeText = "lowprio";    break;
  case Priority_Normal: d->modeText = "normalprio"; break;
  case Priority_High:   d->modeText = "highprio";   break;
  }

  d->serverProcess = NULL;
  d->processStarted = false;
}

SSandboxClient::SSandboxClient(SSandboxServer *localServer, Priority priority, QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->localServer = localServer;
  d->priority = priority;
  d->serverProcess = NULL;
  d->address = localServer->address();
  d->port = localServer->port();
  d->processStarted = true;
}

SSandboxClient::~SSandboxClient()
{
  delete d->serverProcess;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SSandboxClient::Priority SSandboxClient::priority(void) const
{
  return d->priority;
}

void SSandboxClient::ensureStarted(void)
{
  openRequest();
}

void SSandboxClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot)
{
  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  d->requests.append(Data::Request(message, receiver, slot));
  openRequest();
}

void SSandboxClient::socketDestroyed(void)
{
  SHttpClientEngine::socketDestroyed();

  openRequest();
}

void SSandboxClient::processStarted(const QHostAddress &address, quint16 port)
{
  d->address = address;
  d->port = port;
  d->processStarted = true;

  openRequest();
}

void SSandboxClient::openRequest(void)
{
  if ((d->processStarted == false) && (d->serverProcess == NULL) && !d->application.isEmpty())
  {
    d->serverProcess = new SandboxProcess(this, d->application + " " + d->modeText);

    connect(d->serverProcess, SIGNAL(ready(QHostAddress, quint16)), SLOT(processStarted(QHostAddress, quint16)));
    connect(d->serverProcess, SIGNAL(stop()), SLOT(stop()));
    connect(d->serverProcess, SIGNAL(finished(QProcess::ExitStatus)), SLOT(finished(QProcess::ExitStatus)));
    connect(d->serverProcess, SIGNAL(consoleLine(QString)), SIGNAL(consoleLine(QString)));
  }
  else while (d->processStarted && !d->requests.isEmpty() && (socketsAvailable() > 0))
  {
    const Data::Request request = d->requests.takeFirst();
    HttpSocketRequest * const socketRequest = new HttpSocketRequest(this, createSocket(), d->address, d->port, request.message);

    if (request.receiver)
      connect(socketRequest, SIGNAL(connected(QAbstractSocket *)), request.receiver, request.slot);
  }
}

void SSandboxClient::stop(void)
{
  if (d->serverProcess)
  {
    disconnect(d->serverProcess, SIGNAL(ready(QHostAddress, quint16)), this, SLOT(processStarted(QHostAddress, quint16)));
    disconnect(d->serverProcess, SIGNAL(stop()), this, SLOT(stop()));
    disconnect(d->serverProcess, SIGNAL(finished(QProcess::ExitStatus)), this, SLOT(finished(QProcess::ExitStatus)));
    disconnect(d->serverProcess, SIGNAL(consoleLine(QString)), this, SIGNAL(consoleLine(QString)));

    QTimer::singleShot(250, d->serverProcess, SLOT(kill()));
    QTimer::singleShot(1000, d->serverProcess, SLOT(deleteLater()));

    d->serverProcess = NULL;
  }

  d->processStarted = false;
}

void SSandboxClient::finished(QProcess::ExitStatus status)
{
  if (d->serverProcess)
  {
    if (status == QProcess::CrashExit)
      qWarning() << "Sandbox process terminated unexpectedly.";

    d->serverProcess->deleteLater();
    d->serverProcess = NULL;
  }

  d->processStarted = false;
}

} // End of namespace
