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

const int SandboxTest::numResponses = 50;

int SandboxTest::startSandbox(const QString &mode)
{
  struct Callback : SSandboxServer::Callback
  {
    virtual SSandboxServer::SocketOp handleHttpRequest(const SSandboxServer::RequestMessage &request, QAbstractSocket *socket)
    {
      const QUrl url(request.path());
      const QString path = url.path();
      const QString file = path.mid(path.lastIndexOf('/') + 1);

      if (path.left(path.lastIndexOf('/') + 1) == "/")
      {
        if (url.hasQueryItem("nop"))
        {
          return SSandboxServer::sendResponse(request, socket, SHttpServer::Status_Ok);
        }
      }

      return SSandboxServer::sendResponse(request, socket, SHttpServer::Status_NotFound);
    }
  };

  SSandboxServer sandboxServer;

  Callback callback;
  sandboxServer.registerCallback("/", &callback);

  sandboxServer.initialize(mode);

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

  SSandboxClient::RequestMessage nopRequest(sandboxClient);
  nopRequest.setRequest("GET", "/?nop");

  for (int i=0; i<numResponses; i++)
    sandboxClient->sendRequest(nopRequest);

  for (int i=0; (i<100) && (responseCount<numResponses); i++)
    QTest::qWait(100);

  QCOMPARE(responseCount, numResponses);
}

void SandboxTest::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SHttpEngine::Status_Ok)
    responseCount++;
}
