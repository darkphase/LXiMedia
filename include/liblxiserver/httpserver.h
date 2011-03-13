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

#ifndef LXISERVER_HTTPSERVER_H
#define LXISERVER_HTTPSERVER_H

#include <QtCore>
#include <QtNetwork>

namespace LXiServer {

class HttpServer : public QThread
{
Q_OBJECT
public:
  enum SocketOp               { SocketOp_Close, SocketOp_LeaveOpen };

  enum Status
  {
    Status_None               = 0,

    Status_Continue           = 100,
    Status_SwitchingProtocols = 101,
    Status_Ok                 = 200,
    Status_NoContent          = 204,
    Status_MovedPermanently   = 301,
    Status_TemporaryRedirect  = 307,
    Status_BadRequest         = 400,
    Status_NotFound           = 404,
    Status_PreconditionFailed = 412,
    Status_InternalServerError= 500
  };

  class Header
  {
  protected:
    explicit                    Header(const HttpServer *);
                                Header(const QByteArray &, const HttpServer *);

  public:
    inline bool                 isValid(void) const                             { return head.count() == 3; }

    bool                        hasField(const QString &name) const;
    QString                     field(const QString &name) const;
    void                        setField(const QString &name, const QString &value);

    inline QString              connection(void) const                          { return field(fieldConnection); }
    inline void                 setConnection(const QString &type)              { setField(fieldConnection, type); }
    inline qint64               contentLength(void) const                       { return field(fieldContentLength).toLongLong(); }
    inline void                 setContentLength(qint64 len)                    { setField(fieldContentLength, QString::number(len)); }
    inline QString              contentType(void) const                         { return field(fieldContentType); }
    inline void                 setContentType(const QString &type)             { setField(fieldContentType, type); }
    QDateTime                   date(void) const;
    inline void                 setDate(const QDateTime &date)                  { setField(fieldDate, date.toUTC().toString(dateFormat)); }
    inline void                 setDate(void)                                   { setDate(QDateTime::currentDateTime()); }
    inline QString              host(void) const                                { return field(fieldHost); }
    inline void                 setHost(const QString &type)                    { setField(fieldHost, type); }
    inline QString              server(void) const                              { return field(fieldServer); }
    inline void                 setServer(const QString &server)                { setField(fieldServer, server); }
    inline void                 setServer(void)                                 { if (httpServer) setServer(httpServer->serverId()); }

    QByteArray                  toUtf8(void) const;
    inline                      operator QByteArray() const                     { return toUtf8(); }

  public:
    const HttpServer    * const httpServer;

  protected:
    static const char   * const fieldConnection;
    static const char   * const fieldContentLength;
    static const char   * const fieldContentType;
    static const char   * const fieldDate;
    static const char   * const fieldHost;
    static const char   * const fieldServer;
    static const char   * const dateFormat;

    QList<QByteArray>           head;

  private:
    QList< QPair<QString, QString> > fields;
  };

  class RequestHeader : public Header
  {
  public:
    explicit                    RequestHeader(const HttpServer *);
                                RequestHeader(const QByteArray &, const HttpServer *);

    inline QByteArray           method(void) const                              { return isValid() ? head[0] : QByteArray(); }
    inline void                 setMethod(const QByteArray &method)             { if (head.size() > 0) head[0] = method; else head.append(method); }
    inline QByteArray           path(void) const                                { return isValid() ? head[1] : QByteArray(); }
    inline void                 setPath(const QByteArray &path)                 { if (head.size() > 1) head[1] = path; else head.append(path); }
    inline QByteArray           version(void) const                             { return isValid() ? head[2] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 2) head[2] = version; else head.append(version); }

    inline void                 setRequest(const QByteArray &method, const QByteArray &path) { setMethod(method); setPath(path);  }
    inline void                 setRequest(const QByteArray &method, const QByteArray &path, const QByteArray &version) { setMethod(method); setPath(path); setVersion(version); }

    QString                     file(void) const;
    QString                     directory(void) const;
  };

  class ResponseHeader : public Header
  {
  public:
    explicit                    ResponseHeader(const HttpServer *);
    explicit                    ResponseHeader(const RequestHeader &);
                                ResponseHeader(const RequestHeader &, Status);
    explicit                    ResponseHeader(const QByteArray &header, const HttpServer *);

    inline QByteArray           version(void) const                             { return isValid() ? head[0] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 0) head[0] = version; else head.append(version); }
    inline Status               status(void) const                              { return isValid() ? Status(head[1].toInt()) : Status_None; }
    void                        setStatus(Status status);

    inline void                 setResponse(Status status)                      { setStatus(status); }
    inline void                 setResponse(Status status, const QByteArray &version) { setStatus(status); setVersion(version); }

    static const char         * statusText(int) __attribute__((pure));
  };

  struct Callback
  {
    virtual SocketOp            handleHttpRequest(const RequestHeader &, QAbstractSocket *) = 0;
  };

public:
  explicit                      HttpServer(const QString &protocol, const QUuid &serverUuid);
  virtual                       ~HttpServer();

  void                          initialize(const QList<QHostAddress> &addresses, quint16 port = 0);
  void                          close(void);

  QThreadPool                 * threadPool(void);

  quint16                       serverPort(const QHostAddress &) const;
  const QString               & serverId(void) const;
  const QString               & serverUdn(void) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

public:
  static SocketOp               sendResponse(const RequestHeader &, QAbstractSocket *, Status, const QObject * = NULL);
  static SocketOp               sendRedirect(const RequestHeader &, QAbstractSocket *, const QString &);
  static const char           * toMimeType(const QString &fileName) __attribute__((pure));

private:
  virtual void                  run(void);

public:
  static const char     * const httpVersion;

private:
  class Interface;
  class SocketHandler;
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
