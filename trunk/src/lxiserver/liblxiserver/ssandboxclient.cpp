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
#include <QtNetwork>

namespace LXiServer {

const QEvent::Type  SSandboxClient::startServerEventType = QEvent::Type(QEvent::registerEventType());

struct SSandboxClient::Private
{
  inline Private(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QString                       name;
  QString                       mode;
  LogFunc                       logFunc;

  QProcess                    * serverProcess;
  bool                          serverStarted;
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
  p->serverStarted = false;
}

SSandboxClient::~SSandboxClient()
{
  p->mutex.lock();

  if (p->serverProcess && (p->serverProcess->state() != QProcess::NotRunning))
  {
    p->serverProcess->terminate();
    if (!p->serverProcess->waitForFinished(250))
    if (p->serverProcess) // Could have been set to NULL by a slot
    {
      p->serverProcess->kill();
      p->serverProcess->waitForFinished(250);
    }
  }

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

QByteArray SSandboxClient::sendRequest(const QByteArray &path, int timeout)
{
  QUrl url;
  url.setEncodedPath(path);
  url.setHost(p->name);

  return SHttpClientEngine::sendRequest(url, timeout);
}

void SSandboxClient::customEvent(QEvent *e)
{
  if (e->type() == startServerEventType)
  {
    StartServerEvent * const event = static_cast<StartServerEvent *>(e);

    startServer(event->timeout);
    event->sem->release(1);
  }
  else
    SHttpClientEngine::customEvent(e);
}

QIODevice * SSandboxClient::openSocket(const QString &host, int maxTimeout)
{
  Q_ASSERT(host == p->name);

  const int timeout = qMin(30000, maxTimeout); // Should take no more than 30 seconds
  QTime timer;
  timer.start();

  for (unsigned i=0; i<3; i++)
  {
    QMutexLocker l(&p->mutex);

    if ((p->serverProcess == NULL) || (p->serverProcess->state() != QProcess::Running))
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

    if (p->serverProcess && (p->serverProcess->state() == QProcess::Running))
    {
      l.unlock();

      QLocalSocket * const socket = new QLocalSocket();
      socket->setReadBufferSize(262144);
      socket->connectToServer(p->name);
      if (socket->waitForConnected(qMax(timeout - qAbs(timer.elapsed()), 0)))
        return socket;

      qDebug() << "Failed to connect to sandbox server.";
      delete socket;
    }
  }

  return NULL;
}

void SSandboxClient::closeSocket(QIODevice *device, bool, int timeout)
{
  QLocalSocket * const socket = static_cast<QLocalSocket *>(device);

  if (socket->state() == QLocalSocket::ConnectedState)
  {
    QTime timer;
    timer.start();

    while (socket->bytesToWrite() > 0)
    if (!socket->waitForBytesWritten(qMax(timeout - qAbs(timer.elapsed()), 0)))
      break;

    socket->disconnectFromServer();
    if (socket->state() != QLocalSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  delete socket;
}

void SSandboxClient::startServer(int timeout)
{
  Q_ASSERT(QThread::currentThread() == thread());

  QTime timer;
  timer.start();

  if (p->serverProcess == NULL)
  {
    p->serverProcess = new QProcess(this);

    connect(p->serverProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(serverFinished(int, QProcess::ExitStatus)));
  }

  if (p->serverProcess->state() == QProcess::NotRunning)
  {
    p->serverProcess->start(sandboxApplication() + " " + p->name + " " + p->mode);
    p->serverStarted = false;
  }

  if (p->serverProcess->state() == QProcess::Starting)
  {
    p->serverProcess->waitForStarted(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  if (!p->serverStarted)
  {
    p->serverProcess->setReadChannel(QProcess::StandardError);
    while (p->serverProcess->waitForReadyRead(qMax(timeout - qAbs(timer.elapsed()), 0)))
    while (p->serverProcess->canReadLine())
    {
      const QByteArray line = p->serverProcess->readLine();
      if (line.startsWith("##READY"))
      {
        connect(p->serverProcess, SIGNAL(readyRead()), SLOT(readConsole()));
        QTimer::singleShot(15, this, SLOT(readConsole()));

        p->serverStarted = true;
        return;
      }
      else if (p->logFunc)
        p->logFunc(QString::fromUtf8(line));
    }
  }
}

void SSandboxClient::readConsole(void)
{
  QMutexLocker l(&p->mutex);

  if (p->serverProcess && (p->serverProcess->state() == QProcess::Running))
  while (p->serverProcess->canReadLine())
  {
    const QByteArray line = p->serverProcess->readLine();
    if (line.startsWith("##STOP"))
    {
      qDebug() << "Sandbox process was closed";

      // Let the process terminate by itself
      disconnect(p->serverProcess, SIGNAL(readyRead()), this, SLOT(readConsole()));
      disconnect(p->serverProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(serverFinished(int, QProcess::ExitStatus)));
      QTimer::singleShot(250, p->serverProcess, SLOT(kill()));
      QTimer::singleShot(1000, p->serverProcess, SLOT(deleteLater()));
      p->serverProcess = NULL;
      return;
    }
    else if (p->logFunc)
      p->logFunc(QString::fromUtf8(line));
  }
}

void SSandboxClient::serverFinished(int exitCode, QProcess::ExitStatus)
{
  QMutexLocker l(&p->mutex);

  if (p->serverProcess)
  {
    qDebug() << "Sandbox process terminated with exit code" << exitCode;

    p->serverProcess->deleteLater();
    p->serverProcess = NULL;
  }
}

} // End of namespace
