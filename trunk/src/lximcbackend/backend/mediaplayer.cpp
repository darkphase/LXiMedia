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

#include "mediaplayer.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"
#include "vlc/media.h"
#include "vlc/transcode_stream.h"
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

static const size_t min_file_size = 65536;

mediaplayer::mediaplayer(
        class messageloop &messageloop,
        class vlc::instance &vlc_instance,
        pupnp::connection_manager &connection_manager,
        pupnp::content_directory &content_directory,
        const class settings &settings)
    : messageloop(messageloop),
      vlc_instance(vlc_instance),
      connection_manager(connection_manager),
      content_directory(content_directory),
      settings(settings),
      basedir('/' + tr("Media Player") + '/'),
      pending_streams_sever_timer(messageloop, std::bind(&mediaplayer::sever_pending_streams, this))
{
    content_directory.item_source_register(basedir, *this);
}

mediaplayer::~mediaplayer()
{
    content_directory.item_source_unregister(basedir);
}

typedef std::vector<std::pair<std::string, std::vector<vlc::media::track>>> track_list;
static track_list list_tracks(const class vlc::media &media)
{
    std::vector<vlc::media::track> video, audio, text;
    vlc::media::track none;
    none.type = vlc::media::track_type::unknown;
    text.push_back(none);

    for (auto &track : media.tracks())
        switch (track.type)
        {
        case vlc::media::track_type::unknown:   break;
        case vlc::media::track_type::video:     video.push_back(track); break;
        case vlc::media::track_type::audio:     audio.push_back(track); break;
        case vlc::media::track_type::text:      text.push_back(track);  break;
        }

    if (video.empty()) video.push_back(none);
    if (audio.empty()) audio.push_back(none);

    track_list result;
    for (size_t v = 0; v < video.size(); v++)
        for (size_t a = 0; a < audio.size(); a++)
            for (size_t t = 0; t < text.size(); t++)
            {
                std::string name;
                std::vector<vlc::media::track> tracks;

                if (video[v].type != vlc::media::track_type::unknown)
                {
                    tracks.push_back(video[v]);
                    if (video.size() > 1)
                    {
                        name += std::to_string(v + 1) + ". ";
                        if (!video[v].language.empty() && (video[v].language != "und"))
                            name += video[v].language + ' ' + tr("video");
                        else
                            name += tr("Unknown video");
                    }
                }

                if (audio[a].type != vlc::media::track_type::unknown)
                {
                    tracks.push_back(audio[a]);
                    if (audio.size() > 1)
                    {
                        if (!name.empty()) name += ", ";
                        name += std::to_string(a + 1) + ". ";
                        if (!audio[a].language.empty() && (audio[a].language != "und"))
                            name += audio[a].language + ' ' + tr("audio");
                        else
                            name += tr("Unknown audio");
                    }
                }

                if (!name.empty()) name += ", ";
                name += std::to_string(t + 1) + ". ";
                tracks.push_back(text[t]);
                if (text[t].type != vlc::media::track_type::unknown)
                {
                    if (!text[t].language.empty() && (text[t].language != "und"))
                        name += text[t].language + ' ' + tr("subtitles");
                    else
                        name += tr("Unknown subtitles");
                }
                else
                    name += tr("No subtitles");

                if (!name.empty())
                    result.emplace_back(std::make_pair(name, tracks));
            }

    return result;
}

static std::string root_path_name(const std::string &path)
{
    const size_t lsl = std::max(path.find_last_of('/'), path.length() - 1);
    const size_t psl = path.find_last_of('/', lsl - 1);
    if (psl < lsl)
    {
        return path.substr(psl + 1, lsl - psl - 1);
    }
#ifdef WIN32
    else if (lsl != path.npos)
    {
        const std::string name = volume_name(path);
        if (!name.empty())
            return name + " (" + path.substr(0, lsl) + "\\)";
        else
            return path.substr(0, lsl) + "\\";
    }
#endif

    return std::string();
}

