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

const int SandboxTest::numResponses = 100;

int SandboxTest::startSandbox(const QString &mode)
{
  SSandboxServer * const sandboxServer = createSandbox();
  sandboxServer->initialize(mode);

  const int result = qApp->exec();

  sandboxServer->close();
  delete sandboxServer;

  return result;
}

SSandboxServer * SandboxTest::createSandbox(void)
{
  class Callback : public QObject,
                   public SSandboxServer::Callback
  {
  public:
    Callback(QObject *parent)
      : QObject(parent)
    {
    }

    virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *)
    {
      if (request.isGet())
      {
        const QUrl url(request.path());
        const QString path = url.path();
        const QString file = path.mid(path.lastIndexOf('/') + 1);

        if (path.left(path.lastIndexOf('/') + 1) == "/")
        {
          if (url.hasQueryItem("nop"))
          {
            return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok);
          }
        }
      }

      qWarning() << "Corrupted request" << request.method() << request.path() << request.version();

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
    }
  };

  SSandboxServer * const sandboxServer = new SSandboxServer();
  sandboxServer->registerCallback("/", new Callback(sandboxServer));

  return sandboxServer;
}

void SandboxTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);
}

void SandboxTest::cleanupTestCase(void)
{
  delete mediaApp;
  mediaApp = NULL;
}

void SandboxTest::localSandbox(void)
{
  SSandboxServer * const sandboxServer = createSandbox();
  sandboxServer->initialize("local");

  LXiServer::SSandboxClient * const sandboxClient = new SSandboxClient(sandboxServer, SSandboxClient::Priority_Normal, this);
  connect(sandboxClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)), Qt::DirectConnection);

  testClient(sandboxClient);
  testBlockingClient(sandboxClient);

  delete sandboxClient;

  sandboxServer->close();
  delete sandboxServer;
}

void SandboxTest::remoteSandbox(void)
{
  LXiServer::SSandboxClient * const sandboxClient = new SSandboxClient("\"" + qApp->applicationFilePath() + "\" --sandbox", SSandboxClient::Priority_Normal, this);
  connect(sandboxClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)), Qt::DirectConnection);

  testClient(sandboxClient);

  delete sandboxClient;
}

void SandboxTest::testClient(SSandboxClient *client)
{
  responseCount = 0;

  SSandboxClient::RequestMessage nopRequest(client);
  nopRequest.setRequest("GET", "/?nop");

  for (int i=0; i<numResponses; i++)
    client->sendRequest(nopRequest);

  for (int i=0; (i<100) && (responseCount<numResponses); i++)
    QTest::qWait(100);

  QCOMPARE(int(responseCount), numResponses);
}

void SandboxTest::testBlockingClient(SSandboxClient *client)
{
  responseCount = 0;

  class Thread : public QThread
  {
  public:
    inline Thread(SandboxTest *parent, SSandboxClient *client)
      : QThread(parent), parent(parent), client(client)
    {
    }

    inline virtual void run(void)
    {
      SSandboxClient::RequestMessage nopRequest(client);
      nopRequest.setRequest("GET", "/?nop");

      for (int i=0; i<parent->numResponses; i++)
      {
        const SHttpEngine::ResponseMessage response = client->blockingRequest(nopRequest);
        if (response.status() == SHttpEngine::Status_Ok)
          parent->responseCount.ref();
        else
          qWarning() << "Corrupted response" << response.status().statusCode() << response.status().description();
      }
    }

    SandboxTest * const parent;
    SSandboxClient * const client;
  };

  static const int numThreads = 4;
  Thread * thread[numThreads];
  for (int i=0; i<numThreads; i++)
    (thread[i] = new Thread(this, client))->start();

  for (int i=0; i<numThreads; i++)
  while (!thread[i]->wait(0))
    QTest::qWait(100);

  QCOMPARE(int(responseCount), numResponses * numThreads);
}

void SandboxTest::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SSandboxServer::Status_Ok)
    responseCount.ref();
  else
    qWarning() << "Corrupted response" << response.status().statusCode() << response.status().description();
}
