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

#ifndef MPEG_MPEG_H
#define MPEG_MPEG_H

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace mpeg {

enum class stream_type : uint8_t
{
    none                = 0x00,

    end_code            = 0xB9,

    pack_header         = 0xBA,
    system_header       = 0xBB,
    program_stream_map  = 0xBC,

    private1            = 0xBD,
    padding             = 0xBE,
    private2            = 0xBF,

    audio               = 0xC0,
    video               = 0xE0,
    ecm                 = 0xF0,
};

class ps_packet
{
public:
    ps_packet();
    ps_packet(std::vector<uint8_t> &&);
    ps_packet(ps_packet &&);
    ps_packet(const ps_packet &) = delete;
    ps_packet & operator=(std::vector<uint8_t> &&);
    ps_packet & operator=(ps_packet &&);
    ps_packet & operator=(const ps_packet &) = delete;

    const uint8_t * data() const { return buffer.data(); }
    uint8_t * data() { return buffer.data(); }
    inline size_t size() const { return buffer.size(); }
    inline bool empty() const { return buffer.empty(); }

    bool is_valid() const;
    stream_type stream_id() const;
    bool is_audio_stream(void) const;
    bool is_video_stream(void) const;
    bool is_pack_header() const;

protected:
    std::vector<uint8_t> buffer;
};

class ps_pack_header : public ps_packet
{
public:
    ps_pack_header();
    ps_pack_header(ps_packet &&);
    ps_pack_header(const ps_pack_header &) = delete;
    ps_pack_header & operator=(ps_packet &&);
    ps_pack_header & operator=(const ps_pack_header &) = delete;

    bool is_valid() const;
    size_t stuffing_length(void) const;
    uint64_t scr() const;
    void set_scr(uint64_t, uint16_t = 0);
    uint32_t muxrate() const;
    void set_muxrate(uint32_t);
};

class pes_packet : public ps_packet
{
public:
    pes_packet(stream_type);
    pes_packet(ps_packet &&);
    pes_packet(pes_packet &&);
    pes_packet(const pes_packet &) = delete;
    pes_packet & operator=(ps_packet &&);
    pes_packet & operator=(pes_packet &&);
    pes_packet & operator=(const pes_packet &) = delete;

    bool is_valid() const;
    uint16_t packet_length() const;
    void set_packet_length(uint16_t);
    bool has_optional_header(void) const;
    size_t header_length(void) const;
    void set_header_length(size_t len);
    bool has_pts(void) const;
    bool has_dts(void) const;
    uint64_t pts() const;
    void set_pts(uint64_t);
    uint64_t dts() const;
    void set_dts(uint64_t);
    size_t payload_size() const;
    void set_payload_size(size_t len);
    uint8_t * payload(void);
    const uint8_t * payload(void) const;

    static const size_t max_payload_size = 65506;
};

} // End of namespace

#endif