std::vector<pupnp::content_directory::item> mediaplayer::list_contentdir_items(
        const std::string &client,
        const std::string &path,
        size_t start, size_t &count)
{
    const bool return_all = count == 0;
    std::vector<pupnp::content_directory::item> result;

    std::vector<std::string> files;
    if (path == basedir)
    {
        for (auto &i : settings.root_paths())
            files.emplace_back(root_path_name(i.path) + '/');
    }
    else if (starts_with(path, basedir))
    {
        if (ends_with(path, "//"))
        {
            const auto system_path = to_system_path(path.substr(0, path.length() - 2));
            const auto media = vlc::media::from_file(vlc_instance, system_path.path);
            for (auto &track : list_tracks(media))
                files.emplace_back(track.first);
        }
        else for (auto &i : list_files(to_system_path(path).path, false, min_file_size))
        {
            const std::string lname = to_lower(i);
            if (!ends_with(lname, ".db" ) && !ends_with(lname, ".idx") &&
                !ends_with(lname, ".nfo") && !ends_with(lname, ".srt") &&
                !ends_with(lname, ".sub") && !ends_with(lname, ".txt"))
            {
                files.emplace_back(std::move(i));
            }
        }
    }

    std::vector<std::future<pupnp::content_directory::item>> futures;
    for (auto &file : files)
        if (return_all || (count > 0))
        {
            if (start == 0)
            {
                futures.emplace_back(std::async(std::launch::async, [this, &client, &path, &file]
                {
                    return make_item(client, path + file);
                }));

                if (count > 0)
                    count--;
            }
            else
                start--;
        }

    for (auto &future : futures)
        result.emplace_back(future.get());

    count = result.size();
    return result;
}

pupnp::content_directory::item mediaplayer::get_contentdir_item(const std::string &client, const std::string &path)
{
    return make_item(client, path);
}

static unsigned codec_block(const std::string &codec)
{
    if      (codec.find("mp1v") != codec.npos) return 16;
    else if (codec.find("mp2v") != codec.npos) return 8;
    else if (codec.find("h264") != codec.npos) return 8;
    else                                       return 16;
}

static void min_scale(const std::string &codec, unsigned srcw, unsigned srch, unsigned dstw, unsigned dsth, unsigned &w, unsigned &h)
{
    const unsigned block = codec_block(codec);
    if ((srcw > 0) && (srch > 0))
    {
        const unsigned f = std::min((dstw * 65536) / srcw, (dsth * 65536) / srch);

        w = ((srcw * f / 65536) + (block / 2) - 1) & ~(block - 1);
        h = ((srch * f / 65536) + (block / 2) - 1) & ~(block - 1);
    }
    else
    {
        w = (dstw + (block / 2) - 1) & ~(block - 1);
        h = (dsth + (block / 2) - 1) & ~(block - 1);
    }
}

static void max_scale(const std::string &codec, unsigned srcw, unsigned srch, unsigned dstw, unsigned dsth, unsigned &w, unsigned &h)
{
    const unsigned block = codec_block(codec);
    if ((srcw > 0) && (srch > 0))
    {
        const unsigned f = std::max((dstw * 65536) / srcw, (dsth * 65536) / srch);

        w = ((srcw * f / 65536) + (block / 2) - 1) & ~(block - 1);
        h = ((srch * f / 65536) + (block / 2) - 1) & ~(block - 1);
    }
    else
    {
        w = (dstw + (block / 2) - 1) & ~(block - 1);
        h = (dsth + (block / 2) - 1) & ~(block - 1);
    }
}

void mediaplayer::correct_protocol(const pupnp::content_directory::item &item, pupnp::connection_manager::protocol &protocol)
{
    switch (settings.canvas_mode())
    {
    case canvas_mode::none:
        min_scale(
                    protocol.vcodec,
                    item.width, item.height,
                    protocol.width, protocol.height,
                    protocol.width, protocol.height);
        break;

    case canvas_mode::pad:
    case canvas_mode::crop:
        break;
    }
}

static void split_path(const std::string &path, std::string &file_path, std::string &track_name)
{
    const size_t sep = path.find("//");
    if (sep != path.npos)
    {
        file_path = path.substr(0, sep);
        track_name = path.substr(sep + 2);
    }
    else
        file_path = path;
}

