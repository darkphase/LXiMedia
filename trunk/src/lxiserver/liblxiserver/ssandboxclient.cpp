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
  Mode                          mode;
  QString                       modeText;

  QHostAddress                  address;
  quint16                       port;

  SandboxProcess              * serverProcess;
  QList<Request>                requests;
};

SSandboxClient::SSandboxClient(const QString &application, Mode mode, QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->application = application;
  d->mode = mode;

  switch(mode)
  {
  case Mode_Normal: d->modeText = "normal"; break;
  case Mode_Nice:   d->modeText = "nice";   break;
  }

  d->serverProcess = NULL;
}

SSandboxClient::~SSandboxClient()
{
  delete d->serverProcess;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SSandboxClient::Mode SSandboxClient::mode(void) const
{
  return d->mode;
}

void SSandboxClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot)
{
  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  d->requests.append(Data::Request(message, receiver, slot));

  if (d->serverProcess == NULL)
  {
    stop();
    d->serverProcess = new SandboxProcess(this, d->application + " " + d->modeText);

    connect(d->serverProcess, SIGNAL(ready(QHostAddress, quint16)), SLOT(processStarted(QHostAddress, quint16)));
    connect(d->serverProcess, SIGNAL(stop()), SLOT(stop()));
    connect(d->serverProcess, SIGNAL(finished()), SLOT(finished()));
    connect(d->serverProcess, SIGNAL(consoleLine(QString)), SIGNAL(consoleLine(QString)));
  }
  else if (d->requests.count() == 1)
    openSockets();
}

void SSandboxClient::closeRequest(QAbstractSocket *socket, bool canReuse)
{
  new SocketCloseRequest(this, socket);
}

void SSandboxClient::processStarted(const QHostAddress &address, quint16 port)
{
  qDebug() << "SSandboxClient::processStarted" << address.toString() << port;

  d->address = address;
  d->port = port;

  openSockets();
}

void SSandboxClient::openSockets(void)
{
  while (!d->requests.isEmpty())
  {
    const Data::Request request = d->requests.takeFirst();

    connect(new HttpSocketRequest(this, d->address, d->port, request.message), SIGNAL(connected(QAbstractSocket *)), request.receiver, request.slot);
  }
}

void SSandboxClient::stop(void)
{
  if (d->serverProcess)
  {
    disconnect(d->serverProcess, SIGNAL(ready(QHostAddress, quint16)), this, SLOT(processStarted(QHostAddress, quint16)));
    disconnect(d->serverProcess, SIGNAL(stop()), this, SLOT(stop()));
    disconnect(d->serverProcess, SIGNAL(finished()), this, SLOT(finished()));
    disconnect(d->serverProcess, SIGNAL(consoleLine(QString)), this, SIGNAL(consoleLine(QString)));

    QTimer::singleShot(250, d->serverProcess, SLOT(kill()));
    QTimer::singleShot(1000, d->serverProcess, SLOT(deleteLater()));

    d->serverProcess = NULL;
  }
}

void SSandboxClient::finished(void)
{
  if (d->serverProcess)
  {
    qDebug() << "Sandbox process terminated.";

    d->serverProcess->deleteLater();
    d->serverProcess = NULL;
  }
}

} // End of namespace
