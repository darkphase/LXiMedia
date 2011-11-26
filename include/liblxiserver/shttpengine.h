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
#include <QtNetwork>
#include <LXiCore>
#include "export.h"

namespace LXiServer {

/*! This class provides some basics for HTTP clients and servers.
 */
class LXISERVER_PUBLIC SHttpEngine : public QObject
{
Q_OBJECT
public:
  /*! The HTTP status codes.
   */
  enum StatusCode
  {
    Status_None               = 0,

    Status_Continue           = 100,
    Status_SwitchingProtocols = 101,

    Status_Ok                 = 200,
    Status_Created            = 201,
    Status_Accepted           = 202,
    Status_NonAuthInformation = 203,
    Status_NoContent          = 204,
    Status_ResetContent       = 205,
    Status_PartialContent     = 206,

    Status_MultipleChoices    = 300,
    Status_MovedPermanently   = 301,

    Status_Found              = 302,
    Status_SeeOther           = 303,
    Status_NotModified        = 304,
    Status_UseProxy           = 305,
    Status_TemporaryRedirect  = 307,

    Status_BadRequest         = 400,
    Status_Unauthorized       = 401,
    Status_PaymentRequired    = 402,
    Status_Forbidden          = 403,
    Status_NotFound           = 404,
    Status_MethodNotAllowed   = 405,
    Status_NotAcceptable      = 406,
    Status_ProxyAuthRequired  = 407,
    Status_RequestTimeout     = 408,
    Status_Conflict           = 409,
    Status_Gone               = 410,
    Status_LengthRequired     = 411,
    Status_PreconditionFailed = 412,
    Status_RequestEntTooLarge = 413,
    Status_RequestURITooLarge = 414,
    Status_UnsuppMediaType    = 415,
    Status_ReqRangeNotSatisf  = 416,
    Status_ExpectationFailed  = 417,

    Status_InternalServerError= 500,
    Status_NotImplemented     = 501,
    Status_BadGateway         = 502,
    Status_ServiceUnavailable = 503,
    Status_GatewayTimeout     = 504,
    Status_HTTPVersionNotSupp = 505
  };

  /*! This class provides a standard HTTP header.
   */
  class LXISERVER_PUBLIC Header
  {
  protected:
    explicit                    Header(const SHttpEngine *);

  public:
    bool                        isValid(void) const;

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
    inline void                 setHost(const QString &host)                    { setField(fieldHost, host); }
    void                        setHost(const QString &hostname, quint16);
    void                        setHost(const QHostAddress &, quint16 = 0);
    inline QString              server(void) const                              { return field(fieldServer); }
    inline void                 setServer(const QString &server)                { setField(fieldServer, server); }
    inline QString              userAgent(void) const                           { return field(fieldUserAgent); }
    inline void                 setUserAgent(const QString &userAgent)          { setField(fieldUserAgent, userAgent); }

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }
    void                        parse(const QByteArray &);

  public:
    const SHttpEngine         * httpEngine;

  protected:
    QList<QByteArray>           head;

  private:
    QList< QPair<QString, QString> > fields;
  };

  /*! This class provides a standard HTTP request header.
   */
  class LXISERVER_PUBLIC RequestHeader : public Header
  {
  public:
    explicit                    RequestHeader(const SHttpEngine *);

    bool                        isGet(void) const;                              //!< returns true if the method is GET or HEAD.
    bool                        isHead(void) const;                             //!< returns true if the method is HEAD.
    bool                        isPost(void) const;                             //!< returns true if the method is POST.

    inline QByteArray           method(void) const                              { return isValid() ? head[0] : QByteArray(); }
    inline void                 setMethod(const QByteArray &method)             { if (head.size() > 0) head[0] = method; else head.append(method); }
    inline QByteArray           path(void) const                                { return isValid() ? head[1] : QByteArray(); }
    inline void                 setPath(const QByteArray &path)                 { if (head.size() > 1) head[1] = path; else head.append(path); }
    inline QByteArray           version(void) const                             { return isValid() ? head[2] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 2) head[2] = version; else head.append(version); }

