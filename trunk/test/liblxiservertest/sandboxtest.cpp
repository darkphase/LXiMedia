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

#include "sandboxtest.h"
#include <QtTest>
#include <LXiServer>

int SandboxTest::startSandbox(const QString &name, const QString &mode)
{
  struct Callback : SandboxServer::Callback
  {
    virtual SandboxServer::SocketOp handleHttpRequest(const SandboxServer::RequestHeader &request, QIODevice *socket)
    {
      const QUrl url(request.path());
      const QString path = url.path();
      const QString file = path.mid(path.lastIndexOf('/') + 1);

      if (path.left(path.lastIndexOf('/') + 1) == "/")
      {
        if (url.hasQueryItem("echo"))
          return SandboxServer::sendResponse(request, socket, HttpServer::Status_NoContent, url.queryItemValue("echo").toAscii());
      }

      return SandboxServer::sendResponse(request, socket, HttpServer::Status_NotFound);
    }
  };

  SandboxServer sandboxServer;
  sandboxServer.initialize(name, mode);

  Callback callback;
  sandboxServer.registerCallback("/", &callback);

  const int result = qApp->exec();

  sandboxServer.close();

  return result;
}

void SandboxTest::initTestCase(void)
{
  SandboxClient::sandboxApplication() = "\"" + qApp->applicationFilePath() + "\" --sandbox";

  sandboxClient = new SandboxClient(SandboxClient::Mode_Normal, this);
}

void SandboxTest::cleanupTestCase(void)
{
  delete sandboxClient;
  sandboxClient = NULL;
}

void SandboxTest::sendRequest(void)
{
  const int value = qrand();
  QVERIFY(sandboxClient->sendRequest("/?echo=" + QByteArray::number(value)).toInt() == value);
}

void SandboxTest::sendRequestMultiThreaded(void)
{
  QThreadPool::globalInstance()->setMaxThreadCount(10);

  for (unsigned i=0; i<100; i++)
    QtConcurrent::run(this, &SandboxTest::sendRequest);

  QThreadPool::globalInstance()->waitForDone();
  QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
}
