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

#include "connection_manager.h"
#include <cstring>

namespace lximediacenter {

const char connection_manager::service_id[]   = "urn:upnp-org:serviceId:ConnectionManager";
const char connection_manager::service_type[] = "urn:schemas-upnp-org:service:ConnectionManager:1";

connection_manager::connection_manager(class messageloop &messageloop, class rootdevice &rootdevice)
  : messageloop(messageloop),
    rootdevice(rootdevice),
    connection_id_counter(0)
{
  rootdevice.service_register(service_id, *this);
}

connection_manager::~connection_manager()
{
  rootdevice.service_unregister(service_id);
}

void connection_manager::set_protocols(const std::vector<protocol> &source_protocols, const std::vector<protocol> &sink_protocols)
{
  source_protocol_list = source_protocols;
  sink_protocol_list = sink_protocols;

  messageloop.post([this] { rootdevice.emit_event(service_id); });
}

const std::vector<connection_manager::protocol> & connection_manager::source_protocols() const
{
  return source_protocol_list;
}

const char * connection_manager::get_service_type(void)
{
  return service_type;
}

void connection_manager::initialize(void)
{
}

void connection_manager::close(void)
{
  connections.clear();

  for (auto &i : numconnections_changed) if (i.second) i.second(0);
}

void connection_manager::write_service_description(rootdevice::service_description &desc) const
{
  {
    static const char * const argname[] = { "ConnectionIDs"         };
    static const char * const argdir[]  = { "out"                   };
    static const char * const argvar[]  = { "CurrentConnectionIDs"  };
    desc.add_action("GetCurrentConnectionIDs", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "ConnectionID"            , "RcsID"           , "AVTransportID"           , "ProtocolInfo"            , "PeerConnectionManager"       , "PeerConnectionID"        , "Direction"           , "Status"                      };
    static const char * const argdir[]  = { "in"                      , "out"             , "out"                     , "out"                     , "out"                         , "out"                     , "out"                 , "out"                         };
    static const char * const argvar[]  = { "A_ARG_TYPE_ConnectionID" , "A_ARG_TYPE_RcsID", "A_ARG_TYPE_AVTransportID", "A_ARG_TYPE_ProtocolInfo" , "A_ARG_TYPE_connection_manager", "A_ARG_TYPE_ConnectionID" , "A_ARG_TYPE_Direction", "A_ARG_TYPE_ConnectionStatus" };
    desc.add_action("GetCurrentConnectionInfo", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "Source"            , "Sink"              };
    static const char * const argdir[]  = { "out"               , "out"               };
    static const char * const argvar[]  = { "SourceProtocolInfo", "SinkProtocolInfo"  };
    desc.add_action("GetProtocolInfo", argname, argdir, argvar);
  }

  desc.add_statevariable("A_ARG_TYPE_ProtocolInfo"      , "string", false );
  static const char * const connection_status_values[] = { "OK", "ContentFormatMismatch", "InsufficientBandwidth", "UnreliableChannel", "Unknown" };
  desc.add_statevariable("A_ARG_TYPE_ConnectionStatus"  , "string", false, connection_status_values);
  desc.add_statevariable("A_ARG_TYPE_AVTransportID"     , "i4"    , false );
  desc.add_statevariable("A_ARG_TYPE_RcsID"             , "i4"    , false );
  desc.add_statevariable("A_ARG_TYPE_ConnectionID"      , "i4"    , false );
  desc.add_statevariable("A_ARG_TYPE_connection_manager", "string", false );
  desc.add_statevariable("SourceProtocolInfo"           , "string", true  );
  desc.add_statevariable("SinkProtocolInfo"             , "string", true  );
  static const char * const direction_values[] = { "Input", "Output" };
  desc.add_statevariable("A_ARG_TYPE_Direction"         , "string", false, direction_values);
  desc.add_statevariable("CurrentConnectionIDs"         , "string", true  );
}

void connection_manager::write_eventable_statevariables(rootdevice::eventable_propertyset &propset) const
{
  std::string sp;
  for (auto &protocol : source_protocol_list)
    sp += "," + protocol.to_string(true);

  propset.add_property("SourceProtocolInfo", sp.empty() ? sp : sp.substr(1));

  sp.clear();
  for (auto &protocol : sink_protocol_list)
    sp += "," + protocol.to_string(true);

  propset.add_property("SinkProtocolInfo", sp.empty() ? sp : sp.substr(1));

  propset.add_property("CurrentConnectionIDs", "");
}

void connection_manager::output_connection_add(const std::string &content_type, const std::shared_ptr<std::istream> &stream)
{
  const auto id = ++connection_id_counter;

  connection_info connection;
  connection.rcs_id = -1;
  connection.avtransport_id = -1;
  connection.protocol_info = "http-get:*:" + content_type + ":*";
  connection.peerconnection_manager = std::string();
  connection.peerconnection_id = -1;
  connection.direction = connection_info::output;
  connection.status = connection_info::ok;
  connections[id] = connection;
  streams[stream.get()] = id;

  messageloop.post([this] { rootdevice.emit_event(service_id); });
  for (auto &i : numconnections_changed) if (i.second) i.second(connections.size());
}

void connection_manager::output_connection_remove(const std::shared_ptr<std::istream> &stream)
{
  auto i = streams.find(stream.get());
  if (i != streams.end())
  {
    auto j = connections.find(i->second);
    if (j != connections.end())
    {
      connections.erase(j);

      messageloop.post([this] { rootdevice.emit_event(service_id); });
      for (auto &i : numconnections_changed) if (i.second) i.second(connections.size());
    }

    streams.erase(i);
  }
}

void connection_manager::handle_action(const upnp::request &, action_get_current_connectionids &action)
{
  std::vector<int32_t> ids;
  ids.reserve(connections.size());
  for (auto &i : connections)
    ids.push_back(i.first);

  action.set_response(ids);
}

void connection_manager::handle_action(const upnp::request &, action_get_current_connection_info &action)
{
  auto i = connections.find(action.get_connectionid());
  if (i != connections.end())
    action.set_response(i->second);
}

void connection_manager::handle_action(const upnp::request &, action_get_protocol_info &action)
{
  std::string source_protocols;
  for (auto &protocol : source_protocol_list)
    source_protocols += "," + protocol.to_string(true);

  source_protocols = source_protocols.empty() ? source_protocols : source_protocols.substr(1);

  std::string sink_protocols;
  for (auto &protocol : sink_protocol_list)
    sink_protocols += "," + protocol.to_string(true);

  sink_protocols = sink_protocols.empty() ? sink_protocols : sink_protocols.substr(1);

  action.set_response(source_protocols, sink_protocols);
}

connection_manager::protocol::protocol(
    const std::string &network_protocol,
    const std::string &content_format,
    bool conversion_indicator,
    bool operations_range,
    bool operations_timeseek,
    const std::string &profile,
    const std::string &suffix,
    unsigned sample_rate, unsigned channels,
    unsigned resolution_x, unsigned resolution_y,
    uint64_t size)
  : network_protocol(network_protocol), network("*"), content_format(content_format),
    profile(profile), play_speed(true), conversion_indicator(conversion_indicator),
    operations_range(operations_range), operations_timeseek(operations_timeseek),
    flags(starts_with(content_format, "image/") ? "00100000" : "01700000"),
    suffix(suffix), sample_rate(sample_rate), channels(channels),
    resolution_x(resolution_x), resolution_y(resolution_y), size(size)
{
}

std::string connection_manager::protocol::to_string(bool brief) const
{
  std::string result = network_protocol + ":" + network + ":" + content_format + ":";

  if (!profile.empty())
    result += brief ? ("DLNA.ORG_PN=" + profile) : content_features();
  else
    result += "*";

  return result;
}

std::string connection_manager::protocol::content_features(void) const
{
  std::string result;

  if (!profile.empty())
    result += "DLNA.ORG_PN=" + profile + ";";

  if (starts_with(content_format, "image/"))
  {
    result += "DLNA.ORG_PS=" + std::to_string(play_speed ? 1 : 0) + ";" +
              "DLNA.ORG_OP=" + std::to_string(operations_timeseek ? 1 : 0) +
                               std::to_string(operations_range ? 1 : 0) + ";";
  }

  result +=
      "DLNA.ORG_CI=" + std::to_string(conversion_indicator ? 1 : 0);

  if (!flags.empty())
    result += ";DLNA.ORG_FLAGS=" + flags + "000000000000000000000000";

  return result;
}

connection_manager::connection_info::connection_info()
  : rcs_id(0),
    avtransport_id(0),
    peerconnection_id(0),
    direction(output),
    status(unknown)
{
}

connection_manager::connection_info::~connection_info()
{
}

} // End of namespace
