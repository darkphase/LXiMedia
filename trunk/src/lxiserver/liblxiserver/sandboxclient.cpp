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

#include "sandboxclient.h"
#include <QtNetwork>

namespace LXiServer {

const QEvent::Type  SandboxClient::startServerEventType = QEvent::Type(QEvent::registerEventType());

struct SandboxClient::Private
{
  QMutex                        mutex;
  QString                       name;
  QString                       mode;

  QProcess                      serverProcess;
  bool                          serverStarted;
};

SandboxClient::SandboxClient(Mode mode, QObject *parent)
  : HttpClientEngine(parent),
    p(new Private())
{
  p->name = "sandbox_" + QUuid::createUuid().toString().replace("{", "").replace("}", "");

  switch(mode)
  {
  case Mode_Normal: p->mode = "normal"; break;
  case Mode_Nice:   p->mode = "nice";   break;
  }

  p->serverStarted = false;
}

SandboxClient::~SandboxClient()
{
  if (p->serverProcess.state() != QProcess::NotRunning)
  {
    sendRequest("/?exit", 250);
    p->serverProcess.waitForFinished(250);
    p->serverProcess.kill();
  }

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

QByteArray SandboxClient::sendRequest(const QByteArray &path, int timeout)
{
  QUrl url;
  url.setEncodedPath(path);
  url.setHost(p->name);

  return HttpClientEngine::sendRequest(url, timeout);
}

void SandboxClient::customEvent(QEvent *e)
{
  if (e->type() == startServerEventType)
  {
    StartServerEvent * const event = static_cast<StartServerEvent *>(e);

    startServer(event->timeout);
    event->sem->release(1);
  }
  else
    HttpClientEngine::customEvent(e);
}

QIODevice * SandboxClient::openSocket(const QString &host, int timeout)
{
  Q_ASSERT(host == p->name);

  QTime timer;
  timer.start();

  QMutexLocker l(&p->mutex);

  if (p->serverProcess.state() != QProcess::Running)
  {
    if (QThread::currentThread() != thread())
    {
      QSemaphore sem;
      QCoreApplication::postEvent(this, new StartServerEvent(&sem, qMax(timeout - qAbs(timer.elapsed()), 0)));
      sem.acquire(1);
    }
    else
      startServer(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  if (p->serverProcess.state() == QProcess::Running)
  {
    l.unlock();

    QLocalSocket * const socket = new QLocalSocket();
    socket->connectToServer(p->name);
    if (socket->waitForConnected(qMax(timeout - qAbs(timer.elapsed()), 0)))
      return socket;

    delete socket;
  }

  return NULL;
}

void SandboxClient::closeSocket(QIODevice *device, bool, int timeout)
{
  QLocalSocket * const socket = static_cast<QLocalSocket *>(device);

  if (socket->state() == QLocalSocket::ConnectedState)
  {
    QTime timer;
    timer.start();

    socket->waitForBytesWritten(qMax(timeout - qAbs(timer.elapsed()), 0));
    socket->disconnectFromServer();
    if (socket->state() != QLocalSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  delete socket;
}

void SandboxClient::startServer(int timeout)
{
  QTime timer;
  timer.start();

  if (p->serverProcess.state() == QProcess::NotRunning)
  {
    p->serverProcess.start(
        qApp->applicationFilePath(),
        QStringList() << "--sandbox" << p->name << p->mode);

    p->serverStarted = false;
  }

  if (p->serverProcess.state() == QProcess::Starting)
  {
    p->serverProcess.waitForStarted(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  if (!p->serverStarted)
  {
    p->serverProcess.setReadChannel(QProcess::StandardError);
    while (p->serverProcess.waitForReadyRead(qMax(timeout - qAbs(timer.elapsed()), 0)))
    while (p->serverProcess.canReadLine())
    {
      const QByteArray line = p->serverProcess.readLine();
      if (line.startsWith("##READY"))
      {
        connect(&p->serverProcess, SIGNAL(readyRead()), this, SLOT(readConsole()));
        readConsole();

        p->serverStarted = true;
        return;
      }
      else
        emit consoleLine(QString::fromUtf8(line));
    }
  }
}

void SandboxClient::readConsole(void)
{
  if (p->serverProcess.state() == QProcess::Running)
  while (p->serverProcess.canReadLine())
    emit consoleLine(QString::fromUtf8(p->serverProcess.readLine()));
}

void SandboxClient::serverFinished(int, QProcess::ExitStatus exitStatus)
{
  disconnect(&p->serverProcess, SIGNAL(readyRead()), this, SLOT(readConsole()));

  // Restart on crash
  if (exitStatus == QProcess::CrashExit)
    startServer(0);
}

} // End of namespace
