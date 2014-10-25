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

#include "files.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"
#include "vlc/instance.h"
#include "vlc/media.h"
#include "vlc/transcode_stream.h"
#include "watchlist.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

files::files(
        class platform::messageloop_ref &messageloop,
        class vlc::instance &vlc_instance,
        class pupnp::connection_manager &connection_manager,
        class pupnp::content_directory &content_directory,
        class recommended &recommended,
        const class settings &settings,
        class watchlist &watchlist)
    : messageloop(messageloop),
      vlc_instance(vlc_instance),
      media_cache(this->messageloop),
      connection_manager(connection_manager),
      content_directory(content_directory),
      recommended(recommended),
      settings(settings),
      watchlist(watchlist),
      basedir('/' + tr("Files") + '/'),
      max_parse_time(3000)
{
    content_directory.item_source_register(basedir, *this);
    recommended.item_source_register(basedir, *this);
}

files::~files()
{
    recommended.item_source_unregister(basedir);
    content_directory.item_source_unregister(basedir);
}

typedef std::vector<std::pair<std::string, std::vector<vlc::media_cache::track>>> track_list;
static track_list list_tracks(class vlc::media_cache &media_cache, class vlc::media &media)
{
    std::vector<vlc::media_cache::track> video, audio, text;
    vlc::media_cache::track none;
    none.type = vlc::media_cache::track_type::unknown;
    text.push_back(none);

    for (auto &track : media_cache.tracks(media))
        switch (track.type)
        {
        case vlc::media_cache::track_type::unknown:   break;
        case vlc::media_cache::track_type::video:     video.push_back(track); break;
        case vlc::media_cache::track_type::audio:     audio.push_back(track); break;
        case vlc::media_cache::track_type::text:      text.push_back(track);  break;
        }

    if (video.empty()) video.push_back(none);
    if (audio.empty()) audio.push_back(none);

    track_list result;
    for (size_t v = 0; v < video.size(); v++)
        for (size_t a = 0; a < audio.size(); a++)
            for (size_t t = 0; t < text.size(); t++)
            {
                std::string name;
                std::vector<vlc::media_cache::track> tracks;

                if (video[v].type != vlc::media_cache::track_type::unknown)
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

                if (audio[a].type != vlc::media_cache::track_type::unknown)
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
                if (text[t].type != vlc::media_cache::track_type::unknown)
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
        const std::string name = platform::volume_name(path);
        if (!name.empty())
            return name + " (" + path.substr(0, lsl) + "\\)";
        else
            return path.substr(0, lsl) + "\\";
    }
#endif

    return std::string();
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

std::vector<pupnp::content_directory::item> files::list_contentdir_items(
        const std::string &client,
        const std::string &path,
        size_t start, size_t &count)
{
    const size_t chunk_size = count;
    const bool return_all = count == 0;

    const auto &files = list_files(path, start == 0);

    std::vector<vlc::media> items, next_items;
    std::vector<std::string> paths;
    for (auto &file : files)
        if (return_all || (count > 0))
        {
            if (start == 0)
            {
                std::string full_path = path + file, file_path, track_name;
                split_path(full_path, file_path, track_name);
                if (!ends_with(file_path, "/"))
                    items.emplace_back(vlc::media::from_file(vlc_instance, to_system_path(file_path).path));

                paths.emplace_back(std::move(full_path));

                if (count > 0)
                    count--;
            }
            else
                start--;
        }
        else if ((start == 0) && (next_items.size() < chunk_size))
        {
            std::string full_path = path + file, file_path, track_name;
            split_path(full_path, file_path, track_name);
            if (!ends_with(file_path, "/"))
                next_items.emplace_back(vlc::media::from_file(vlc_instance, to_system_path(file_path).path));
        }

    media_cache.on_finished = nullptr;
    media_cache.async_parse_items(items);
    if (media_cache.wait_for(max_parse_time) == std::cv_status::timeout)
    {
        media_cache.on_finished = [this, path, next_items]
        {
            content_directory.update_path(path);
            if (!next_items.empty())
                media_cache.async_parse_items(next_items);
        };
    }
    else if (!next_items.empty())
        media_cache.async_parse_items(next_items);

    std::vector<pupnp::content_directory::item> result;
    for (auto &path : paths)
        result.emplace_back(make_item(client, path));

    count = files.size();
    return result;
}

std::vector<pupnp::content_directory::item> files::list_recommended_items(
        const std::string &client,
        size_t start, size_t &count)
{
    const bool return_all = count == 0;
    const auto watched_items = watchlist.watched_items();

    std::set<std::string> directories;
    for (auto &i : watched_items)
    {
#if defined(__unix__)
        if (starts_with(i.first, "file://"))
        {
            std::string path = from_percent(i.first.substr(7));
            const auto ls = path.find_last_of('/');
#elif defined(WIN32)
        if (starts_with(i.first, "file:"))
        {
            std::string path = from_percent(i.first.substr(5));
            const auto ls = path.find_last_of("/\\");
#endif

            if (ls != path.npos)
            {
                const auto virtual_path = to_virtual_path(path.substr(0, ls + 1));
                if (directories.find(virtual_path) == directories.end())
                {
                    const auto system_path = to_system_path(virtual_path);
                    if ((system_path.type == path_type::auto_) ||
                        (system_path.type == path_type::videos))
                    {
                        directories.emplace(virtual_path);
                    }
                }
            }
        }
    }

    std::vector<vlc::media> recommended_items;
    std::multimap<double, std::string> recommended_paths;
    for (auto &i : directories)
    {
        std::chrono::minutes last_seen(0);
        vlc::media last_media;
        std::string last_path;
        double last_score = 0.0;

        for (auto &j : list_files(i, false))
        {
            std::string full_path = i + j, file_path, track_name;
            split_path(full_path, file_path, track_name);
            if (!ends_with(file_path, "/"))
            {
                auto media = vlc::media::from_file(vlc_instance, to_system_path(file_path).path);
                auto watched = watched_items.find(media.mrl());
                if (watched != watched_items.end())
                {
                    last_media = vlc::media();
                    last_path.clear();
                    last_score = 0.0;
                    last_seen = watched->second;
                }
                else if (last_seen.count() > 0)
                {
                    last_media = std::move(media);
                    last_path = file_path;
                    last_score = 1.0 - (1.0 / double(last_seen.count()));
                    last_seen = std::chrono::minutes(0);
                }
            }
        }

        if (last_media)
        {
            recommended_items.emplace_back(std::move(last_media));
            recommended_paths.emplace(last_score, std::move(last_path));
        }
    }

    media_cache.on_finished = nullptr;
    media_cache.async_parse_items(recommended_items);
    media_cache.wait_for(max_parse_time);

    std::vector<pupnp::content_directory::item> result;
    for (auto &path : recommended_paths)
        if (return_all || (count > 0))
        {
            if (start == 0)
            {
                result.emplace_back(make_item(client, path.second));

                if (count > 0)
                    count--;
            }
            else
                start--;
        }

    count = recommended_paths.size();
    return result;
}

pupnp::content_directory::item files::get_contentdir_item(const std::string &client, const std::string &path)
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

bool files::correct_protocol(const pupnp::content_directory::item &item, pupnp::connection_manager::protocol &protocol)
{
    switch (settings.canvas_mode())
    {
    case canvas_mode::none:
        min_scale(
                    protocol.vcodec,
                    item.width / protocol.aspect, item.height,
                    protocol.width, protocol.height,
                    protocol.width, protocol.height);
        break;

    case canvas_mode::pad:
    case canvas_mode::crop:
        break;
    }

    return true;
}

int files::play_item(
        const std::string &source_address,
        const pupnp::content_directory::item &item,
        const std::string &profile,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    using namespace std::placeholders;

    auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
    if (!protocol.profile.empty() && correct_protocol(item, protocol))
    {
        float rate = 1.0f;
        std::string transcode_plugin = "#transcode";
        std::ostringstream transcode;
        if (!protocol.acodec.empty() || !protocol.vcodec.empty())
        {
            // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html
            transcode << "{";

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
                                unsigned((item.width / protocol.aspect) + 0.5f), item.height,
                                protocol.width, protocol.height,
                                width, height);
                    break;

                case canvas_mode::crop:
                    max_scale(
                                protocol.vcodec,
                                unsigned((item.width / protocol.aspect) + 0.5f), item.height,
                                protocol.width, protocol.height,
                                width, height);
                    break;
                }

                transcode << ",width=" << protocol.width << ",height=" << protocol.height;

                if ((width != protocol.width) || (height != protocol.height))
                {
                    // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
                    if (vlc::instance::compare_version(2, 1) == 0)
                        transcode_plugin = "#lximedia_transcode";

                    transcode
                            << ",vfilter=canvas{width=" << protocol.width
                            << ",height=" << protocol.height;

                    const float aspect = float(protocol.width * protocol.aspect) / float(protocol.height);
                    if (std::abs(aspect - 1.33333f) < 0.1f)
                        transcode << ",aspect=4:3";
                    else if (std::abs(aspect - 1.77778f) < 0.1f)
                        transcode << ",aspect=16:9";

                    transcode << ",padd=true}";
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

        std::ostringstream opt;
        if (item.chapter > 0)               opt << "@C" << item.chapter;
        else if (item.position.count() > 0) opt << "@" << item.position.count();

        // First try to attach to an already running stream.
        response = connection_manager.try_attach_output_connection(protocol, item.mrl, source_address, opt.str());
        if (!response)
        {
            std::clog << '[' << this << "] Creating new stream " << item.mrl << " transcode=" << transcode.str() << " mux=" << protocol.mux << std::endl;

            struct vlc::transcode_stream::track_ids track_ids;
            std::string file_path, track_name;
            split_path(item.path, file_path, track_name);
            const auto system_path = to_system_path(file_path);
            auto media = vlc::media::from_file(vlc_instance, system_path.path);
            for (auto &track : list_tracks(media_cache, media))
                if (track.first == track_name)
                    for (auto &t : track.second)
                        switch (t.type)
                        {
                        case vlc::media_cache::track_type::unknown: break;
                        case vlc::media_cache::track_type::audio:  track_ids.audio = t.id; break;
                        case vlc::media_cache::track_type::video:  track_ids.video = t.id; break;
                        case vlc::media_cache::track_type::text:   track_ids.text  = t.id; break;
                        }

            std::unique_ptr<vlc::transcode_stream> stream(new vlc::transcode_stream(messageloop, vlc_instance));
            stream->on_playback_progress = std::bind(&files::playback_progress, this, item, _1);
            if ((item.chapter > 0)
                    ? stream->open(item.mrl, item.chapter, track_ids, transcode_plugin + transcode.str(), protocol.mux, rate)
                    : stream->open(item.mrl, item.position, track_ids, transcode_plugin + transcode.str(), protocol.mux, rate))
            {
                auto proxy = std::make_shared<pupnp::connection_proxy>(std::move(stream));
                connection_manager.add_output_connection(proxy, protocol, item.mrl, source_address, opt.str());
                response = proxy;
            }
        }

        if (response)
        {
            content_type = protocol.content_format;
            return pupnp::upnp::http_ok;
        }
    }

    return pupnp::upnp::http_not_found;
}

static void fill_item(
        pupnp::content_directory::item &item,
        class vlc::media_cache &media_cache,
        class vlc::media &media,
        const std::vector<vlc::media_cache::track> &tracks,
        path_type type)
{
    for (auto &track : tracks)
        switch (track.type)
        {
        case vlc::media_cache::track_type::unknown:
            break;

        case vlc::media_cache::track_type::audio:
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

        case vlc::media_cache::track_type::video:
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

        case vlc::media_cache::track_type::text:
            break;
        }

    if (item.is_audio() || item.is_video())
    {
        item.duration = media_cache.duration(media);

        for (int i = 0, n = media_cache.chapter_count(media); i < n; i++)
        {
            std::ostringstream str;
            str << tr("Chapter") << ' ' << (i + 1);

            item.chapters.emplace_back(pupnp::content_directory::chapter { str.str() });
        }
    }
}

const std::vector<std::string> & files::list_files(const std::string &path, bool flush_cache)
{
    static const char dir_prefix = 'D', file_prefix = 'F';

    auto files_cache_item = (!flush_cache) ? files_cache.find(path) : files_cache.end();
    if (files_cache_item == files_cache.end())
    {
        std::multimap<std::string, std::string, alphanum_less> files;
        if (path == basedir)
        {
            for (auto &i : settings.root_paths())
            {
                const auto name = root_path_name(i.path) + '/';
                files.emplace(dir_prefix + to_lower(name), name);
            }

            if (settings.share_removable_media())
                for (auto &i : platform::list_removable_media())
                {
                    const auto name = root_path_name(i) + '/';
                    files.emplace(dir_prefix + to_lower(name), name);
                }
        }
        else if (starts_with(path, basedir))
        {
            if (ends_with(path, "//"))
            {
                const auto system_path = to_system_path(path.substr(0, path.length() - 2));
                auto media = vlc::media::from_file(vlc_instance, system_path.path);
                if (media_cache.has_data(media))
                    for (auto &track : list_tracks(media_cache, media))
                        files.emplace(file_prefix + to_lower(track.first), track.first);
            }
            else
            {
                const auto system_path = to_system_path(path).path;
                for (auto &i : platform::list_files(system_path, false))
                {
                    if (ends_with(i, "/"))
                    {
                        const auto children = platform::list_files(system_path + i, false, 2);
                        if ((children.size() == 1) && !ends_with(children.front(), "/"))
                            files.emplace(file_prefix + to_lower(children.front()), i + children.front());
                        else if (children.size() > 0)
                            files.emplace(dir_prefix + to_lower(i), i);
                    }
                    else
                        files.emplace(file_prefix + to_lower(i), i);
                }
            }
        }

        std::vector<std::string> sorted_files;
        for (auto &i : files)
            sorted_files.emplace_back(std::move(i.second));

        files_cache[path] = std::move(sorted_files);
        files_cache_item = files_cache.find(path);
    }

    return files_cache_item->second;
}

pupnp::content_directory::item files::make_item(const std::string &/*client*/, const std::string &path) const
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
        auto media = vlc::media::from_file(vlc_instance, system_path.path);

        if (media_cache.has_data(media))
        {
            const auto mrl = media.mrl();
            const auto uuid = media_cache.uuid(media);

            item.last_position = watchlist.last_position(uuid);

            auto tracks = list_tracks(media_cache, media);
            if (!track_name.empty())
            {
                for (auto &track : tracks)
                    if (track.first == track_name)
                    {
                        item.mrl = mrl;
                        item.uuid = uuid;

                        fill_item(item, media_cache, media, track.second, system_path.type);
                        break;
                    }
            }
            else if (tracks.size() > 1)
            {
                item.is_dir = true;
                item.path = file_path + "//";
                item.uuid = uuid;
                item.duration = media_cache.duration(media);
            }
            else if (tracks.size() == 1)
            {
                item.mrl = mrl;
                item.uuid = uuid;
                fill_item(item, media_cache, media, tracks.front().second, system_path.type);
            }
        }
    }

    return item;
}

