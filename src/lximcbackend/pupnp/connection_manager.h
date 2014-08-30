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

#ifndef PUPNP_CONNECTION_MAMANGER_H
#define PUPNP_CONNECTION_MAMANGER_H

#include "rootdevice.h"
#include "upnp.h"
#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pupnp {

class connection_manager : public rootdevice::service
{
public:
    struct protocol
    {
        protocol();
        protocol(
                const std::string &network_protocol,
                const std::string &content_format,
                bool conversion_indicator,
                bool operations_range,
                bool operations_timeseek,
                const std::string &profile,
                const std::string &suffix,
                unsigned sample_rate = 0, unsigned channels = 0,
                unsigned width = 0, unsigned height = 0, float frame_rate = 0.0f);

        std::string to_string(bool brief = false) const;  //!< Returns the DLNA protocol string.
        std::string content_features() const;             //!< Returns the DLNA contentFeatures string.

        std::string network_protocol;                     //!< The network protocol used (e.g. "http-get").
        std::string network;                              //!< The network used, usually not needed.
        std::string content_format;                       //!< The content format used with the protocol (e.g. MIME-type for "http-get").

        std::string profile;                              //!< The profile name of the protocol (e.g. "JPEG_TN").
        bool play_speed;                                  //!< false = invalid play speed, true = normal play speed.
        bool conversion_indicator;                        //!< false = not transcoded, true = transcoded.
        bool operations_range;                            //!< true = range supported.
        bool operations_timeseek;                         //!< true = time seek range supported.

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
        std::string flags;

        std::string suffix;                               //!< The file extension (including .).

        unsigned sample_rate, channels;
        unsigned width, height;
        float frame_rate;

        std::string acodec, vcodec, mux;

        std::string fast_encode_options;
        std::string slow_encode_options;
    };

    struct connection_info
    {
        connection_info();
        ~connection_info();

        int32_t     rcs_id;
        int32_t     avtransport_id;
        std::string protocol_info;
        std::string peerconnection_manager;
        int32_t     peerconnection_id;
        enum { input, output } direction;
        enum { ok, contentformat_mismatch, insufficient_bandwidth, unreliable_channel, unknown } status;

        std::string mrl;
        std::string endpoint;
    };

    struct action_get_current_connectionids
    {
        virtual void set_response(const std::vector<int32_t> &) = 0;
    };

    struct action_get_current_connection_info
    {
        virtual int32_t get_connectionid() const = 0;

        virtual void set_response(const connection_info &info) = 0;
    };

    struct action_get_protocol_info
    {
        virtual void set_response(const std::string &source, const std::string &sink) = 0;
    };

public:
    static const char service_id[];
    static const char service_type[];

    connection_manager(class messageloop &, class rootdevice &);
    virtual ~connection_manager();

    void add_source_audio_protocol(
            const char *name,
            const char *mime, const char *suffix,
            unsigned sample_rate, unsigned channels,
            const char *acodec, const char *mux);

    void add_source_video_protocol(
            const char *name,
            const char *mime, const char *suffix,
            unsigned sample_rate, unsigned channels,
            unsigned width, unsigned height, float frame_rate,
            const char *acodec, const char *vcodec, const char *mux,
            const char *fast_encode_options, const char *slow_encode_options);

    std::vector<protocol> get_protocols(unsigned channels) const;
    std::vector<protocol> get_protocols(unsigned channels, unsigned width, float frame_rate) const;
    protocol get_protocol(const std::string &profile, unsigned num_channels) const;
    protocol get_protocol(const std::string &profile, unsigned num_channels, unsigned width, float frame_rate) const;

    void output_connection_add(std::istream &, const struct protocol &);
    void output_connection_remove(std::istream &);
    connection_info output_connection(std::istream &) const;
    void output_connection_update(std::istream &, const struct connection_info &);
    std::vector<connection_info> output_connections() const;

    void handle_action(const upnp::request &, action_get_current_connectionids &);
    void handle_action(const upnp::request &, action_get_current_connection_info &);
    void handle_action(const upnp::request &, action_get_protocol_info &);

    std::map<void *, std::function<void(size_t)>> numconnections_changed;

private: // From rootdevice::service
    virtual const char * get_service_type(void) override final;

    virtual void initialize(void) override final;
    virtual void close(void) override final;

    virtual void write_service_description(rootdevice::service_description &) const override final;
    virtual void write_eventable_statevariables(rootdevice::eventable_propertyset &) const override final;

private:
    class messageloop &messageloop;
    class rootdevice &rootdevice;

    std::vector<protocol> source_audio_protocol_list;
    std::vector<protocol> source_video_protocol_list;

    std::vector<protocol> sink_protocol_list;

    int32_t connection_id_counter;
    std::map<int32_t, connection_info> connections;
    std::map<std::istream *, int32_t> streams;
};

} // End of namespace

#endif
