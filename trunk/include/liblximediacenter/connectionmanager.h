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

#ifndef LXIMEDIACENTER_CONNECTIONMAMANGER_H
#define LXIMEDIACENTER_CONNECTIONMAMANGER_H

#include <QtCore>
#include "export.h"
#include "rootdevice.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC ConnectionManager : public QObject,
                                                public RootDevice::Service
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

  struct LXIMEDIACENTER_PUBLIC ConnectionInfo
  {
                                ConnectionInfo();
                                ~ConnectionInfo();

    qint32                      rcsID;
    qint32                      avTransportID;
    QByteArray                  protocolInfo;
    QByteArray                  peerConnectionManager;
    qint32                      peerConnectionID;
    enum { Input, Output }      direction;
    enum { OK, ContentFormatMismatch, InsufficientBandwidth, UnreliableChannel, Unknown } status;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetCurrentConnectionIDs
  {
    virtual void                setResponse(const QList<qint32> &) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetCurrentConnectionInfo
  {
    virtual qint32              getConnectionID() const = 0;

    virtual void                setResponse(const ConnectionInfo &info) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetProtocolInfo
  {
    virtual void                setResponse(const QByteArray &source, const QByteArray &sink) = 0;
  };

public:
  explicit                      ConnectionManager(RootDevice *parent);
  virtual                       ~ConnectionManager();

  void                          setProtocols(const ProtocolList &sourceProtocols, const ProtocolList &sinkProtocols);
  const ProtocolList          & sourceProtocols() const;

  void                          addOutputConnection(const QUrl &, const QByteArray &, const QIODevice *);

  void                          handleAction(const QByteArray &, ActionGetCurrentConnectionIDs &);
  void                          handleAction(const QByteArray &, ActionGetCurrentConnectionInfo &);
  void                          handleAction(const QByteArray &, ActionGetProtocolInfo &);

protected: // From RootDevice::Service
  virtual const char          * serviceType(void);

  virtual void                  initialize(void);
  virtual void                  close(void);

  virtual void                  writeServiceDescription(RootDevice::ServiceDescription &) const;
  virtual void                  writeEventableStateVariables(RootDevice::EventablePropertySet &) const;

protected:
  virtual void                  customEvent(QEvent *);

private slots:
  void                          connectionClosed(QObject *);

protected:
  static const char             serviceId[];

private:
  RootDevice            * const parent;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
