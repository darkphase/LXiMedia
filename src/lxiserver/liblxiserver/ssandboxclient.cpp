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
  QString                       name;
  QString                       mode;
  LogFunc                       logFunc;

  SandboxProcess              * serverProcess;
  QList<SandboxHeaderRequest *> requests;
};

QString & SSandboxClient::sandboxApplication(void)
{
  static QString a;

  return a;
}

SSandboxClient::SSandboxClient(Mode mode, QObject *parent)
  : SHttpClientEngine(parent),
    p(new Private())
{
  p->name = QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-", ".") + ".lxisandbox";

  switch(mode)
  {
  case Mode_Normal: p->mode = "normal"; break;
  case Mode_Nice:   p->mode = "nice";   break;
  }

  p->logFunc = NULL;

  p->serverProcess = NULL;
}

SSandboxClient::~SSandboxClient()
{
  delete p->serverProcess;
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSandboxClient::setLogFunc(LogFunc logFunc)
{
  p->logFunc = logFunc;
}

const QString & SSandboxClient::serverName(void) const
{
  return p->name;
}

void SSandboxClient::openRequest(const SHttpEngine::RequestHeader &header, QObject *receiver, const char *slot)
{
  if (header.host() != p->name)
    qFatal("SSandboxClient::openRequest() header.host() should be equal to "
           "SSandboxClient::serverName().");

  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  SandboxHeaderRequest * const headerRequest = new SandboxHeaderRequest(header);
  connect(headerRequest, SIGNAL(headerSent(QIODevice *)), receiver, slot);
  p->requests.append(headerRequest);

  if (p->serverProcess == NULL)
  {
    stop();
    p->serverProcess = new SandboxProcess(this, sandboxApplication() + " " + p->name + " " + p->mode);

    connect(p->serverProcess, SIGNAL(ready()), SLOT(openSockets()));
    connect(p->serverProcess, SIGNAL(stop()), SLOT(stop()));
    connect(p->serverProcess, SIGNAL(finished()), SLOT(finished()));
    connect(p->serverProcess, SIGNAL(consoleLine(QString)), SLOT(consoleLine(QString)));
  }
  else if (p->requests.count() == 1)
    openSockets();
}

void SSandboxClient::closeRequest(QIODevice *socket, bool canReuse)
{
  new SandboxSocketRequest(socket);
}

void SSandboxClient::openSockets(void)
{
  while (!p->requests.isEmpty())
    connect(new SandboxSocketRequest(this), SIGNAL(connected(QIODevice *)), p->requests.takeFirst(), SLOT(connected(QIODevice *)));
}

void SSandboxClient::stop(void)
{
  if (p->serverProcess)
  {
    disconnect(p->serverProcess, SIGNAL(ready()), this, SLOT(openSockets()));
    disconnect(p->serverProcess, SIGNAL(stop()), this, SLOT(stop()));
    disconnect(p->serverProcess, SIGNAL(finished()), this, SLOT(finished()));
    disconnect(p->serverProcess, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));

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

void SSandboxClient::consoleLine(const QString &line)
{
  if (p->logFunc)
    p->logFunc(line);
}

} // End of namespace
