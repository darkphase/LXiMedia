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
#include "string.h"
#include "translator.h"
#include "vlc/media.h"
#include "vlc/transcode_stream.h"
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

static const off_t min_file_size = 65536;
static std::vector<std::string> list_files(const std::string &path);

struct mediaplayer::transcode_stream : vlc::transcode_stream
{
    transcode_stream(
            mediaplayer &parent,
            const std::string &stream_id,
            const pupnp::connection_manager::protocol &protocol)
        :   vlc::transcode_stream(parent.messageloop, parent.vlc_instance),
          parent(parent),
          stream_id(stream_id),
          sever_timer(parent.messageloop, std::bind(&transcode_stream::sever, this))
    {
        parent.connection_manager.output_connection_add(*this, protocol);

        sever_timer.start(std::chrono::seconds(10), true);
    }

    ~transcode_stream()
    {
        parent.connection_manager.output_connection_remove(*this);
    }

    void sever()
    {
        parent.pending_transcode_streams.erase(stream_id);
    }

    mediaplayer &parent;
    const std::string stream_id;
    timer sever_timer;
};

mediaplayer::mediaplayer(
        class messageloop &messageloop,
        class vlc::instance &vlc_instance,
        pupnp::connection_manager &connection_manager,
        pupnp::content_directory &content_directory,
        const class settings &settings)
    :   messageloop(messageloop),
      vlc_instance(vlc_instance),
      connection_manager(connection_manager),
      content_directory(content_directory),
      settings(settings),
      basedir('/' + tr("Media Player") + '/')
{
    content_directory.item_source_register(basedir, *this);
}

mediaplayer::~mediaplayer()
{
    content_directory.item_source_unregister(basedir);
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
        {
            const size_t lsl = std::max(i.path.find_last_of('/'), i.path.length() - 1);
            const size_t psl = i.path.find_last_of('/', lsl - 1);
            if (psl < lsl)
                files.emplace_back(i.path.substr(psl + 1, lsl - psl - 1) + '/');
        }
    }
    else if (starts_with(path, basedir))
        files = list_files(to_system_path(path).path);

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

