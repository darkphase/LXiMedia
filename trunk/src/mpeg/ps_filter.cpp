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

//#define DEBUG_OUTPUT

#include "ps_filter.h"
#include <cassert>
#include <cstring>
#include <thread>

#ifdef DEBUG_OUTPUT
# include <iostream>
#endif

namespace mpeg {

class ps_filter::streambuf : public std::streambuf
{
public:
    streambuf(class ps_filter &);

    int underflow() override;

private:
    class ps_filter &parent;
    std::list<class ps_packet> pack;
};

ps_filter::ps_filter(std::unique_ptr<std::istream> &&input)
    : std::istream(new class streambuf(*this)),
      input(std::move(input)),
      stream_finished(false),
      clock_offset(-1),
      next_pack_header(90000)
{
}

ps_filter::~ps_filter()
{
    delete std::istream::rdbuf();
}

static uint64_t get_timestamp(const class pes_packet &pes_packet)
{
    if (pes_packet.has_pts())
        return pes_packet.pts();
    else if (pes_packet.has_dts())
        return pes_packet.dts();

    return uint64_t(-1);
}

std::list<ps_packet> ps_filter::read_pack()
{
    static const size_t max_packet_queue_size = 64;

    std::list<ps_packet> pack;
    for (;;)
    {
        uint64_t lts = uint64_t(-1);
        std::list<pes_packet> *lstream = nullptr;
        size_t min = size_t(-1), max = 0;

        for (auto &i : streams)
        {
            auto &stream = i.second;
            if (!stream.empty())
            {
                min = std::min(min, stream.size());
                max = std::max(max, stream.size());

                const uint64_t ts = get_timestamp(stream.front());
                if (ts == uint64_t(-1))
                {
                    class pes_packet pes_packet(std::move(stream.front()));
                    stream.pop_front();
                    if (!pack.empty())
                        pack.emplace_back(std::move(pes_packet));
                }
                else if (ts < lts)
                {
                    lts = ts;
                    lstream = &stream;
                }
            }
        }

        if (pack.empty() && !stream_finished && (min < max_packet_queue_size))
        {
            // Make sure all stream timestamps starts within 250 ms.
            if (lstream)
                for (auto &i : streams)
                    if ((&i.second != lstream) && !i.second.empty())
                    {
                        const uint64_t ts = get_timestamp(i.second.front());
                        if (ts != uint64_t(-1))
                            while (!lstream->empty() &&
                                   ((int64_t(ts) - int64_t(get_timestamp(lstream->front()))) > 15000))
                            {
                                lstream->pop_front();
                            }
                    }

            filter_packet();
        }
        else if (lstream)
        {
            const stream_type stream_id = lstream->front().stream_id();
            const uint64_t ts = get_timestamp(lstream->front());

            // Handle clock offset.
            auto lts = last_timestamp.find(stream_id);
            if (lts != last_timestamp.end())
            {
                if (std::abs(int64_t(ts) - int64_t(lts->second)) >= 15000)
                {
                    last_timestamp.clear();
                    clock_offset = -1;
                    pack.clear();
                    continue;
                }

                lts->second = ts;
            }
            else
                lts = last_timestamp.emplace(stream_id, ts).first;

            if (clock_offset == uint64_t(-1))
            {
                clock_offset = ts - next_pack_header;

#ifdef DEBUG_OUTPUT
                std::cout << std::hex << unsigned(stream_id) << std::dec
                          << " clock_offset: " << clock_offset
                          << std::endl;
#endif
            }

            if ((int64_t(ts - clock_offset) - int64_t(next_pack_header)) >= 45000)
            {
                // Finish pack.
                class ps_pack_header ps_pack_header;
                ps_pack_header.set_scr(next_pack_header);
                next_pack_header += 45000;

                size_t pack_size = 0;
                for (const auto &i : pack)
                    pack_size += i.size();

                ps_pack_header.set_muxrate(uint32_t(pack_size * 2 / 50));

#ifdef DEBUG_OUTPUT
                std::cout << std::hex << unsigned(ps_pack_header.stream_id()) << std::dec
                          << " scr: " << ps_pack_header.scr()
                          << " muxrate: " << ps_pack_header.muxrate()
                          << std::endl;
#endif

                pack.emplace_front(std::move(ps_pack_header));
                break;
            }
            else if (stream_finished ||
                     (min > (max_packet_queue_size / 2)) ||
                     (max > max_packet_queue_size))
            {
                // Add packet.
                class pes_packet pes_packet(std::move(lstream->front()));
                if (pes_packet.has_pts()) pes_packet.set_pts(pes_packet.pts() - clock_offset);
                if (pes_packet.has_dts()) pes_packet.set_dts(pes_packet.dts() - clock_offset);
                lstream->pop_front();

#ifdef DEBUG_OUTPUT
                std::cout << std::hex << unsigned(pes_packet.stream_id()) << std::dec;
                if (pes_packet.has_pts()) std::cout << " pts: " << pes_packet.pts();
                if (pes_packet.has_dts()) std::cout << " dts: " << pes_packet.dts();
                std::cout << std::endl;
#endif

                pack.emplace_back(std::move(pes_packet));
            }
            else
                filter_packet();
        }
        else if (!stream_finished)
            filter_packet();
        else
            break;
    }

    return std::move(pack);
}

void ps_filter::filter_packet()
{
    class ps_packet ps_packet = read_ps_packet();
    if (!ps_packet.empty())
    {
        if (ps_packet.stream_id() >= stream_type::private1)
        {
            class pes_packet pes_packet(std::move(ps_packet));
            if (pes_packet.is_valid())
            {
                if (pes_packet.packet_length() != (pes_packet.size() - 6))
                    pes_packet.set_packet_length(uint16_t(pes_packet.size() - 6));

                streams[pes_packet.stream_id()].emplace_back(std::move(pes_packet));
            }
        }
    }
    else
        stream_finished = true;
}

static bool is_valid_ps_packet(const uint8_t *buffer)
{
    return (buffer[0] == 0x00) && (buffer[1] == 0x00) && (buffer[2] == 0x01) && (buffer[3] >= 0xBA);
}

ps_packet ps_filter::read_ps_packet()
{
    uint8_t header[6];
    if (input->read(reinterpret_cast<char *>(header), sizeof(header)))
    {
        while (!is_valid_ps_packet(header) && *input)
        {
            memmove(header, header + 1, sizeof(header) - 1);
            header[sizeof(header) - 1] = uint8_t(input->get());
        }

        if (stream_type(header[3]) != stream_type::pack_header)
        {
            const uint16_t length = (uint16_t(header[4]) << 8) | uint16_t(header[5]);

            std::vector<uint8_t> buffer;
            buffer.resize(sizeof(header) + size_t(length));
            memcpy(buffer.data(), header, sizeof(header));
            if (input->read(reinterpret_cast<char *>(buffer.data() + sizeof(header)), length))
                return std::move(buffer);
        }
        else
        {
            std::vector<uint8_t> buffer;
            buffer.resize(sizeof(header) + 15);
            memcpy(buffer.data(), header, sizeof(header));
            if (input->read(reinterpret_cast<char *>(buffer.data() + sizeof(header)), 8))
            {
                const uint8_t stuffing = buffer[13] & 0x07;
                if (stuffing > 0)
                    input->read(reinterpret_cast<char *>(buffer.data() + 14), stuffing);

                buffer.resize(14 + stuffing);
                return std::move(buffer);
            }
        }
    }

    return ps_packet();
}


ps_filter::streambuf::streambuf(class ps_filter &parent)
    : parent(parent)
{
}

int ps_filter::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    if (!pack.empty())
        pack.pop_front();

    if (pack.empty())
        pack = parent.read_pack();

    if (!pack.empty())
    {
        char * const data = reinterpret_cast<char *>(pack.front().data());
        const size_t size = pack.front().size();

        setg(data, data, data + size);
        return traits_type::to_int_type(*gptr());
    }
    else
        return traits_type::eof();
}

} // End of namespace