    inline void                 setRequest(const QByteArray &method, const QByteArray &path) { setMethod(method); setPath(path);  }
    inline void                 setRequest(const QByteArray &method, const QByteArray &path, const QByteArray &version) { setMethod(method); setPath(path); setVersion(version); }

    QString                     file(void) const;
  };

  class LXISERVER_PUBLIC Status
  {
  public:
    inline                      Status(StatusCode statusCode) : code(statusCode), desc(errorDescription(statusCode)) { }
    inline                      Status(int statusCode, const QByteArray &description) : code(statusCode), desc(description) { }

    inline bool                 operator==(StatusCode statusCode) const         { return code == statusCode; }
    inline bool                 operator!=(StatusCode statusCode) const         { return code != statusCode; }

    inline int                  statusCode(void) const                          { return code; }
    inline const QByteArray   & description(void) const                         { return desc; }

  private:
    int                         code;
    QByteArray                  desc;
  };

  /*! This class provides a standard HTTP response header.
   */
  class LXISERVER_PUBLIC ResponseHeader : public Header
  {
  public:
    explicit                    ResponseHeader(const SHttpEngine *);
    explicit                    ResponseHeader(const RequestHeader &);
                                ResponseHeader(const RequestHeader &, const Status &);

    inline QByteArray           version(void) const                             { return isValid() ? head[0] : QByteArray(); }
    inline void                 setVersion(const QByteArray &version)           { if (head.size() > 0) head[0] = version; else head.append(version); }
    inline Status               status(void) const                              { return isValid() ? Status(head[1].toInt(), head[2]) : Status(Status_None); }
    void                        setStatus(const Status &status);

    inline void                 setResponse(Status status)                      { setStatus(status); }
    inline void                 setResponse(Status status, const QByteArray &version) { setStatus(status); setVersion(version); }

    void                        parse(const QByteArray &);
  };

  /*! This class provides a standard HTTP request message.
   */
  class LXISERVER_PUBLIC RequestMessage : public RequestHeader
  {
  public:
    explicit                    RequestMessage(const SHttpEngine * = NULL);

    bool                        isComplete(void) const;

    inline const QByteArray   & content(void) const                             { return data; }
    void                        setContent(const QByteArray &content);

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }
    void                        parse(const QByteArray &);

  private:
    QByteArray                  data;
  };

  /*! This class provides a standard HTTP response message.
   */
  class LXISERVER_PUBLIC ResponseMessage : public ResponseHeader
  {
  public:
    explicit                    ResponseMessage(const SHttpEngine *);
    explicit                    ResponseMessage(const RequestHeader &request);
                                ResponseMessage(const RequestHeader &request, Status status);
                                ResponseMessage(const RequestHeader &request, Status status, const QByteArray &content, const QString &contentType);

    bool                        isComplete(void) const;

    inline const QByteArray   & content(void) const                             { return data; }
    void                        setContent(const QByteArray &content);

    QByteArray                  toByteArray(void) const;
    inline                      operator QByteArray() const                     { return toByteArray(); }
    void                        parse(const QByteArray &);

  private:
    QByteArray                  data;
  };

public:
                                SHttpEngine(QObject * = NULL);
  virtual                       ~SHttpEngine();

  virtual const char          * senderType(void) const = 0;
  virtual const QString       & senderId(void) const = 0;

  static const char           * errorDescription(StatusCode);
  static const char           * toMimeType(const QString &fileName);
  static bool                   splitHost(const QString &host, QString &hostname, quint16 &port);

  static void                   closeSocket(QIODevice *);

