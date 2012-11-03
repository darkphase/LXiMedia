/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "sandboxtest.h"
#include <QtTest>
#include <LXiServer>

const int SandboxTest::numResponses = 100;

int SandboxTest::startSandbox(const QString &mode)
{
  SSandboxServer * const sandboxServer = createSandbox();
  sandboxServer->initialize(mode, QString::null);

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
    Callback(SSandboxServer *parent)
      : QObject(parent),
        parent(parent)
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
          else if (url.hasQueryItem("exit"))
          {
            parent->close();

            return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok);
          }
        }
      }

      qWarning() << "Corrupted request" << request.method() << request.path() << request.version();

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
    }

  private:
    SSandboxServer * const parent;
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
  sandboxServer->initialize("local", QString::null);

  LXiServer::SSandboxClient * const sandboxClient = new SSandboxClient(sandboxServer, SSandboxClient::Priority_Normal, this);
  connect(sandboxClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)), Qt::DirectConnection);

  testClient(sandboxClient);

  delete sandboxClient;

  testBlockingClient(sandboxServer);

  sandboxServer->close();
  delete sandboxServer;
}

void SandboxTest::remoteSandbox(void)
{
  LXiServer::SSandboxClient * const sandboxClient = new SSandboxClient("\"" + qApp->applicationFilePath() + "\" --sandbox", SSandboxClient::Priority_Normal, this);
  connect(sandboxClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)), Qt::DirectConnection);

  testClient(sandboxClient);
  testBlockingClient(sandboxClient);

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

  SSandboxClient::RequestMessage nopRequest(client);
  nopRequest.setRequest("GET", "/?nop");

  for (int i=0; i<numResponses; i++)
  {
    const SHttpEngine::ResponseMessage response = client->blockingRequest(nopRequest);
    if (response.status() == SHttpEngine::Status_Ok)
      responseCount.ref();
    else
      qWarning() << "Corrupted response" << response.status().statusCode() << response.status().description();
  }

  QCOMPARE(int(responseCount), numResponses);
}

void SandboxTest::testBlockingClient(SSandboxServer *server)
{
  responseCount = 0;

  class Thread : public QThread
  {
  public:
    inline Thread(SandboxTest *parent, SSandboxServer *server)
      : QThread(parent), parent(parent), server(server)
    {
    }

    inline virtual void run(void)
    {
      LXiServer::SSandboxClient * const client = new SSandboxClient(server, SSandboxClient::Priority_Normal, NULL);

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

      delete client;
    }

    SandboxTest * const parent;
    SSandboxServer * const server;
  };

  static const int numThreads = 4;
  Thread * thread[numThreads];
  for (int i=0; i<numThreads; i++)
    (thread[i] = new Thread(this, server))->start();

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
