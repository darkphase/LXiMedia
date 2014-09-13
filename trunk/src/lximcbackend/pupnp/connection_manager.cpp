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
#include "platform/string.h"
#include <cmath>
#include <cstring>
#include <iostream>

namespace pupnp {

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

void connection_manager::add_source_audio_protocol(
        const char *name,
        const char *mime, const char *suffix,
        unsigned sample_rate, unsigned channels,
        const char *acodec, const char *mux)
{
    std::clog << "[" << this << "] connection_manager: enabled audio protocol "
              << name << " " << sample_rate << "/" << channels
              << std::endl;

    source_audio_protocol_list.emplace_back(protocol(
                                                "http-get", mime,
                                                true, false, false,
                                                name, suffix,
                                                sample_rate, channels));

    auto &protocol = source_audio_protocol_list.back();
    protocol.acodec = acodec;
    protocol.mux = mux;
}

void connection_manager::add_source_video_protocol(
        const char *name,
        const char *mime, const char *suffix,
        unsigned sample_rate, unsigned channels,
        unsigned width, unsigned height,
        unsigned frame_rate_num, unsigned frame_rate_den,
        const char *acodec, const char *vcodec, const char *mux,
        const char *fast_encode_options, const char *slow_encode_options)
{
    std::clog << "[" << this << "] connection_manager: enabled video protocol "
              << name << " " << sample_rate << "/" << channels << " "
              << width << "x" << height << "@" << (float(frame_rate_num) / float(frame_rate_den))
              << std::endl;

    source_video_protocol_list.emplace_back(protocol(
                                                "http-get", mime,
                                                true, false, false,
                                                name, suffix,
                                                sample_rate, channels,
                                                width, height,
                                                frame_rate_num, frame_rate_den));

    auto &protocol = source_video_protocol_list.back();
    protocol.acodec = acodec;
    protocol.vcodec = vcodec;
    protocol.mux = mux;
    protocol.fast_encode_options = fast_encode_options;
    protocol.slow_encode_options = slow_encode_options;
}

std::vector<connection_manager::protocol> connection_manager::get_protocols(unsigned channels) const
{
    std::map<int, std::vector<protocol>> protocols;
    for (auto &protocol : source_audio_protocol_list)
    {
        int score = 0;
        score += ((protocol.channels > 2) == (channels > 2)) ? -2 : 0;

        protocols[score].emplace_back(protocol);
    }

    std::set<std::string> profiles;
    std::vector<protocol> result;
    for (auto &i : protocols)
        if (result.empty() || (i.first <= 0))
            for (auto &protocol : i.second)
                if (profiles.find(protocol.profile) == profiles.end())
                {
                    profiles.insert(protocol.profile);
                    result.emplace_back(std::move(protocol));
                    result.back().channels = std::min(result.back().channels, channels);
                }

    return result;
}

std::vector<connection_manager::protocol> connection_manager::get_protocols(unsigned channels, unsigned width, float frame_rate) const
{
    std::map<int, std::vector<protocol>> protocols;
    for (auto &protocol : source_video_protocol_list)
    {
        int score = 0;
        score += ((protocol.channels > 2) == (channels > 2)) ? -2 : 0;
        score += ((protocol.width > 1280) == (width > 1280)) ? -1 : 0;
        score += ((protocol.width >  876) == (width >  876)) ? -1 : 0;
        score += ((protocol.width >  640) == (width >  640)) ? -1 : 0;
        score += (std::fabs((float(protocol.frame_rate_num) / float(protocol.frame_rate_den)) - frame_rate) > 0.3f) ? 6 : 0;

        protocols[score].emplace_back(protocol);
    }

    std::set<std::string> profiles;
    std::vector<protocol> result;
    for (auto &i : protocols)
        for (auto &protocol : i.second)
            if (profiles.find(protocol.profile) == profiles.end())
            {
                profiles.insert(protocol.profile);
                result.emplace_back(std::move(protocol));
                result.back().channels = std::min(result.back().channels, channels);
            }

    return result;
}

connection_manager::protocol connection_manager::get_protocol(const std::string &profile, unsigned num_channels) const
{
    for (auto &i : get_protocols(num_channels))
        if (i.profile == profile)
            return i;

    return connection_manager::protocol();
}

connection_manager::protocol connection_manager::get_protocol(const std::string &profile, unsigned num_channels, unsigned width, float frame_rate) const
{
    for (auto &i : get_protocols(num_channels, width, frame_rate))
        if (i.profile == profile)
            return i;

    return connection_manager::protocol();
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
    for (auto &protocol : source_audio_protocol_list)
        sp += "," + protocol.to_string(true);

    propset.add_property("SourceProtocolInfo", sp.empty() ? sp : sp.substr(1));

    sp.clear();
    for (auto &protocol : sink_protocol_list)
        sp += "," + protocol.to_string(true);

    propset.add_property("SinkProtocolInfo", sp.empty() ? sp : sp.substr(1));

    propset.add_property("CurrentConnectionIDs", "");
}

int32_t connection_manager::output_connection_add(const struct protocol &protocol)
{
    const auto id = ++connection_id_counter;

    connection_info connection;
    connection.rcs_id = -1;
    connection.avtransport_id = -1;
    connection.protocol_info = "http-get:*:" + protocol.content_format + ":*";
    connection.peerconnection_manager = std::string();
    connection.peerconnection_id = -1;
    connection.direction = connection_info::output;
    connection.status = connection_info::ok;
    connections[id] = connection;

    messageloop.post([this] { rootdevice.emit_event(service_id); });
    for (auto &i : numconnections_changed) if (i.second) i.second(connections.size());

    return id;
}

void connection_manager::output_connection_remove(int32_t id)
{
    auto i = connections.find(id);
    if (i != connections.end())
    {
        connections.erase(i);

        messageloop.post([this] { rootdevice.emit_event(service_id); });
        for (auto &j : numconnections_changed) if (j.second) j.second(connections.size());
    }
}

connection_manager::connection_info connection_manager::output_connection(int32_t id) const
{
    auto i = connections.find(id);
    if (i != connections.end())
        return i->second;

    return connection_info();
}

void connection_manager::output_connection_update(int32_t id, const struct connection_info &connection_info)
{
    auto i = connections.find(id);
    if (i != connections.end())
        i->second = connection_info;
}

std::vector<connection_manager::connection_info> connection_manager::output_connections() const
{
    std::vector<connection_info> result;
    for (auto &i : connections)
        result.emplace_back(i.second);

    return result;
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
    for (auto &protocol : source_audio_protocol_list)
        source_protocols += "," + protocol.to_string(true);

    source_protocols = source_protocols.empty() ? source_protocols : source_protocols.substr(1);

    std::string sink_protocols;
    for (auto &protocol : sink_protocol_list)
        sink_protocols += "," + protocol.to_string(true);

    sink_protocols = sink_protocols.empty() ? sink_protocols : sink_protocols.substr(1);

    action.set_response(source_protocols, sink_protocols);
}

connection_manager::protocol::protocol()
    : play_speed(true), conversion_indicator(false),
      operations_range(false), operations_timeseek(false),
      sample_rate(0), channels(0),
      width(0), height(0)
{
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
        unsigned width, unsigned height,
        unsigned frame_rate_num, unsigned frame_rate_den)
    : network_protocol(network_protocol), network("*"), content_format(content_format),
      profile(profile), play_speed(true), conversion_indicator(conversion_indicator),
      operations_range(operations_range), operations_timeseek(operations_timeseek),
      flags(starts_with(content_format, "image/") ? "00100000" : "01700000"),
      suffix(suffix), sample_rate(sample_rate), channels(channels),
      width(width), height(height),
      frame_rate_num(frame_rate_num), frame_rate_den(frame_rate_den)
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