int mediaplayer::play_item(
        const std::string &source_address,
        const pupnp::content_directory::item &item,
        const std::string &profile,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
    if (!protocol.profile.empty())
    {
        correct_protocol(item, protocol);

        float rate = 1.0f;
        std::ostringstream transcode;
        if (!protocol.acodec.empty() || !protocol.vcodec.empty())
        {
            // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html
            transcode << "#transcode{";

            if (!protocol.vcodec.empty())
            {
                transcode << protocol.vcodec;

                const float frame_rate = float(protocol.frame_rate_num) / float(protocol.frame_rate_den);
                if (std::abs(frame_rate - item.frame_rate) > 0.3f)
                {
                    transcode << ",fps=" << frame_rate;
//                    if (std::abs(protocol.frame_rate - item.frame_rate) < 1.5f)
//                        rate = protocol.frame_rate / item.frame_rate;
                }

                unsigned width = 0, height = 0;
                switch (settings.canvas_mode())
                {
                case canvas_mode::none:
                    width = protocol.width;
                    height = protocol.height;
                    break;

                case canvas_mode::pad:
                    min_scale(
                                protocol.vcodec,
                                item.width, item.height,
                                protocol.width, protocol.height,
                                width, height);
                    break;

                case canvas_mode::crop:
                    max_scale(
                                protocol.vcodec,
                                item.width, item.height,
                                protocol.width, protocol.height,
                                width, height);
                    break;
                }

                transcode << ",width=" << protocol.width << ",height=" << protocol.height;
                if (width < protocol.width)
                {
                    const unsigned pad = ((protocol.width - width) / 2);
                    transcode << ",vfilter=croppadd{paddleft=" << pad << ",paddright=" << pad << '}';
                }
                else if (width > protocol.width)
                {
                    const unsigned crop = ((width - protocol.width) / 2);
                    transcode << ",vfilter=croppadd{cropleft=" << crop << ",cropright=" << crop << '}';
                }
                else if (height < protocol.height)
                {
                    const unsigned pad = ((protocol.height - height) / 2);
                    transcode << ",vfilter=croppadd{paddtop=" << pad << ",paddbottom=" << pad << '}';
                }
                else if (height > protocol.height)
                {
                    const unsigned crop = ((height - protocol.height) / 2);
                    transcode << ",vfilter=croppadd{croptop=" << crop << ",cropbottom=" << crop << '}';
                }

                switch (settings.encode_mode())
                {
                case ::encode_mode::fast:
                    if (!protocol.fast_encode_options.empty())
                        transcode << ',' << protocol.fast_encode_options;

                    break;

                case ::encode_mode::slow:
                    if (!protocol.slow_encode_options.empty())
                        transcode << ',' << protocol.slow_encode_options;

                    break;
                }

                transcode << ",soverlay";
            }

            if (!protocol.acodec.empty())
            {
                if (!protocol.vcodec.empty()) transcode << ',';

                transcode
                        << protocol.acodec
                        << ",samplerate=" << protocol.sample_rate
                        << ",channels=" << protocol.channels;
            }

            transcode << '}';
        }

        std::ostringstream stream_id;
        stream_id << '[' << item.mrl;
        if (item.chapter > 0)               stream_id << "][C" << item.chapter;
        else if (item.position.count() > 0) stream_id << "][" << item.position.count();
        stream_id << "][" << transcode.str() << "][" << protocol.mux << ']';

        // First try to attach to an already running stream.
        auto pending_stream = pending_streams.find(stream_id.str());
        if (pending_stream != pending_streams.end())
        {
            auto stream = std::make_shared<vlc::transcode_stream>(messageloop, vlc_instance, nullptr);
            if (stream->attach(*pending_stream->second.second))
            {
                content_type = protocol.content_format;
                response = stream;
                return pupnp::upnp::http_ok;
            }
        }

        // Get the track IDs.
        struct vlc::transcode_stream::track_ids track_ids;
        std::string file_path, track_name;
        split_path(item.path, file_path, track_name);
        const auto system_path = to_system_path(file_path);
        const auto media = vlc::media::from_file(vlc_instance, system_path.path);
        for (auto &track : list_tracks(media))
            if (track.first == track_name)
                for (auto &t : track.second)
                    switch (t.type)
                    {
                    case vlc::media::track_type::unknown: break;
                    case vlc::media::track_type::audio:  track_ids.audio = t.id; break;
                    case vlc::media::track_type::video:  track_ids.video = t.id; break;
                    case vlc::media::track_type::text:   track_ids.text  = t.id; break;
                    }

        // Otherwise create a new stream.
        std::clog << '[' << this << "] Creating new stream " << item.mrl << " transcode=" << transcode.str() << " mux=" << protocol.mux << std::endl;

        auto &connection_manager = this->connection_manager;
        const auto mrl = item.mrl;
        auto stream = std::make_shared<vlc::transcode_stream>(messageloop, vlc_instance,
            [&connection_manager, protocol, mrl, source_address](int32_t id)
            {
                if (id == 0)
                {
                    id = connection_manager.output_connection_add(protocol);
                    if (id != 0)
                    {
                        auto connection_info = connection_manager.output_connection(id);
                        connection_info.mrl = mrl;
                        connection_info.endpoint = source_address;
                        connection_manager.output_connection_update(id, connection_info);
                    }
                }
                else
                {
                    connection_manager.output_connection_remove(id);
                    id = 0;
                }

                return id;
            });

        if ((item.chapter > 0)
                ? stream->open(item.mrl, item.chapter, track_ids, transcode.str(), protocol.mux, rate)
                : stream->open(item.mrl, item.position, track_ids, transcode.str(), protocol.mux, rate))
        {
            if (pending_streams.empty())
                pending_streams_sever_timer.start(std::chrono::seconds(1));

            pending_streams[stream_id.str()] = std::make_pair(0, stream);
            content_type = protocol.content_format;
            response = stream;
            return pupnp::upnp::http_ok;
        }
    }

    return pupnp::upnp::http_not_found;
}

