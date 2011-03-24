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

#include "httpservertest.h"
#include <QtTest>
#include <LXiServer>

void HttpServerTest::initTestCase(void)
{
}

void HttpServerTest::cleanupTestCase(void)
{
}

/*! Tests the HTTP server.
 */
void HttpServerTest::Server(void)
{
  static const QHostAddress localhost = QHostAddress("127.0.0.1");

  struct Callback : SHttpServer::Callback
  {
    virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestHeader &request, QIODevice *socket)
    {
      if (request.path() == "/test.txt")
      {
        const QString text = "hello world\n";

        SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
        response.setContentLength(text.size());
        response.setContentType(SHttpServer::toMimeType(".txt"));
        socket->write(response);
        socket->write(text.toUtf8());
        return SHttpServer::SocketOp_Close;
      }

      return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound);
    }
  } callback;

  SHttpServer httpServer("TEST/1.0", QUuid::createUuid());
  httpServer.initialize(QList<QHostAddress>() << localhost);
  QVERIFY(httpServer.serverPort(localhost) > 0);

  httpServer.registerCallback("/", &callback);

  gotHttpServerReply = false;
  QNetworkAccessManager manager;
  connect(&manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(ServerReply(QNetworkReply *)));

  // An existing file
  manager.get(QNetworkRequest(QUrl("http://127.0.0.1:" + QString::number(httpServer.serverPort(localhost)) + "/test.txt")));

  for (unsigned i=0; (i<20) && (gotHttpServerReply==false); i++)
    QTest::qWait(100);

  QVERIFY(gotHttpServerReply == true);

  // A non-existing file
  manager.get(QNetworkRequest(QUrl("http://127.0.0.1:" + QString::number(httpServer.serverPort(localhost)) + "/dummy")));

  for (unsigned i=0; (i<20) && (gotHttpServerReply==true); i++)
    QTest::qWait(100);

  QVERIFY(gotHttpServerReply == false);

  httpServer.unregisterCallback(&callback);
  httpServer.close();
}

void HttpServerTest::ServerReply(QNetworkReply *reply)
{
  gotHttpServerReply = (reply->error() == QNetworkReply::NoError);

  reply->deleteLater();
}
