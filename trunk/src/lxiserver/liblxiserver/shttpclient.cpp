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

#include "shttpclient.h"
#include "lxiserverprivate.h"
#include <QtNetwork>

namespace LXiServer {

struct SHttpClient::Data
{
  struct Request
  {
    inline Request(const QString &hostname, quint16 port, const QByteArray &message, QObject *receiver, const char *slot)
      : hostname(hostname), port(port), message(message), receiver(receiver), slot(slot)
    {
    }

    QString                     hostname;
    quint16                     port;
    QByteArray                  message;
    QObject                   * receiver;
    const char                * slot;
  };

  QList<Request>                requests;
};

SHttpClient::SHttpClient(QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
}

SHttpClient::~SHttpClient()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SHttpClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot)
{
  if (QThread::currentThread() != thread())
    qFatal("SHttpClient::openRequest() should be invoked from the thread "
           "that owns the SHttpClient object.");

  QString hostname;
  quint16 port = 80;
  if (splitHost(message.host(), hostname, port))
  {
    d->requests.append(Data::Request(hostname, port, message, receiver, slot));
    openRequest();
  }
}

void SHttpClient::socketDestroyed(void)
{
  SHttpClientEngine::socketDestroyed();

  openRequest();
}

void SHttpClient::openRequest(void)
{
  while (!d->requests.isEmpty() && (socketsAvailable() > 0))
  {
    const Data::Request request = d->requests.takeFirst();
    HttpSocketRequest * const socketRequest = new HttpSocketRequest(this, createSocket(), request.hostname, request.port, request.message);

    connect(socketRequest, SIGNAL(connected(QAbstractSocket *)), request.receiver, request.slot);
  }
}

} // End of namespace