static void fill_item(
        pupnp::content_directory::item &item,
        const class vlc::media &media,
        const std::vector<vlc::media::track> &tracks,
        path_type type)
{
    for (auto &track : tracks)
        switch (track.type)
        {
        case vlc::media::track_type::unknown:
            break;

        case vlc::media::track_type::audio:
            if (!item.is_audio() && !item.is_video())
                switch (type)
                {
                case path_type::auto_:
                case path_type::pictures:
                case path_type::videos:
                    if (item.duration >= std::chrono::minutes(50))
                        item.type = pupnp::content_directory::item_type::audio_book;
                    else if (item.duration < std::chrono::minutes(5))
                        item.type = pupnp::content_directory::item_type::music;
                    else
                        item.type = pupnp::content_directory::item_type::audio;

                    break;

                case path_type::music:      item.type = pupnp::content_directory::item_type::music;  break;
                }

            item.sample_rate = track.audio.sample_rate;
            item.channels = track.audio.channels;
            break;

        case vlc::media::track_type::video:
            if (!item.is_video())
                switch (type)
                {
                case path_type::auto_:
                case path_type::pictures:
                case path_type::videos:
                    if (item.duration >= std::chrono::minutes(50))
                        item.type = pupnp::content_directory::item_type::movie;
                    else
                        item.type = pupnp::content_directory::item_type::video;

                    break;

                case path_type::music: item.type = pupnp::content_directory::item_type::music_video; break;
                }

            item.width = track.video.width;
            item.height = track.video.height;
            item.frame_rate = track.video.frame_rate;
            break;

        case vlc::media::track_type::text:
            break;
        }

    if (item.is_audio() || item.is_video())
    {
        item.duration = media.duration();

        for (int i = 0, n = media.chapter_count(); i < n; i++)
        {
            std::ostringstream str;
            str << tr("Chapter") << ' ' << (i + 1);

            item.chapters.emplace_back(pupnp::content_directory::chapter { str.str() });
        }
    }
}

pupnp::content_directory::item mediaplayer::make_item(const std::string &/*client*/, const std::string &path) const
{
    std::string file_path, track_name;
    split_path(path, file_path, track_name);

    struct pupnp::content_directory::item item;
    item.is_dir = file_path[file_path.length() - 1] == '/';
    item.path = path;

    if (item.is_dir)
    {
        const size_t lsl = std::max(path.find_last_of('/'), path.length() - 1);
        const size_t psl = path.find_last_of('/', lsl - 1);
        item.title = path.substr(psl + 1, lsl - psl - 1);
    }
    else
    {
        const size_t lsl = path.find_last_of('/');
        item.title = path.substr(lsl + 1);

        const auto system_path = to_system_path(file_path);
        const auto media = vlc::media::from_file(vlc_instance, system_path.path);
        auto tracks = list_tracks(media);
        if (!track_name.empty())
        {
            for (auto &track : tracks)
                if (track.first == track_name)
                {
                    item.mrl = media.mrl();

                    fill_item(item, media, track.second, system_path.type);
                    break;
                }
        }
        else if (tracks.size() > 1)
        {
            item.is_dir = true;
            item.path = file_path + "//";
        }
        else if (tracks.size() == 1)
        {
            item.mrl = media.mrl();

            fill_item(item, media, tracks.front().second, system_path.type);
        }
    }

    return item;
}

root_path mediaplayer::to_system_path(const std::string &virtual_path) const
{
    if (starts_with(virtual_path, basedir))
    {
        const std::string path = virtual_path.substr(basedir.length());
        const std::string root = path.substr(0, path.find_first_of('/'));
        for (auto &i : settings.root_paths())
            if (root == root_path_name(i.path))
                return root_path { i.type, i.path + path.substr(root.length() + 1) };
    }

    return root_path { path_type::auto_, std::string() };
}

std::string mediaplayer::to_virtual_path(const std::string &system_path) const
{
    for (auto &i : settings.root_paths())
        if (starts_with(system_path, i.path))
            return basedir + '/' + root_path_name(i.path) + '/' + system_path.substr(i.path.length());

    return std::string();
}

void mediaplayer::sever_pending_streams()
{
    for (auto i = pending_streams.begin(); i != pending_streams.end(); )
        if (++(i->second.first) >= 10)
            i = pending_streams.erase(i);
        else
            i++;

    if (pending_streams.empty())
        pending_streams_sever_timer.stop();
}
