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

#ifndef LXISERVER_SHTTPENGINE_H
#define LXISERVER_SHTTPENGINE_H

#include <QtCore>

namespace LXiServer {

class SHttpEngine
{
public:
  enum Status
  {
    Status_None               = 0,

    Status_Continue           = 100,
    Status_SwitchingProtocols = 101,
    Status_Ok                 = 200,
    Status_NoContent          = 204,
    Status_MultipleChoices    = 300,
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
    explicit                    Header(const SHttpEngine *);
                                Header(const QByteArray &, const SHttpEngine *);

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
    inline QString              userAgent(void) const                           { return field(fieldUserAgent); }
    inline void                 setUserAgent(const QString &userAgent)          { setField(fieldUserAgent, userAgent); }

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }

  public:
    const SHttpEngine    * const httpEngine;

  protected:
    QList<QByteArray>           head;

  private:
    QList< QPair<QString, QString> > fields;
  };

  class RequestHeader : public Header
  {
  public:
    explicit                    RequestHeader(const SHttpEngine *);
                                RequestHeader(const QByteArray &, const SHttpEngine *);

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
    explicit                    ResponseHeader(const SHttpEngine *);
    explicit                    ResponseHeader(const RequestHeader &);
                                ResponseHeader(const RequestHeader &, Status);
                                ResponseHeader(const QByteArray &, const SHttpEngine *);

    inline QByteArray           version(void) const                             { return isValid() ? head[0] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 0) head[0] = version; else head.append(version); }
    inline Status               status(void) const                              { return isValid() ? Status(head[1].toInt()) : Status_None; }
    void                        setStatus(Status status);

    inline void                 setResponse(Status status)                      { setStatus(status); }
    inline void                 setResponse(Status status, const QByteArray &version) { setStatus(status); setVersion(version); }

    static const char         * statusText(int) __attribute__((pure));
  };

  class RequestMessage : public RequestHeader
  {
  public:
    inline explicit             RequestMessage(const SHttpEngine *httpEngine = NULL) : RequestHeader(httpEngine) { }
                                RequestMessage(const QByteArray &, const SHttpEngine * = NULL);

    inline const QByteArray   & content(void) const                             { return data; }
    inline void                 setContent(const QByteArray &content)           { data = content; }

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }

  private:
    QByteArray                  data;
  };

  class ResponseMessage : public ResponseHeader
  {
  public:
    inline explicit             ResponseMessage(const SHttpEngine *httpEngine = NULL) : ResponseHeader(httpEngine) { }
    inline explicit             ResponseMessage(const RequestHeader &request) : ResponseHeader(request) { }
    inline                      ResponseMessage(const RequestHeader &request, Status status) : ResponseHeader(request, status) { }
                                ResponseMessage(const QByteArray &, const SHttpEngine * = NULL);

    inline const QByteArray   & content(void) const                             { return data; }
    inline void                 setContent(const QByteArray &content)           { data = content; }

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }

  private:
    QByteArray                  data;
  };

public:
                                SHttpEngine(void);
  virtual                       ~SHttpEngine();

  virtual const char          * senderType(void) const = 0;
  virtual const QString       & senderId(void) const = 0;

  static const char           * toMimeType(const QString &fileName) __attribute__((pure));

public:
  static const char     * const httpVersion;
  static const int              maxTTL;

  static const char     * const fieldConnection;
  static const char     * const fieldContentLength;
  static const char     * const fieldContentType;
  static const char     * const fieldDate;
  static const char     * const fieldHost;
  static const char     * const fieldServer;
  static const char     * const fieldUserAgent;
  static const char     * const dateFormat;
};


class SHttpServerEngine : public QObject,
                          public SHttpEngine
{
Q_OBJECT
public:
  enum SocketOp               { SocketOp_Close, SocketOp_LeaveOpen };

  struct Callback
  {
    virtual SocketOp            handleHttpRequest(const RequestHeader &, QIODevice *) = 0;
  };

public:
  explicit                      SHttpServerEngine(const QString &protocol, QObject * = NULL);
  virtual                       ~SHttpServerEngine();

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

  QThreadPool                 * threadPool(void);

  virtual const char          * senderType(void) const;
  virtual const QString       & senderId(void) const;

public:
  static SocketOp               sendResponse(const RequestHeader &, QIODevice *, Status, const QByteArray &content, const QObject * = NULL);
  static SocketOp               sendResponse(const RequestHeader &, QIODevice *, Status, const QObject * = NULL);
  static SocketOp               sendRedirect(const RequestHeader &, QIODevice *, const QString &);

protected:
  QReadWriteLock              * lock(void) const;
  bool                          handleConnection(quintptr);

  virtual QIODevice           * openSocket(quintptr, int timeout) = 0;
  virtual void                  closeSocket(QIODevice *, bool canReuse, int timeout) = 0;

private:
  class SocketHandler;
  struct Private;
  Private               * const p;
};


class SHttpClientEngine : public QObject,
                          public SHttpEngine
{
Q_OBJECT
public:
  explicit                      SHttpClientEngine(QObject * = NULL);
  virtual                       ~SHttpClientEngine();

  virtual const char          * senderType(void) const;
  virtual const QString       & senderId(void) const;

  ResponseMessage               sendRequest(const RequestMessage &, int timeout = maxTTL);
  QByteArray                    sendRequest(const QUrl &, int timeout = maxTTL);

  QIODevice                   * openRequest(const RequestHeader &, int timeout = maxTTL);
  void                          closeRequest(QIODevice *, int timeout = maxTTL);

protected:
  virtual QIODevice           * openSocket(const QString &host, int timeout) = 0;
  virtual void                  closeSocket(QIODevice *, bool canReuse, int timeout) = 0;

private:
  class SocketHandler;
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