root_path files::to_system_path(const std::string &virtual_path) const
{
    if (starts_with(virtual_path, basedir))
    {
        const std::string path = virtual_path.substr(basedir.length());
        const std::string root = path.substr(0, path.find_first_of('/'));

        for (auto &i : settings.root_paths())
            if (root == root_path_name(i.path))
                return root_path { i.type, i.path + path.substr(root.length() + 1) };

        if (settings.share_removable_media())
            for (auto &i : platform::list_removable_media())
                if (root == root_path_name(i))
                    return root_path { path_type::auto_, i + path.substr(root.length() + 1) };
    }

    return root_path { path_type::auto_, std::string() };
}

std::string files::to_virtual_path(const std::string &system_path) const
{
    for (auto &i : settings.root_paths())
        if (starts_with(system_path, i.path))
            return basedir + root_path_name(i.path) + '/' + system_path.substr(i.path.length());

    if (settings.share_removable_media())
        for (auto &i : platform::list_removable_media())
            if (starts_with(system_path, i))
                return basedir + root_path_name(i) + '/' + system_path.substr(i.length());

    return std::string();
}

void files::playback_progress(
        const pupnp::content_directory::item &item,
        std::chrono::milliseconds time)
{
    if (time.count() >= 0)
    {
        static const int increment = 15000;
        static const int delay = increment * 2;

        const auto rounded = ((time.count() / increment) * increment);
        if (rounded > delay)
            watchlist.set_last_position(item.uuid, std::chrono::milliseconds(rounded - delay), item.mrl);
    }
    else // finished
        watchlist.set_last_position(item.uuid, item.duration, item.mrl);
}
