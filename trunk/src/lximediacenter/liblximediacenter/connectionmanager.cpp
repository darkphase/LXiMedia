/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "connectionmanager.h"
#include "mediaserver.h"

namespace LXiMediaCenter {

const char  ConnectionManager::serviceId[]       = "urn:upnp-org:serviceId:ConnectionManager";

struct ConnectionManager::Data
{
  static const QEvent::Type     emitUpdateEventType;

  ProtocolList                  sourceProtocols;
  ProtocolList                  sinkProtocols;

  qint32                        connectionIdCounter;
  QMap<qint32, ConnectionInfo>  connections;
  QMap<const QObject *, qint32> ioDevices;
};

const QEvent::Type  ConnectionManager::Data::emitUpdateEventType = QEvent::Type(QEvent::registerEventType());

ConnectionManager::ConnectionManager(RootDevice *parent)
  : QObject(parent),
    parent(parent),
    d(new Data())
{
  d->connectionIdCounter = 0;

  parent->registerService(serviceId, this);
}

ConnectionManager::~ConnectionManager()
{
  parent->unregisterService(serviceId);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void ConnectionManager::setProtocols(const ProtocolList &sourceProtocols, const ProtocolList &sinkProtocols)
{
  d->sourceProtocols = sourceProtocols;
  d->sinkProtocols = sinkProtocols;

  qApp->postEvent(this, new QEvent(d->emitUpdateEventType), Qt::LowEventPriority);
}

const ConnectionManager::ProtocolList & ConnectionManager::sourceProtocols() const
{
  return d->sourceProtocols;
}

const char * ConnectionManager::serviceType(void)
{
  return RootDevice::serviceTypeConnectionManager;
}

void ConnectionManager::initialize(void)
{
}

void ConnectionManager::close(void)
{
  d->connections.clear();
  d->ioDevices.clear();
}

void ConnectionManager::writeServiceDescription(RootDevice::ServiceDescription &desc) const
{
  {
    static const char * const argname[] = { "ConnectionIDs"         };
    static const char * const argdir[]  = { "out"                   };
    static const char * const argvar[]  = { "CurrentConnectionIDs"  };
    desc.addAction("GetCurrentConnectionIDs", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "ConnectionID"            , "RcsID"           , "AVTransportID"           , "ProtocolInfo"            , "PeerConnectionManager"       , "PeerConnectionID"        , "Direction"           , "Status"                      };
    static const char * const argdir[]  = { "in"                      , "out"             , "out"                     , "out"                     , "out"                         , "out"                     , "out"                 , "out"                         };
    static const char * const argvar[]  = { "A_ARG_TYPE_ConnectionID" , "A_ARG_TYPE_RcsID", "A_ARG_TYPE_AVTransportID", "A_ARG_TYPE_ProtocolInfo" , "A_ARG_TYPE_ConnectionManager", "A_ARG_TYPE_ConnectionID" , "A_ARG_TYPE_Direction", "A_ARG_TYPE_ConnectionStatus" };
    desc.addAction("GetCurrentConnectionInfo", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "Source"            , "Sink"              };
    static const char * const argdir[]  = { "out"               , "out"               };
    static const char * const argvar[]  = { "SourceProtocolInfo", "SinkProtocolInfo"  };
    desc.addAction("GetProtocolInfo", argname, argdir, argvar);
  }

  desc.addStateVariable("A_ARG_TYPE_ProtocolInfo"      , "string", false );
  static const char * const connectionStatusValues[] = { "OK", "ContentFormatMismatch", "InsufficientBandwidth", "UnreliableChannel", "Unknown" };
  desc.addStateVariable("A_ARG_TYPE_ConnectionStatus"  , "string", false, connectionStatusValues);
  desc.addStateVariable("A_ARG_TYPE_AVTransportID"     , "i4"    , false );
  desc.addStateVariable("A_ARG_TYPE_RcsID"             , "i4"    , false );
  desc.addStateVariable("A_ARG_TYPE_ConnectionID"      , "i4"    , false );
  desc.addStateVariable("A_ARG_TYPE_ConnectionManager" , "string", false );
  desc.addStateVariable("SourceProtocolInfo"           , "string", true  );
  desc.addStateVariable("SinkProtocolInfo"             , "string", true  );
  static const char * const directionValues[] = { "Input", "Output" };
  desc.addStateVariable("A_ARG_TYPE_Direction"         , "string", false, directionValues);
  desc.addStateVariable("CurrentConnectionIDs"         , "string", true  );
}

void ConnectionManager::writeEventableStateVariables(RootDevice::EventablePropertySet &propset) const
{
  QByteArray sp;
  foreach (const Protocol &protocol, d->sourceProtocols)
    sp += "," + protocol.toByteArray(true);

  propset.addProperty("SourceProtocolInfo", sp.isEmpty() ? sp : sp.mid(1));

  sp.clear();
  foreach (const Protocol &protocol, d->sinkProtocols)
    sp += "," + protocol.toByteArray(true);

  propset.addProperty("SinkProtocolInfo", sp.isEmpty() ? sp : sp.mid(1));

  propset.addProperty("CurrentConnectionIDs", "");
}

void ConnectionManager::addOutputConnection(const QUrl &url, const QByteArray &contentType, const QIODevice *ioDevice)
{
  const qint32 id = ++(d->connectionIdCounter);

  ConnectionInfo connection;
  connection.rcsID = -1;
  connection.avTransportID = -1;
  connection.protocolInfo = (url.scheme() == "http") ? ("http-get:*:" + contentType + ":*") : QByteArray();
  connection.peerConnectionManager = QByteArray();
  connection.peerConnectionID = -1;
  connection.direction = ConnectionInfo::Output;
  connection.status = ConnectionInfo::OK;
  d->connections[id] = connection;
  d->ioDevices[ioDevice] = id;

  connect(ioDevice, SIGNAL(destroyed(QObject*)), this, SLOT(connectionClosed(QObject*)));

  qApp->postEvent(this, new QEvent(d->emitUpdateEventType), Qt::LowEventPriority);
}

void ConnectionManager::customEvent(QEvent *e)
{
  if (e->type() == d->emitUpdateEventType)
    parent->emitEvent(serviceId);
  else
    QObject::customEvent(e);
}

void ConnectionManager::connectionClosed(QObject *ioDevice)
{
  QMap<const QObject *, qint32>::Iterator i = d->ioDevices.find(ioDevice);
  if (i != d->ioDevices.end())
  {
    QMap<qint32, ConnectionInfo>::Iterator j = d->connections.find(*i);
    if (j != d->connections.end())
    {
      d->connections.erase(j);

      qApp->postEvent(this, new QEvent(d->emitUpdateEventType), Qt::LowEventPriority);
    }

    d->ioDevices.erase(i);
  }
}

void ConnectionManager::handleAction(const UPnP::HttpRequestInfo &, ActionGetCurrentConnectionIDs &action)
{
  action.setResponse(d->connections.keys());
}

void ConnectionManager::handleAction(const UPnP::HttpRequestInfo &, ActionGetCurrentConnectionInfo &action)
{
  QMap<qint32, ConnectionInfo>::Iterator i = d->connections.find(action.getConnectionID());
  if (i != d->connections.end())
    action.setResponse(*i);
}

void ConnectionManager::handleAction(const UPnP::HttpRequestInfo &, ActionGetProtocolInfo &action)
{
  QByteArray sourceProtocols;
  foreach (const Protocol &protocol, d->sourceProtocols)
    sourceProtocols += "," + protocol.toByteArray(true);

  sourceProtocols = sourceProtocols.isEmpty() ? sourceProtocols : sourceProtocols.mid(1);

  QByteArray sinkProtocols;
  foreach (const Protocol &protocol, d->sinkProtocols)
    sinkProtocols += "," + protocol.toByteArray(true);

  sinkProtocols = sinkProtocols.isEmpty() ? sinkProtocols : sinkProtocols.mid(1);

  action.setResponse(sourceProtocols, sinkProtocols);
}

QByteArray ConnectionManager::Protocol::toByteArray(bool brief) const
{
  QByteArray result = protocol + ":" + network + ":" + contentFormat + ":";

  if (!profile.isEmpty())
    result += brief ? ("DLNA.ORG_PN=" + profile) : contentFeatures();
  else
    result += "*";

  return result;
}

QByteArray ConnectionManager::Protocol::contentFeatures(void) const
{
  QByteArray result;

  if (!profile.isEmpty())
    result += "DLNA.ORG_PN=" + profile + ";";

  if (!contentFormat.startsWith("image/"))
  {
    result += "DLNA.ORG_PS=" + QByteArray::number(playSpeed ? 1 : 0) + ";" +
              "DLNA.ORG_OP=" + QByteArray::number(operationsTimeSeek ? 1 : 0) +
                               QByteArray::number(operationsRange ? 1 : 0) + ";";
  }

  result +=
      "DLNA.ORG_CI=" + QByteArray::number(conversionIndicator ? 1 : 0);

  if (!flags.isEmpty())
    result += ";DLNA.ORG_FLAGS=" + flags + "000000000000000000000000";

  return result;
}

ConnectionManager::ConnectionInfo::ConnectionInfo()
  : rcsID(0),
    avTransportID(0),
    peerConnectionID(0),
    direction(Output),
    status(Unknown)
{
}

ConnectionManager::ConnectionInfo::~ConnectionInfo()
{
}

} // End of namespace
