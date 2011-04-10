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

int SandboxTest::startSandbox(const QString &mode)
{
  struct Callback : SSandboxServer::Callback
  {
    virtual SSandboxServer::SocketOp handleHttpRequest(const SSandboxServer::RequestHeader &request, QAbstractSocket *socket)
    {
      const QUrl url(request.path());
      const QString path = url.path();
      const QString file = path.mid(path.lastIndexOf('/') + 1);

      if (path.left(path.lastIndexOf('/') + 1) == "/")
      {
        if (url.hasQueryItem("nop"))
        {
          return SSandboxServer::sendResponse(request, socket, SHttpServer::Status_NoContent);
        }
        else if (url.hasQueryItem("stop"))
        {
          qApp->exit(0);
          return SSandboxServer::SocketOp_LeaveOpen;
        }
      }

      return SSandboxServer::sendResponse(request, socket, SHttpServer::Status_NotFound);
    }
  };

  SSandboxServer sandboxServer;
  sandboxServer.initialize(mode);

  Callback callback;
  sandboxServer.registerCallback("/", &callback);

  const int result = qApp->exec();

  sandboxServer.close();

  return result;
}

void SandboxTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);

  sandboxClient = new SSandboxClient("\"" + qApp->applicationFilePath() + "\" --sandbox", SSandboxClient::Mode_Normal, this);
  connect(sandboxClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)), Qt::DirectConnection);
}

void SandboxTest::cleanupTestCase(void)
{
  delete sandboxClient;
  sandboxClient = NULL;

  delete mediaApp;
  mediaApp = NULL;
}

void SandboxTest::sendRequests(void)
{
  responseCount = 0;

  SSandboxClient::RequestMessage nopMessage(sandboxClient);
  nopMessage.setRequest("GET", "/?nop");

  for (int i=0; i<numResponses/2; i++)
    sandboxClient->sendRequest(nopMessage);

  for (int i=0; (i<20) && (responseCount<numResponses/2); i++)
    QTest::qWait(100);

  for (int i=0; i<numResponses/2; i++)
    sandboxClient->sendRequest(nopMessage);

  for (int i=0; (i<20) && (responseCount<numResponses); i++)
    QTest::qWait(100);

  QVERIFY(responseCount == numResponses);
}

void SandboxTest::sendTerminate(void)
{
  responseCount = 0;

  SSandboxClient::RequestMessage nopMessage(sandboxClient);
  nopMessage.setRequest("GET", "/?nop");
  sandboxClient->sendRequest(nopMessage);

  SSandboxClient::RequestMessage stopMessage(sandboxClient);
  stopMessage.setRequest("GET", "/?stop");
  sandboxClient->sendRequest(stopMessage);

  for (int i=0; (i<20) && (responseCount<2); i++)
    QTest::qWait(100);

  sandboxClient->sendRequest(nopMessage);

  for (int i=0; (i<20) && (responseCount<3); i++)
    QTest::qWait(100);

  QVERIFY(responseCount == 3);
}

void SandboxTest::handleResponse(const SHttpEngine::ResponseMessage &)
{
  responseCount.ref();
}
