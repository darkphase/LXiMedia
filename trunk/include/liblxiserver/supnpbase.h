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
    inline Protocol(const QString &protocol = "http-get",
                    const QString &contentFormat = "",
                    bool conversionIndicator = false,
                    const QString &profile = "",
                    const QString &suffix = "")
      : protocol(protocol), network("*"), contentFormat(contentFormat),
        profile(profile), playSpeed(true), conversionIndicator(conversionIndicator),
        operationsRange(false), operationsTimeSeek(false),
        flags("01700000000000000000000000000000"), suffix(suffix)
    {
    }

    inline Protocol(const QString &protocol,
                    const QString &contentFormat,
                    bool conversionIndicator,
                    const QString &profile,
                    const QString &suffix,
                    const QMap<QString, QString> &queryItems)
      : protocol(protocol), network("*"), contentFormat(contentFormat),
        profile(profile), playSpeed(true), conversionIndicator(conversionIndicator),
        operationsRange(false), operationsTimeSeek(false),
        flags("01700000000000000000000000000000"), suffix(suffix), queryItems(queryItems)
    {
    }

    QString                     toString(bool brief = false) const;             //!< Returns the DLNA protocol string.
    QString                     contentFeatures(void) const;                    //!< Returns the DLNA contentFeatures string.

    QString                     protocol;                                       //!< The network protocol used (e.g. "http-get").
    QString                     network;                                        //!< The network used, usually not needed.
    QString                     contentFormat;                                  //!< The content format used with the protocol (e.g. MIME-type for "http-get").

    QString                     profile;                                        //!< The profile name of the protocol (e.g. "DLNA.ORG_PN=JPEG_TN").
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
    QString                     flags;

    QString                     suffix;                                         //!< The file extension (including .).
    QMap<QString, QString>      queryItems;                                     //!< Query items that are added to the URL.
  };

  typedef QList<Protocol>       ProtocolList;

  enum ProtocolType
  {
    ProtocolType_None         = 0,
    ProtocolType_Audio,
    ProtocolType_Video,
    ProtocolType_Image
  };

public:
  explicit                      SUPnPBase(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPBase();

  void                          initialize(SHttpServer *, SUPnPMediaServer::Service &);
  void                          close(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

protected:
  virtual SHttpServer::ResponseMessage handleControl(const SHttpServer::RequestMessage &, QIODevice *);
  virtual SHttpServer::ResponseMessage handleDescription(const SHttpServer::RequestMessage &);

  virtual void                  buildDescription(QDomDocument &, QDomElement &) = 0;
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &) = 0;

protected:
  const QString               & basePath(void) const;
  SHttpServer                 * httpServer(void) const;

public:
  static QString                protocol(void);

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
  static const char             xmlContentType[];
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
