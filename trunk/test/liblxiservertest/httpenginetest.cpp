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

#include "httpenginetest.h"
#include <QtTest>

const int HttpEngineTest::numResponses = 100;

void HttpEngineTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);
}

void HttpEngineTest::cleanupTestCase(void)
{
  if (httpServer)
  {
    httpServer->close();
    delete httpServer;
    httpServer = NULL;
  }

  delete mediaApp;
  mediaApp = NULL;
}

/*! Tests the HTTP server on IPv4.
 */
void HttpEngineTest::HttpServerIPv4(void)
{
  if (startServer(QHostAddress::LocalHost))
  {
    testQtClient(QHostAddress::LocalHost);
    stopServer();
  }
  else
  {
    stopServer();
    QSKIP("IPv4 not available.", SkipSingle);
  }
}

/*! Tests the HTTP server on IPv6.
 */
void HttpEngineTest::HttpServerIPv6(void)
{
  if (startServer(QHostAddress::LocalHostIPv6))
  {
    testQtClient(QHostAddress::LocalHostIPv6);
    stopServer();
  }
  else
  {
    stopServer();
    QSKIP("IPv6 not available.", SkipSingle);
  }
}

/*! Tests the HTTP client on IPv4.
 */
void HttpEngineTest::HttpClientIPv4(void)
{
  if (startServer(QHostAddress::LocalHost))
  {
    testHttpClient(QHostAddress::LocalHost);
    testBlockingHttpClient(QHostAddress::LocalHost);
    stopServer();
  }
  else
  {
    stopServer();
    QSKIP("IPv4 not available.", SkipSingle);
  }
}

/*! Tests the HTTP client on IPv6.
 */
void HttpEngineTest::HttpClientIPv6(void)
{
  if (startServer(QHostAddress::LocalHostIPv6))
  {
    testHttpClient(QHostAddress::LocalHostIPv6);
    testBlockingHttpClient(QHostAddress::LocalHostIPv6);
    stopServer();
  }
  else
  {
    stopServer();
    QSKIP("IPv6 not available.", SkipSingle);
  }
}

bool HttpEngineTest::startServer(const QHostAddress &address)
{
  class Callback : public QObject,
                   public SHttpServer::Callback
  {
  public:
    Callback(QObject *parent)
      : QObject(parent)
    {
    }

    virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
    {
      if (request.isGet())
      {
        if (request.path() == "/test.txt")
        {
          const QString text = "hello world\n";

          SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
          response.setContentType(SHttpServer::toMimeType(".txt"));
          response.setContent(text.toUtf8());

          return response;
        }
      }

      qWarning() << "Corrupted request" << request.method() << request.path() << request.version();

      return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
    }
  };

  httpServer = new SHttpServer("TEST/1.0", QUuid::createUuid());
  httpServer->initialize();
  httpServer->bind(address);
  httpServer->registerCallback("/", new Callback(this));

  return httpServer->serverPort(address) > 0;
}

void HttpEngineTest::stopServer(void)
{
  httpServer->close();
  delete httpServer;
  httpServer = NULL;
}

void HttpEngineTest::testQtClient(const QHostAddress &address)
{
  responseCount = 0;

  QNetworkAccessManager manager;
  connect(&manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(serverReply(QNetworkReply *)));

  QString url = "http://";
  if (address.protocol() == QAbstractSocket::IPv4Protocol)
    url += address.toString();
  else if (address.protocol() == QAbstractSocket::IPv6Protocol)
    url += '[' + address.toString() + ']';

  url += ':' + QString::number(httpServer->serverPort(address));
  url += "/test.txt";

  for (int i=0; i<numResponses; i++)
    manager.get(QNetworkRequest(url));

  for (unsigned i=0; (i<100) && (responseCount<numResponses); i++)
    QTest::qWait(100);

  QCOMPARE(int(responseCount), numResponses);
}

void HttpEngineTest::testHttpClient(const QHostAddress &address)
{
  responseCount = 0;

  SHttpClient httpClient;
  connect(&httpClient, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));

  SHttpEngine::RequestMessage request(&httpClient);
  request.setRequest("GET", "/test.txt");
  request.setHost(address, httpServer->serverPort(address));

  for (int i=0; i<numResponses; i++)
    httpClient.sendRequest(request);

  for (unsigned i=0; (i<100) && (responseCount<numResponses); i++)
    QTest::qWait(100);

  QCOMPARE(int(responseCount), numResponses);
}

void HttpEngineTest::testBlockingHttpClient(const QHostAddress &address)
{
  responseCount = 0;

  class Thread : public QThread
  {
  public:
    inline Thread(HttpEngineTest *parent, const QHostAddress &address)
      : QThread(parent), parent(parent), address(address)
    {
    }

    inline virtual void run(void)
    {
      SHttpClient httpClient;

      SHttpEngine::RequestMessage request(&httpClient);
      request.setRequest("GET", "/test.txt");
      request.setHost(address, parent->httpServer->serverPort(address));

      for (int i=0; i<parent->numResponses; i++)
      {
        const SHttpEngine::ResponseMessage response = httpClient.blockingRequest(request);
        if (response.status() == SHttpEngine::Status_Ok)
          parent->responseCount.ref();
        else
          qWarning() << "Corrupted response" << response.status().statusCode() << response.status().description();
      }
    }

    HttpEngineTest * const parent;
    const QHostAddress address;
  };

  static const int numThreads = 4;
  Thread * thread[numThreads];
  for (int i=0; i<numThreads; i++)
    (thread[i] = new Thread(this, address))->start();

  for (int i=0; i<numThreads; i++)
  while (!thread[i]->wait(0))
    QTest::qWait(100);

  QCOMPARE(int(responseCount), numResponses * numThreads);
}

void HttpEngineTest::serverReply(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError)
    responseCount.ref();
  else
    qWarning() << "Corrupted response" << reply->errorString();

  reply->deleteLater();
}

void HttpEngineTest::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SHttpEngine::Status_Ok)
    responseCount.ref();
  else
    qWarning() << "Corrupted response" << response.status().statusCode() << response.status().description();
}
