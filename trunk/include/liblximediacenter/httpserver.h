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

#ifndef LXMEDIACENTER_HTTPSERVER_H
#define LXMEDIACENTER_HTTPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiStream>

namespace LXiMediaCenter {

class HttpServer : public QThread
{
Q_OBJECT
protected:
  class Header
  {
  protected:
    inline                      Header(void)                                    { }
    explicit                    Header(const QByteArray &);

  public:
    inline bool                 isValid(void) const                             { return head.count() == 3; }

    inline bool                 hasField(const QString &name) const             { return fields.contains(name); }
    inline QString              field(const QString &name) const                { QMap<QString, QString>::ConstIterator i = fields.find(name); return (i != fields.end()) ? *i : QString::null; }
    inline void                 setField(const QString &name, const QString &value) { fields.insert(name, value); }

    inline qint64               contentLength(void) const                       { return field(fieldContentLength).toLongLong(); }
    inline void                 setContentLength(qint64 len)                    { setField(fieldContentLength, QString::number(len)); }
    inline QString              contentType(void) const                         { return field(fieldContentType); }
    inline void                 setContentType(const QString &type)             { setField(fieldContentType, type); }
    inline QString              host(void) const                                { return field(fieldHost); }
    inline void                 setHost(const QString &type)                    { setField(fieldHost, type); }

    QByteArray                  toUtf8(void) const;
    inline                      operator QByteArray() const                     { return toUtf8(); }

  protected:
    QList<QByteArray>           head;
    QMap<QString, QString>      fields;
  };

public:
  enum SocketOp               { SocketOp_Close, SocketOp_LeaveOpen };

  enum Status
  {
    Status_None               = 0,

    Status_Ok                 = 200,
    Status_NoContent          = 204,
    Status_MovedPermanently   = 301,
    Status_BadRequest         = 400,
    Status_NotFound           = 404,
    Status_InternalServerError= 500
  };

  class RequestHeader : public Header
  {
  public:
                                RequestHeader(void);
    inline explicit             RequestHeader(const QByteArray &header) : Header(header) { }

    inline QByteArray           method(void) const                              { return isValid() ? head[0] : QByteArray(); }
    inline void                 setMethod(const QByteArray &method)             { if (head.size() > 0) head[0] = method; else head.append(method); }
    inline QByteArray           path(void) const                                { return isValid() ? head[1] : QByteArray(); }
    inline void                 setPath(const QByteArray &path)                 { if (head.size() > 1) head[1] = path; else head.append(path); }
    inline QByteArray           version(void) const                             { return isValid() ? head[2] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 2) head[2] = version; else head.append(version); }

    QString                     file(void) const;
  };

  class ResponseHeader : public Header
  {
  public:
                                ResponseHeader(Status = Status_Ok);
    explicit                    ResponseHeader(const QByteArray &header);

    inline QByteArray           version(void) const                             { return isValid() ? head[0] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 0) head[0] = version; else head.append(version); }
    inline Status               status(void) const                              { return isValid() ? Status(head[1].toInt()) : Status_None; }
    void                        setStatus(Status status);
  };

  struct Callback
  {
    virtual SocketOp            handleHttpRequest(const RequestHeader &, QAbstractSocket *) = 0;
  };

public:
  explicit                      HttpServer(void);
  virtual                       ~HttpServer();

  void                          initialize(const QList<QHostAddress> &addresses, quint16 port = 0);
  void                          close(void);

  quint16                       serverPort(const QHostAddress &) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

public:
  static const char           * toMimeType(const QString &fileName) __attribute__((pure));
  static const char           * statusText(int) __attribute__((pure));

private:
  virtual void                  run(void);

private:
  static const char     * const fieldContentLength;
  static const char     * const fieldContentType;
  static const char     * const fieldHost;

private:
  class Interface;
  class SocketHandler;
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