public:
  static const char             httpVersion[];
  static const int              maxTTL;

  static const char             fieldConnection[];
  static const char             fieldContentLength[];
  static const char             fieldContentType[];
  static const char             fieldDate[];
  static const char             fieldHost[];
  static const char             fieldServer[];
  static const char             fieldUserAgent[];
  static const char             dateFormat[];

  static const char             mimeAppOctet[];
  static const char             mimeAudioAac[];
  static const char             mimeAudioAc3[];
  static const char             mimeAudioLpcm[];
  static const char             mimeAudioMp3[];
  static const char             mimeAudioMpeg[];
  static const char             mimeAudioMpegUrl[];
  static const char             mimeAudioOgg[];
  static const char             mimeAudioWave[];
  static const char             mimeAudioWma[];
  static const char             mimeImageJpeg[];
  static const char             mimeImagePng[];
  static const char             mimeImageSvg[];
  static const char             mimeImageTiff[];
  static const char             mimeVideo3g2[];
  static const char             mimeVideoAsf[];
  static const char             mimeVideoAvi[];
  static const char             mimeVideoFlv[];
  static const char             mimeVideoMatroska[];
  static const char             mimeVideoMpeg[];
  static const char             mimeVideoMpegM2TS[];
  static const char             mimeVideoMpegTS[];
  static const char             mimeVideoMp4[];
  static const char             mimeVideoOgg[];
  static const char             mimeVideoQt[];
  static const char             mimeVideoWmv[];
  static const char             mimeTextCss[];
  static const char             mimeTextHtml[];
  static const char             mimeTextJs[];
  static const char             mimeTextPlain[];
  static const char             mimeTextXml[];
};


/*! This class provides the basics for a HTTP server.
 */
class LXISERVER_PUBLIC SHttpServerEngine : public SHttpEngine
{
Q_OBJECT
public:
  struct LXISERVER_PUBLIC Callback
  {
    virtual ResponseMessage     httpRequest(const RequestMessage &, QIODevice *) = 0;
    virtual ResponseMessage     httpOptions(const RequestMessage &);
  };

public:
  explicit                      SHttpServerEngine(const QString &protocol, QObject * = NULL);
  virtual                       ~SHttpServerEngine();

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

  virtual const char          * senderType(void) const;
  virtual const QString       & senderId(void) const;

  ResponseMessage               handleHttpRequest(const RequestMessage &, QIODevice *) const;
  static bool                   sendHttpResponse(const RequestHeader &, ResponseMessage &, QIODevice *, bool reuse = true);
  
private:
  struct Data;
  Data                  * const d;
};


/*! This class provides the basics for a HTTP client.
 */
class LXISERVER_PUBLIC SHttpClientEngine : public SHttpEngine
{
Q_OBJECT
public:
  explicit                      SHttpClientEngine(QObject * = NULL);
  virtual                       ~SHttpClientEngine();

  virtual const char          * senderType(void) const;
  virtual const QString       & senderId(void) const;

  /*! This sends a request message to the server specified by the host in the
      message. After the connection has been established and the message has been
      sent, the provided slot is invoked with the opened socket (QIODevice *) as
      the first argument.
   */
  virtual void                  openRequest(const RequestMessage &message, QObject *receiver, const char *slot) = 0;

  void                          sendRequest(const RequestMessage &);

  /*! Sends a request to the host mentioned in the RequestMessage and receives a
      result message.
      \param  request           The request message to send.
      \param  timeout           The timeout in millicesonds.
      \returns                  A ResponseMessage containing the response.
      \note This method blocks until a response has been received or the timeout
            expires.
   */
  virtual ResponseMessage       blockingRequest(const RequestMessage &, int timeout = 30000) = 0;

signals:
  void                          response(const SHttpEngine::ResponseMessage &);
  void                          opened(QIODevice *);

protected:
  virtual void                  customEvent(QEvent *);

  int                           socketsAvailable(void) const;
  virtual void                  socketCreated(void);
  virtual void                  socketDestroyed(void);

protected slots:
  virtual void                  handleResponse(const SHttpEngine::ResponseMessage &);

protected:
  static const QEvent::Type     socketCreatedEventType;
  static const QEvent::Type     socketDestroyedEventType;

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
