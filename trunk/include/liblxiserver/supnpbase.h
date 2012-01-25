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

#ifndef LXISERVER_SUPNPBASE_H
#define LXISERVER_SUPNPBASE_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include "shttpserver.h"
#include "supnpmediaserver.h"
#include "export.h"

namespace LXiServer {

class SUPnPMediaServer;

class LXISERVER_PUBLIC SUPnPBase : public QObject,
                                   protected SHttpServer::Callback
{
Q_OBJECT
public:
  struct Protocol
  {
    inline Protocol(const QByteArray &protocol = "http-get",
                    const QByteArray &contentFormat = "",
                    bool conversionIndicator = false,
                    bool operationsRange = false,
                    bool operationsTimeSeek = false,
                    const QByteArray &profile = "",
                    const QByteArray &suffix = "",
                    unsigned sampleRate = 0, unsigned channels = 0,
                    const QSize &resolution = QSize(),
                    quint64 size = 0)
      : protocol(protocol), network("*"), contentFormat(contentFormat),
        profile(profile), playSpeed(true), conversionIndicator(conversionIndicator),
        operationsRange(operationsRange), operationsTimeSeek(operationsTimeSeek),
        flags(contentFormat.startsWith("image/") ? "00100000" : "01700000"),
        suffix(suffix), sampleRate(sampleRate), channels(channels),
        resolution(resolution), size(size)
    {
    }

    QByteArray                  toByteArray(bool brief = false) const;          //!< Returns the DLNA protocol string.
    QByteArray                  contentFeatures(void) const;                    //!< Returns the DLNA contentFeatures string.

    QByteArray                  protocol;                                       //!< The network protocol used (e.g. "http-get").
    QByteArray                  network;                                        //!< The network used, usually not needed.
    QByteArray                  contentFormat;                                  //!< The content format used with the protocol (e.g. MIME-type for "http-get").

    QByteArray                  profile;                                        //!< The profile name of the protocol (e.g. "JPEG_TN").
    bool                        playSpeed;                                      //!< false = invalid play speed, true = normal play speed.
    bool                        conversionIndicator;                            //!< false = not transcoded, true = transcoded.
    bool                        operationsRange;                                //!< true = range supported.
    bool                        operationsTimeSeek;                             //!< true = time seek range supported.

    /*! DLNA.ORG_FLAGS, padded with 24 trailing 0s
           80000000  31  senderPaced
           40000000  30  lsopTimeBasedSeekSupported
           20000000  29  lsopByteBasedSeekSupported
           10000000  28  playcontainerSupported
            8000000  27  s0IncreasingSupported
            4000000  26  sNIncreasingSupported
            2000000  25  rtspPauseSupported
            1000000  24  streamingTransferModeSupported
             800000  23  interactiveTransferModeSupported
             400000  22  backgroundTransferModeSupported
             200000  21  connectionStallingSupported
             100000  20  dlnaVersion15Supported

           Example: (1 << 24) | (1 << 22) | (1 << 21) | (1 << 20)
             DLNA.ORG_FLAGS=01700000[000000000000000000000000] // [] show padding
     */
    QByteArray                  flags;

    QByteArray                  suffix;                                         //!< The file extension (including .).

    unsigned                    sampleRate, channels;
    QSize                       resolution;
    quint64                     size;

    QList< QPair<QString, QString> > queryItems;
  };

  typedef QList<Protocol>       ProtocolList;

public:
  explicit                      SUPnPBase(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPBase();

  void                          initialize(SHttpServer *, SUPnPMediaServer::Service &);
  void                          close(void);
  void                          reset(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

protected:
  virtual SHttpServer::ResponseMessage handleControl(const SHttpServer::RequestMessage &, QIODevice *);
  virtual SHttpServer::ResponseMessage handleDescription(const SHttpServer::RequestMessage &);

  virtual void                  buildDescription(QDomDocument &, QDomElement &) = 0;
  virtual SHttpServer::Status   handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &) = 0;

protected:
  const QString               & basePath(void) const;
  SHttpServer                 * httpServer(void) const;

public:
  static QString                protocol(void);
  static QString                toClientString(const QHostAddress &, const SHttpServer::RequestMessage &);

  static QDomElement            addTextElm(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &value);
  static QDomElement            addTextElmNS(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &nsUri, const QString &value);
  static void                   addSpecVersion(QDomDocument &doc, QDomElement &elm);
  static void                   addActionArgument(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &direction, const QString &relatedStateVariable);
  static void                   addStateVariable(QDomDocument &doc, QDomElement &elm, bool sendEvents, const QString &name, const QString &dataType, const QStringList &allowedValues = QStringList());

  static QDomElement            createElementNS(QDomDocument &doc, const QDomElement &nsElm, const QString &localName);
  static QDomElement            makeSoapMessage(QDomDocument &doc, const QDomElement &nsElm);
  static QByteArray             serializeSoapMessage(const QDomDocument &doc);

  static QDomElement            firstChildElementNS(const QDomElement &, const QString &nsURI, const QString &localName);
  static QDomElement            parseSoapMessage(QDomDocument &doc, const QByteArray &data);

public:
  static const int              majorVersion;
  static const int              minorVersion;
  static const int              responseTimeout;
  static const char             dlnaDoc[];
  static const char             xmlDeclaration[];
  static const char             dlnaNS[];
  static const char             didlNS[];
  static const char             dublinCoreNS[];
  static const char             metadataNS[];
  static const char             soapNS[];

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