int mediaplayer::play_item(
        const pupnp::content_directory::item &item,
        const std::string &profile,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
    if (!protocol.profile.empty())
    {
        correct_protocol(item, protocol);

        std::ostringstream transcode;
        if (!protocol.acodec.empty() || !protocol.vcodec.empty())
        {
            // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

            transcode << "#transcode{";
            // Fixes: https://forum.videolan.org/viewtopic.php?f=13&t=115390
            //transcode << "#lximedia_transcode{";

            if (!protocol.vcodec.empty())
            {
                transcode << protocol.vcodec << ",fps=" << protocol.frame_rate;

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

                transcode << ",width=" << width << ",height=" << height;
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
        if (item.chapter > 0)   stream_id << "][C" << item.chapter;
        else                    stream_id << "][" << item.position.count();
        stream_id << "][" << transcode.str() << "][" << protocol.mux << ']';

        // First try to attach to an already running stream.
        auto pending_stream = pending_transcode_streams.find(stream_id.str());
        if (pending_stream != pending_transcode_streams.end())
        {
            auto stream = std::make_shared<vlc::transcode_stream>(messageloop, vlc_instance);
            if (stream->attach(*pending_stream->second))
            {
                content_type = protocol.content_format;
                response = stream;
                return pupnp::upnp::http_ok;
            }
        }

        // Otherwise create a new stream.
        std::clog << '[' << this << "] Creating new stream " << item.mrl << " transcode=" << transcode.str() << " mux=" << protocol.mux << std::endl;
        auto stream = std::make_shared<transcode_stream>(*this, stream_id.str(), protocol);
        if ((item.chapter > 0)
                ? stream->open(item.mrl, item.chapter, transcode.str(), protocol.mux)
                : stream->open(item.mrl, item.position, transcode.str(), protocol.mux))
        {
            pending_transcode_streams[stream_id.str()] = stream;
            content_type = protocol.content_format;
            response = stream;
            return pupnp::upnp::http_ok;
        }
    }

    return pupnp::upnp::http_not_found;
}

pupnp::content_directory::item mediaplayer::make_item(const std::string &/*client*/, const std::string &path) const
{
    struct pupnp::content_directory::item item;
    item.is_dir = path[path.length() - 1] == '/';
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

        const auto system_path = to_system_path(path);
        const auto media = vlc::media::from_file(vlc_instance, system_path.path);
        for (auto &track : media.tracks())
        {
            switch (track.type)
            {
            case vlc::media::track_type::unknown:
                break;

            case vlc::media::track_type::audio:
                if (!item.is_audio() && !item.is_video())
                    switch (system_path.type)
                    {
                    case path_type::auto_: item.type = pupnp::content_directory::item_type::audio;  break;
                    case path_type::music: item.type = pupnp::content_directory::item_type::music;  break;
                    }

                item.sample_rate = track.audio.sample_rate;
                item.channels = track.audio.channels;
                break;

            case vlc::media::track_type::video:
                if (!item.is_video())
                    switch (system_path.type)
                    {
                    case path_type::auto_: item.type = pupnp::content_directory::item_type::movie;          break;
                    case path_type::music: item.type = pupnp::content_directory::item_type::music_video;    break;
                    }

                item.width = track.video.width;
                item.height = track.video.height;
                item.frame_rate = track.video.frame_rate;
                break;

            case vlc::media::track_type::text:
                break;
            }
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

        item.mrl = media.mrl();
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
        {
            const size_t lsl = std::max(i.path.find_last_of('/'), i.path.length() - 1);
            const size_t psl = i.path.find_last_of('/', lsl - 1);
            const std::string name = i.path.substr(psl + 1, lsl - psl - 1);
            if (root == name)
                return root_path { i.type, i.path + path.substr(root.length() + 1) };
        }
    }

    return root_path { path_type::auto_, std::string() };
}

std::string mediaplayer::to_virtual_path(const std::string &system_path) const
{
    for (auto &i : settings.root_paths())
        if (starts_with(system_path, i.path))
        {
            const size_t lsl = std::max(i.path.find_last_of('/'), i.path.length() - 1);
            const size_t psl = i.path.find_last_of('/', lsl - 1);
            const std::string name = i.path.substr(psl + 1, lsl - psl - 1);

            return basedir + '/' + name + '/' + system_path.substr(i.path.length());
        }

    return std::string();
}

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static std::vector<std::string> list_files(const std::string &path)
{
    std::multimap<std::string, std::string> dirs, files;

    auto dir = ::opendir(path.c_str());
    if (dir)
    {
        for (auto dirent = ::readdir(dir); dirent; dirent = ::readdir(dir))
        {
            struct stat stat;
            if ((dirent->d_name[0] != '.') &&
                (::stat((path + '/' + dirent->d_name).c_str(), &stat) == 0))
            {
                std::string name = dirent->d_name, lname = to_lower(name);
                if (S_ISDIR(stat.st_mode) &&
                    (lname != "@eadir"))
                {
                    dirs.emplace(std::move(lname), name + '/');
                }
                else if ((stat.st_size >= min_file_size) &&
                         !ends_with(lname, ".db" ) &&
                         !ends_with(lname, ".idx") &&
                         !ends_with(lname, ".nfo") &&
                         !ends_with(lname, ".srt") &&
                         !ends_with(lname, ".sub") &&
                         !ends_with(lname, ".txt"))
                {
                    files.emplace(std::move(lname), std::move(name));
                }
            }
        }

        ::closedir(dir);
    }

    std::vector<std::string> result;
    result.reserve(dirs.size() + files.size());
    for (auto &i : dirs) result.emplace_back(std::move(i.second));
    for (auto &i : files) result.emplace_back(std::move(i.second));
    return result;
}
#endif
