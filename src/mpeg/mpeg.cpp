/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#include "mpeg.h"
#include <cassert>
#include <cstring>

namespace mpeg {

const uint64_t max_pack_header_interval = 63000;

ps_packet::ps_packet()
{
}

ps_packet::ps_packet(std::vector<uint8_t> &&buffer)
    : buffer(std::move(buffer))
{
}

ps_packet::ps_packet(ps_packet &&from)
    : buffer(std::move(from.buffer))
{
}

ps_packet & ps_packet::operator=(std::vector<uint8_t> &&buffer)
{
    this->buffer = std::move(buffer);
    return *this;
}

ps_packet & ps_packet::operator=(ps_packet &&from)
{
    buffer = std::move(from.buffer);
    return *this;
}

bool ps_packet::is_valid() const
{
    if (buffer.size() >= 4)
        return (buffer[0] == 0x00) && (buffer[1] == 0x00) && (buffer[2] == 0x01) && (buffer[3] >= 0xB9);

    return false;
}

stream_type ps_packet::stream_id() const
{
    if (is_valid())
        return stream_type(buffer[3]);

    return stream_type::none;
}

static bool is_audio_type(stream_type stream_id)
{
    return
            (stream_id >= stream_type::audio) &&
            (stream_id <  stream_type::video);
}

bool ps_packet::is_audio_stream(void) const
{
    if (is_valid())
        return is_audio_type(stream_type(buffer[3]));

    return false;
}

static bool is_video_type(stream_type stream_id)
{
    return
            (stream_id >= stream_type::video) &&
            (stream_id <= stream_type(uint8_t(stream_type::video) + 0x0F));
}

bool ps_packet::is_video_stream(void) const
{
    if (is_valid())
        return is_video_type(stream_type(buffer[3]));

    return false;
}

bool ps_packet::is_pack_header() const
{
    if (is_valid() && (buffer.size() >= 14))
        return buffer[3] == 0xBA;

    return false;
}

ps_pack_header::ps_pack_header()
{
    buffer.resize(14);
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x01;
    buffer[3] = uint8_t(stream_type::pack_header);

    set_scr(0, 0);
    set_muxrate(20000);

    buffer[13] = 0xF8; // reserved + pack_stuffing_length
}

ps_pack_header::ps_pack_header(ps_packet &&from)
    : ps_packet(std::move(from))
{
}

ps_pack_header & ps_pack_header::operator=(ps_packet &&from)
{
    ps_packet::operator=(std::move(from));
    return *this;
}

bool ps_pack_header::is_valid() const
{
    return
            ps_packet::is_valid() &&
            (stream_type(buffer[3]) == stream_type::pack_header) &&
            (buffer.size() >= 14);
}

size_t ps_pack_header::stuffing_length(void) const
{
    if (buffer.size() >= 14)
        return buffer[13] & 0x07;

    return 0;
}

uint64_t ps_pack_header::scr() const
{
    if (is_valid())
    {
        return
                (uint64_t(buffer[4] & 0x38) << 27) |  // Bits 32..30
                (uint64_t(buffer[4] & 0x03) << 28) |  // Bits 29..28
                (uint64_t(buffer[5]       ) << 20) |  // Bits 27..20
                (uint64_t(buffer[6] & 0xF8) << 12) |  // Bits 19..15
                (uint64_t(buffer[6] & 0x03) << 13) |  // Bits 14..13
                (uint64_t(buffer[7]       ) <<  5) |  // Bits 12..5
                (uint64_t(buffer[8]       ) >>  3);   // Bits  4..0
    }
    else
      return 0;
}

void ps_pack_header::set_scr(uint64_t base, uint16_t ext)
{
    if (is_valid())
    {
        buffer[4]  = uint8_t(base >> 27) & 0x38;  // Base bits 32..30
        buffer[4] |= uint8_t(base >> 28) & 0x03;  // Base bits 29..28
        buffer[4] |= 0x44;                        // Marker bits
        buffer[5]  = uint8_t(base >> 20);         // Base bits 27..20
        buffer[6]  = uint8_t(base >> 12) & 0xF8;  // Base bits 19..15
        buffer[6] |= uint8_t(base >> 13) & 0x03;  // Base bits 14..13
        buffer[6] |= 0x04;                        // Marker bit
        buffer[7]  = uint8_t(base >> 5);          // Base bits 12..5
        buffer[8]  = uint8_t(base << 3);          // Base bits 4..0
        buffer[8] |= uint8_t(ext >> 7) & 0x03;    // Ext bits 8..7
        buffer[8] |= 0x04;                        // Marker bit
        buffer[9]  = uint8_t(ext << 1);           // Ext bits 6..0
        buffer[9] |= 0x01;                        // Marker bit
    }
}

uint32_t ps_pack_header::muxrate() const
{
    if (is_valid())
    {
        return
                (uint32_t(buffer[10]) << 14) |  // Bits 21..14
                (uint32_t(buffer[11]) <<  6) |  // Bits 13..6
                (uint32_t(buffer[12]) >>  2);   // Bits  5..0
    }
    else
      return 0;
}

void ps_pack_header::set_muxrate(uint32_t rate)
{
    if (is_valid())
    {
        buffer[10]  = uint8_t(rate >> 14);         // Bits 21..14
        buffer[11]  = uint8_t(rate >> 6);          // Bits 13..6
        buffer[12]  = uint8_t(rate << 2);          // Bits 5..0
        buffer[12] |= 0x03;                        // Marker bits
    }
}


pes_packet::pes_packet(stream_type stream_id)
{
    const bool ext_header =
            is_audio_type(stream_id) ||
            is_video_type(stream_id) ||
            (stream_id == stream_type::private1);

    buffer.resize(ext_header ? 9 : 4);
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x01;
    buffer[3] = uint8_t(stream_id);

    if (ext_header)
    {
        buffer[6] = 0x80;
        buffer[7] = 0x00;
        buffer[8] = 0x00;
    }
}

pes_packet::pes_packet(ps_packet &&from)
    : ps_packet(std::move(from))
{
}

pes_packet::pes_packet(pes_packet &&from)
    : ps_packet(std::move(from))
{
}

pes_packet & pes_packet::operator=(ps_packet &&from)
{
    ps_packet::operator=(std::move(from));
    return *this;
}

pes_packet & pes_packet::operator=(pes_packet &&from)
{
    ps_packet::operator=(std::move(from));
    return *this;
}

bool pes_packet::is_valid() const
{
    return
            ps_packet::is_valid() &&
            (stream_type(buffer[3]) != stream_type::pack_header) &&
            (buffer.size() >= 9);
}

uint16_t pes_packet::packet_length() const
{
    if (is_valid())
        return (uint16_t(buffer[4]) << 8) | uint16_t(buffer[5]);

    return 0;
}

void pes_packet::set_packet_length(uint16_t length)
{
    if (is_valid())
    {
        buffer[4] = uint8_t(length >> 8);
        buffer[5] = uint8_t(length & 0xFF);
    }
}

bool pes_packet::has_optional_header(void) const
{
    if (is_valid())
    {
        return
                ((buffer[6] & 0xC0) == 0x80) &&
                (is_audio_stream() || is_video_stream() || (stream_type(buffer[3]) == stream_type::private1));
    }

    return false;
}

size_t pes_packet::header_length(void) const
{
    if (is_valid())
        return has_optional_header() ? (size_t(buffer[8]) + 9) : 6;

    return 0;
}

void pes_packet::set_header_length(size_t len)
{
    if (is_valid())
    {
        buffer.resize(len);
        buffer[8] = len - 9;
    }
}

bool pes_packet::has_pts(void) const
{
    if (is_valid())
        return (buffer[7] & 0x80) != 0;

    return false;
}

bool pes_packet::has_dts(void) const
{
    if (is_valid())
        return (buffer[7] & 0x40) != 0;

    return false;
}

uint64_t pes_packet::pts() const
{
    if (has_pts() && (header_length() >= 14))
    {
        return
                (uint64_t(buffer[ 9] & 0x0E) << 29) |  // Bits 30..32
                (uint64_t(buffer[10])        << 22) |  // Bits 22..29
                (uint64_t(buffer[11] & 0xFE) << 14) |  // Bits 15..21
                (uint64_t(buffer[12])        << 7 ) |  // Bits 14..6
                (uint64_t(buffer[13] & 0xFE) >> 1 );   // Bits  6..0
    }
    else
      return 0;
}

void pes_packet::set_pts(uint64_t pts)
{
    if (is_valid())
    {
        if (header_length() < 14)
            set_header_length(14);

        buffer[ 7] |= 0x80; // Set PTS flag
        buffer[ 9]  = 0x21 | uint8_t((pts >> 29) & 0x0E);
        buffer[10]  =        uint8_t((pts >> 22) & 0xFF);
        buffer[11]  = 0x01 | uint8_t((pts >> 14) & 0xFE);
        buffer[12]  =        uint8_t((pts >> 7 ) & 0xFF);
        buffer[13]  = 0x01 | uint8_t((pts << 1 ) & 0xFE);
    }
}

uint64_t pes_packet::dts() const
{
    if (has_dts() && (header_length() >= 19))
    {
        return
                (uint64_t(buffer[14] & 0x0E) << 29) |  // Bits 30..32
                (uint64_t(buffer[15])        << 22) |  // Bits 22..29
                (uint64_t(buffer[16] & 0xFE) << 14) |  // Bits 15..21
                (uint64_t(buffer[17])        << 7 ) |  // Bits 14..6
                (uint64_t(buffer[18] & 0xFE) >> 1 );   // Bits  6..0
    }
    else
        return 0;
}

void pes_packet::set_dts(uint64_t dts)
{
    if (is_valid())
    {
        if (header_length() < 19)
            set_header_length(19);

        buffer[ 7] |= 0x40; // Set DTS flag
        buffer[ 9] |= 0x31;
        buffer[14]  = 0x01 | uint8_t((dts >> 29) & 0x0E);
        buffer[15]  =        uint8_t((dts >> 22) & 0xFF);
        buffer[16]  = 0x01 | uint8_t((dts >> 14) & 0xFE);
        buffer[17]  =        uint8_t((dts >> 7 ) & 0xFF);
        buffer[18]  = 0x01 | uint8_t((dts << 1 ) & 0xFE);
    }
}

size_t pes_packet::payload_size() const
{
    return buffer.size() - header_length();
}

void pes_packet::set_payload_size(size_t len)
{
    buffer.resize(header_length() + len);
}

uint8_t * pes_packet::payload(void)
{
    return buffer.data() + header_length();
}

const uint8_t * pes_packet::payload(void) const
{
    return buffer.data() + header_length();
}

} // End of namespace
