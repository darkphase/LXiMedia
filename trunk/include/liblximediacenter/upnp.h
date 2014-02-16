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

#ifndef LXIMEDIACENTER_UPNP_H
#define LXIMEDIACENTER_UPNP_H

#include <QtCore>
#include "export.h"

namespace LXiMediaCenter {

enum HttpStatus
{
  HttpStatus_Ok                   = 200,
  HttpStatus_NotFound             = 404,
  HttpStatus_InternalServerError  = 500
};

class LXIMEDIACENTER_PUBLIC UPnP : public QObject
{
Q_OBJECT
public:
  struct LXIMEDIACENTER_PUBLIC HttpRequestInfo
  {
    QByteArray                host;
    QByteArray                userAgent;
    QByteArray                sourceAddress;
  };

  struct LXIMEDIACENTER_PUBLIC HttpCallback
  {
    virtual HttpStatus          httpRequest(const QUrl &request, const HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response) = 0;
  };

public:
  explicit                      UPnP(QObject *parent = NULL);
  virtual                       ~UPnP();

  static QString                hostname();

  void                          registerHttpCallback(const QString &path, HttpCallback *);
  void                          unregisterHttpCallback(HttpCallback *);

  virtual bool                  initialize(quint16 port, bool bindPublicInterfaces = false);
  virtual void                  close(void);

  bool                          isMyAddress(const QByteArray &) const;

  HttpStatus                    handleHttpRequest(const QUrl &path, const HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response);

protected:
  virtual void                  customEvent(QEvent *);

private slots:
  void                          clearResponses();
  void                          closedFile(QObject *);
  void                          updateInterfaces();

protected:
  static bool                   isLocalAddress(const char *);

  struct LXIMEDIACENTER_PUBLIC Functor { virtual ~Functor() { } virtual void operator()() = 0; };
  void                          send(Functor &) const;

private:
  void                          enableWebserver(void);
  HttpStatus                    getResponse(const QByteArray &host, const QByteArray &path, const QByteArray &userAgent, const QByteArray &sourceAddress, QByteArray &contentType, QIODevice *&response, bool erase);

public:
  static const char           * toMimeType(const QString &fileName);
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

private:
  struct Data;
  Data                 * const d;
};

} // End of namespace

#endif
