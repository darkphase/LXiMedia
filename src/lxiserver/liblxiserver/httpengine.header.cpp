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

#include "httpengine.h"

#include <QtNetwork>

namespace LXiServer {

HttpEngine::Header::Header(const HttpEngine *httpEngine)
  : httpEngine(httpEngine)
{
}

HttpEngine::Header::Header(const QByteArray &header, const HttpEngine *httpEngine)
  : httpEngine(httpEngine)
{
  if (header.contains("\r\n\r\n"))
  {
    QList<QByteArray> lines = header.split('\r');
    if (!lines.isEmpty())
      head = lines.takeFirst().split(' ');

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

bool HttpEngine::Header::hasField(const QString &name) const
{
  for (QList< QPair<QString, QString> >::ConstIterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
    return true;

  return false;
}

QString HttpEngine::Header::field(const QString &name) const
{
  for (QList< QPair<QString, QString> >::ConstIterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
    return i->second;

  return QString::null;
}

void HttpEngine::Header::setField(const QString &name, const QString &value)
{
  for (QList< QPair<QString, QString> >::Iterator i=fields.begin(); i!=fields.end(); i++)
  if (i->first.compare(name, Qt::CaseInsensitive) == 0)
  {
    i->second = value;
    return;
  }

  fields += qMakePair(name, value);
}

QDateTime HttpEngine::Header::date(void) const
{
  QDateTime date = QDateTime::fromString(field(fieldDate), dateFormat);
  date.setTimeSpec(Qt::UTC);

  return date.toLocalTime();
}

QByteArray HttpEngine::Header::toByteArray(void) const
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


HttpEngine::RequestHeader::RequestHeader(const HttpEngine *httpEngine)
  : Header(httpEngine)
{
  head << "GET" << "/" << httpVersion;

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

HttpEngine::RequestHeader::RequestHeader(const QByteArray &header, const HttpEngine *httpEngine)
  : Header(header, httpEngine)
{
}

QString HttpEngine::RequestHeader::file(void) const
{
  QByteArray result = path();

  const int q = result.indexOf('?');
  if (q >= 0)
    result = result.left(q);

  const int s = result.lastIndexOf('/');
  if (s > 0)
    result = result.mid(s + 1);

  return QString::fromUtf8(QByteArray::fromPercentEncoding(result));
}

QString HttpEngine::RequestHeader::directory(void) const
{
  QByteArray result = path();

  const int q = result.indexOf('?');
  if (q >= 0)
    result = result.left(q);

  const int s = result.lastIndexOf('/');
  if (s >= 0)
    result = result.left(s + 1);

  return QString::fromUtf8(QByteArray::fromPercentEncoding(result));
}

HttpEngine::ResponseHeader::ResponseHeader(const HttpEngine *httpEngine)
  : Header(httpEngine)
{
  head << httpVersion << "200" << "OK";
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

HttpEngine::ResponseHeader::ResponseHeader(const RequestHeader &request)
  : Header(request.httpEngine)
{
  head << qMin(request.version(), QByteArray(httpVersion)) << "200" << "OK";
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

HttpEngine::ResponseHeader::ResponseHeader(const RequestHeader &request, Status status)
  : Header(request.httpEngine)
{
  head << qMin(request.version(), QByteArray(httpVersion)) << QByteArray::number(status) << statusText(status);
  setDate();

  if (httpEngine)
    setField(httpEngine->senderType(), httpEngine->senderId());
}

HttpEngine::ResponseHeader::ResponseHeader(const QByteArray &header, const HttpEngine *httpEngine)
  : Header(header, httpEngine)
{
  while (head.count() > 3)
    head[2] += " " + head.takeAt(3);
}

void HttpEngine::ResponseHeader::setStatus(Status status)
{
  if (head.size() > 1)
    head[1] = QByteArray::number(status);
  else
    head.append(QByteArray::number(status));

  if (head.size() > 2)
    head[2] = statusText(status);
  else
    head.append(statusText(status));
}

const char * HttpEngine::ResponseHeader::statusText(int code)
{
  switch (code)
  {
  case 100: return "Continue";
  case 101: return "Switching Protocols";

  case 200: return "OK";
  case 201: return "Created";
  case 202: return "Accepted";
  case 203: return "Non-Authoritative Information";
  case 204: return "No Content";
  case 205: return "Reset Content";
  case 206: return "Partial Content";

  case 300: return "Multiple Choices";
  case 301: return "Moved Permanently";
  case 302: return "Found";
  case 303: return "See Other";
  case 304: return "Not Modified";
  case 305: return "Use Proxy";
  case 307: return "Temporary Redirect";

  case 400: return "Bad Request";
  case 401: return "Unauthorized";
  case 402: return "Payment Required";
  case 403: return "Forbidden";
  case 404: return "Not Found";
  case 405: return "Method Not Allowed";
  case 406: return "Not Acceptable";
  case 407: return "Proxy Authentication Required";
  case 408: return "Request Time-out";
  case 409: return "Conflict";
  case 410: return "Gone";
  case 411: return "Length Required";
  case 412: return "Precondition Failed";
  case 413: return "Request Entity Too Large";
  case 414: return "Request-URI Too Large";
  case 415: return "Unsupported Media Type";
  case 416: return "Requested range not satisfiable";
  case 417: return "Expectation Failed";

  case 500: return "Internal Server Error";
  case 501: return "Not Implemented";
  case 502: return "Bad Gateway";
  case 503: return "Service Unavailable";
  case 504: return "Gateway Time-out";
  case 505: return "HTTP Version not supported";
  }

  return "";
}


HttpEngine::RequestMessage::RequestMessage(const QByteArray &message, const HttpEngine *server)
  : RequestHeader(message, server)
{
  const int eoh = message.indexOf("\r\n\r\n");
  if (eoh >= 0)
    data = message.mid(eoh + 4);
}

QByteArray HttpEngine::RequestMessage::toByteArray(void) const
{
  return RequestHeader::toByteArray() + data;
}


HttpEngine::ResponseMessage::ResponseMessage(const QByteArray &message, const HttpEngine *server)
  : ResponseHeader(message, server)
{
  const int eoh = message.indexOf("\r\n\r\n");
  if (eoh >= 0)
    data = message.mid(eoh + 4);
}

QByteArray HttpEngine::ResponseMessage::toByteArray(void) const
{
  return ResponseHeader::toByteArray() + data;
}


} // End of namespace
