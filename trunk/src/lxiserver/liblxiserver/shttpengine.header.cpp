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

#include "shttpengine.h"

#include <QtNetwork>

namespace LXiServer {

SHttpEngine::Header::Header(const SHttpEngine *httpEngine)
  : httpEngine(httpEngine)
{
}

bool SHttpEngine::Header::isValid(void) const
{
  return head.count() == 3;
}

bool SHttpEngine::Header::hasField(const QString &name) const
{
  for (QList< QPair<QString, QString> >::ConstIterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
    return true;

  return false;
}

QString SHttpEngine::Header::field(const QString &name) const
{
  for (QList< QPair<QString, QString> >::ConstIterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
    return i->second;

  return QString::null;
}

void SHttpEngine::Header::setField(const QString &name, const QString &value)
{
  for (QList< QPair<QString, QString> >::Iterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
  {
    i->second = value;
    return;
  }

  fields += qMakePair(name, value);
}

QDateTime SHttpEngine::Header::date(void) const
{
  QDateTime date = QDateTime::fromString(field(fieldDate), dateFormat);
  date.setTimeSpec(Qt::UTC);

  return date.toLocalTime();
}

void SHttpEngine::Header::setHost(const QString &hostname, quint16 port)
{
  QString host;
  if (!hostname.contains(':'))
    host += hostname;
  else
    host += '[' + hostname + ']';

  if ((port > 0) && (port != 80))
    host += ':' + QString::number(port);

  setHost(host);
}

void SHttpEngine::Header::setHost(const QHostAddress &address, quint16 port)
{
  QString host;
  if (address.protocol() == QAbstractSocket::IPv4Protocol)
    host += address.toString();
  else if (address.protocol() == QAbstractSocket::IPv6Protocol)
    host += '[' + address.toString() + ']';

  if ((port > 0) && (port != 80))
    host += ':' + QString::number(port);

  setHost(host);
}

QByteArray SHttpEngine::Header::toByteArray(void) const
{
  QByteArray result;
  if (isValid())
  {
    result = head[0] + ' ' + head[1] + ' ' + head[2] + "\r\n";
    for (QList< QPair<QString, QString> >::ConstIterator i=fields.begin(); i!=fields.end(); i++)
      result += i->first.toUtf8() + ": " + i->second.toUtf8() + "\r\n";

    result += "\r\n";
  }

  return result;
}

void SHttpEngine::Header::parse(const QByteArray &header)
{
  head.clear();
  fields.clear();

  if (header.contains("\r\n\r\n"))
  {
    QList<QByteArray> lines = header.split('\r');
    if (!lines.isEmpty())
    foreach (const QByteArray &item, lines.takeFirst().simplified().split(' '))
      head.append(item);

    foreach (const QByteArray &line, lines) // Note: each line starts with a \n.
    {
      const int colon = line.indexOf(':');
      if (colon > 0)
      {
        setField(
            QString::fromUtf8(line.mid(1, colon - 1)).trimmed(),
            QString::fromUtf8(line.mid(colon + 1)).trimmed());
      }
    }
  }
}


SHttpEngine::RequestHeader::RequestHeader(const SHttpEngine *httpEngine)
  : Header(httpEngine)
{
  head << "GET" << "/" << httpVersion;

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

bool SHttpEngine::RequestHeader::isGet(void) const
{
  return (method() == "GET") || (method() == "HEAD");
}

bool SHttpEngine::RequestHeader::isHead(void) const
{
  return method() == "HEAD";
}

bool SHttpEngine::RequestHeader::isPost(void) const
{
  return method() == "POST";
}

QString SHttpEngine::RequestHeader::file(void) const
{
  QByteArray result = path();

  const int q = result.indexOf('?');
  if (q >= 0)
    result = result.left(q);

  return QString::fromUtf8(QByteArray::fromPercentEncoding(result));
}

SHttpEngine::ResponseHeader::ResponseHeader(const SHttpEngine *httpEngine)
  : Header(httpEngine)
{
  head << httpVersion << "200" << "OK";
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

SHttpEngine::ResponseHeader::ResponseHeader(const RequestHeader &request)
  : Header(request.httpEngine)
{
  head << qMin(request.version(), QByteArray(httpVersion)) << "200" << "OK";
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

SHttpEngine::ResponseHeader::ResponseHeader(const RequestHeader &request, const Status &status)
  : Header(request.httpEngine)
{
  head << qMin(request.version(), QByteArray(httpVersion)) << QByteArray::number(status.statusCode()) << status.description();
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

void SHttpEngine::ResponseHeader::setStatus(const Status &status)
{
  if (head.size() > 1)
    head[1] = QByteArray::number(status.statusCode());
  else
    head.append(QByteArray::number(status.statusCode()));

  if (head.size() > 2)
    head[2] = status.description();
  else
    head.append(status.description());
}

void SHttpEngine::ResponseHeader::parse(const QByteArray &header)
{
  Header::parse(header);

  while (head.count() > 3)
    head[2] += " " + head.takeAt(3);
}


SHttpEngine::RequestMessage::RequestMessage(const SHttpEngine *server)
  : RequestHeader(server)
{
  if (data.length() > 0)
    setContentLength(data.length());
}

bool SHttpEngine::RequestMessage::isComplete(void) const
{
  return data.length() >= contentLength();
}

void SHttpEngine::RequestMessage::setContent(const QByteArray &content)
{
  data = content;

  if (data.length() > 0)
    setContentLength(data.length());
}

QByteArray SHttpEngine::RequestMessage::toByteArray(void) const
{
  return RequestHeader::toByteArray() + data;
}

void SHttpEngine::RequestMessage::parse(const QByteArray &message)
{
  const int eoh = message.indexOf("\r\n\r\n");
  if (eoh >= 0)
  {
    RequestHeader::parse(message.left(eoh + 4));
    data = message.mid(eoh + 4);
  }
  else
    RequestHeader::parse(message);
}


SHttpEngine::ResponseMessage::ResponseMessage(const SHttpEngine *httpEngine = NULL)
  : ResponseHeader(httpEngine) 
{ 
  setContentLength(data.length());
}

SHttpEngine::ResponseMessage::ResponseMessage(const RequestHeader &request) 
  : ResponseHeader(request) 
{ 
  setContentLength(data.length());
}

SHttpEngine::ResponseMessage::ResponseMessage(const RequestHeader &request, Status status) 
  : ResponseHeader(request, status) 
{ 
  setContentLength(data.length());
}

SHttpEngine::ResponseMessage::ResponseMessage(const RequestHeader &request, Status status, const QByteArray &data, const QString &contentType)
  : ResponseHeader(request, status),
    data(data)
{
  setContentLength(data.length());
  setContentType(contentType);
}

bool SHttpEngine::ResponseMessage::isComplete(void) const
{
  return data.length() >= contentLength();
}

void SHttpEngine::ResponseMessage::setContent(const QByteArray &content)
{
  data = content;
  setContentLength(data.length());
}

QByteArray SHttpEngine::ResponseMessage::toByteArray(void) const
{
  return ResponseHeader::toByteArray() + data;
}

void SHttpEngine::ResponseMessage::parse(const QByteArray &message)
{
  const int eoh = message.indexOf("\r\n\r\n");
  if (eoh >= 0)
  {
    ResponseHeader::parse(message.left(eoh + 4));
    data = message.mid(eoh + 4);
  }
  else
    ResponseHeader::parse(message);
}


} // End of namespace
