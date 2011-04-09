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

struct SSandboxClient::Private
{
  QString                       application;
  QString                       name;
  Mode                          mode;
  QString                       modeText;

  SandboxProcess              * serverProcess;
  QList<SandboxMessageRequest *> requests;
};

SSandboxClient::SSandboxClient(const QString &application, Mode mode, QObject *parent)
  : SHttpClientEngine(parent),
    p(new Private())
{
  p->application = application;
  p->name = QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-", ".") + ".lxisandbox";
  p->mode = mode;

  switch(mode)
  {
  case Mode_Normal: p->modeText = "normal"; break;
  case Mode_Nice:   p->modeText = "nice";   break;
  }

  p->serverProcess = NULL;
}

SSandboxClient::~SSandboxClient()
{
  delete p->serverProcess;
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

const QString & SSandboxClient::serverName(void) const
{
  return p->name;
}

SSandboxClient::Mode SSandboxClient::mode(void) const
{
  return p->mode;
}

void SSandboxClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot)
{
  if (message.host() != p->name)
    qFatal("SSandboxClient::openRequest() message.host() should be equal to "
           "SSandboxClient::serverName().");

  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  SandboxMessageRequest * const messageRequest = new SandboxMessageRequest(message);
  connect(messageRequest, SIGNAL(headerSent(QIODevice *)), receiver, slot);
  p->requests.append(messageRequest);

  if (p->serverProcess == NULL)
  {
    stop();
    p->serverProcess = new SandboxProcess(this, p->application + " " + p->name + " " + p->modeText);

    connect(p->serverProcess, SIGNAL(ready()), SLOT(openSockets()));
    connect(p->serverProcess, SIGNAL(stop()), SLOT(stop()));
    connect(p->serverProcess, SIGNAL(finished()), SLOT(finished()));
    connect(p->serverProcess, SIGNAL(consoleLine(QString)), SIGNAL(consoleLine(QString)));
  }
  else if (p->requests.count() == 1)
    openSockets();
}

void SSandboxClient::closeRequest(QIODevice *socket, bool canReuse)
{
  new SocketCloseRequest(socket);
}

void SSandboxClient::openSockets(void)
{
  while (!p->requests.isEmpty())
  {
    SandboxSocketRequest * const socketRequest = new SandboxSocketRequest(this);
    connect(socketRequest, SIGNAL(connected(QIODevice *)), SLOT(socketOpened(QIODevice *)));
    connect(socketRequest, SIGNAL(connected(QIODevice *)), p->requests.takeFirst(), SLOT(connected(QIODevice *)));
  }
}

void SSandboxClient::stop(void)
{
  if (p->serverProcess)
  {
    disconnect(p->serverProcess, SIGNAL(ready()), this, SLOT(openSockets()));
    disconnect(p->serverProcess, SIGNAL(stop()), this, SLOT(stop()));
    disconnect(p->serverProcess, SIGNAL(finished()), this, SLOT(finished()));
    disconnect(p->serverProcess, SIGNAL(consoleLine(QString)), this, SIGNAL(consoleLine(QString)));

    QTimer::singleShot(250, p->serverProcess, SLOT(kill()));
    QTimer::singleShot(1000, p->serverProcess, SLOT(deleteLater()));

    p->serverProcess = NULL;
  }
}

void SSandboxClient::finished(void)
{
  if (p->serverProcess)
  {
    qDebug() << "Sandbox process terminated.";

    p->serverProcess->deleteLater();
    p->serverProcess = NULL;

    while (!p->requests.isEmpty())
      p->requests.takeFirst()->connected(NULL);
  }
}

} // End of namespace
