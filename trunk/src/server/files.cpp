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
#include "mpeg/m2ts_filter.h"
#include "mpeg/ps_filter.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"
#include "vlc/image_stream.h"
#include "vlc/media.h"
#include "vlc/transcode_stream.h"
#include "watchlist.h"
#include <algorithm>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using std::chrono::duration_cast;

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
      media_cache(),
      connection_manager(connection_manager),
      content_directory(content_directory),
      recommended(recommended),
      settings(settings),
      watchlist(watchlist),
      basedir('/' + tr("Files") + '/'),
      min_parse_time(3000),
      max_parse_time(30000),
      item_parse_time(500)
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
static track_list list_tracks(const struct vlc::media_cache::media_info &media_info)
{
    std::vector<vlc::media_cache::track> video, audio, text;
    vlc::media_cache::track none;
    none.type = vlc::track_type::unknown;
    text.push_back(none);

    for (auto &track : media_info.tracks)
        switch (track.type)
        {
        case vlc::track_type::unknown:  break;
        case vlc::track_type::video:    video.push_back(track); break;
        case vlc::track_type::audio:    audio.push_back(track); break;
        case vlc::track_type::text:     text.push_back(track);  break;
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

                if (video[v].type != vlc::track_type::unknown)
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

                if (audio[a].type != vlc::track_type::unknown)
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
                if (text[t].type != vlc::track_type::unknown)
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
    const bool return_all = count == 0;

    const auto &files = list_files(path, start == 0);

    std::vector<std::string> paths;
    for (auto &file : files)
        if (return_all || (count > 0))
        {
            if (start == 0)
            {
                paths.emplace_back(path + file);

                if (count > 0)
                    count--;
            }
            else
                start--;
        }
        else
            break;

    std::vector<std::future<pupnp::content_directory::item>> futures;
    for (auto &path : paths)
    {
        futures.emplace_back(std::async(
            std::launch::async,
            std::bind(&files::make_item, this, client, path, true)));
    }

    std::vector<pupnp::content_directory::item> result;
    for (auto &future : futures)
        result.emplace_back(future.get());

    count = files.size();
    return result;
}

std::vector<pupnp::content_directory::item> files::list_recommended_items(
        const std::string &client,
        size_t start, size_t &count)
{
    const bool return_all = count == 0;

    const auto watched_items = watchlist.watched_items();
    std::unordered_map<std::string, const watchlist::entry &> watched_by_mrl;
    std::map<std::chrono::system_clock::time_point, const watchlist::entry &> watched_by_last_seen;
    for (auto &i : watched_items)
    {
        watched_by_mrl.emplace(i.mrl, i);
        watched_by_last_seen.emplace(i.last_seen, i);
    }

    std::unordered_set<std::string> directories;
    for (auto i = watched_by_last_seen.rbegin();
         (i != watched_by_last_seen.rend()) && (directories.size() < 8);
         i++)
    {
        const std::string &mrl = i->second.mrl;

#if defined(__unix__) || defined(__APPLE__)
        if (starts_with(mrl, "file://"))
        {
            std::string path = from_percent(mrl.substr(7));
#elif defined(WIN32)
        if (starts_with(mrl, "file:"))
        {
            std::string path = from_percent(mrl.substr(5));
            std::replace(path.begin(), path.end(), '\\', '/');
#endif

            const auto ls = path.find_last_of('/');
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

    const auto now = std::chrono::system_clock::now();
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
                auto watched = watched_by_mrl.find(media.mrl());
                if ((watched != watched_by_mrl.end()) && watchlist.watched_till_end(watched->second))
                {
                    last_media = vlc::media();
                    last_path.clear();
                    last_score = 0.0;
                    last_seen = duration_cast<std::chrono::minutes>(now - watched->second.last_seen);
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
            recommended_paths.emplace(last_score, std::move(last_path));
    }

    std::vector<std::future<pupnp::content_directory::item>> futures;
    for (auto &path : recommended_paths)
        if (return_all || (count > 0))
        {
            if (start == 0)
            {
                futures.emplace_back(std::async(
                    std::launch::async,
                    std::bind(&files::make_item, this, client, path.second, true)));

                if (count > 0)
                    count--;
            }
            else
                start--;
        }

    std::vector<pupnp::content_directory::item> result;
    for (auto &future : futures)
        result.emplace_back(future.get());

    count = recommended_paths.size();
    return result;
}

pupnp::content_directory::item files::get_contentdir_item(const std::string &client, const std::string &path)
{
    return make_item(client, path, false);
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
    if ((settings.canvas_mode() == canvas_mode::none) || item.is_image())
    {
        min_scale(
                    protocol.vcodec,
                    item.width, item.height,
                    protocol.width, protocol.height,
                    protocol.width, protocol.height);
    }

    return true;
}

int files::play_audio_video_item(
        const std::string &source_address,
        const pupnp::content_directory::item &item,
        const pupnp::connection_manager::protocol &protocol,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    using namespace std::placeholders;

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
        std::clog << "files: creating new stream " << item.mrl << " transcode=" << transcode.str() << " mux=" << protocol.mux << std::endl;

        struct vlc::transcode_stream::track_ids track_ids;
        std::string file_path, track_name;
        split_path(item.path, file_path, track_name);
        const auto system_path = to_system_path(file_path);
        auto media = vlc::media::from_file(vlc_instance, system_path.path);

        for (auto &track : list_tracks(media_cache.media_info(media)))
            if (track.first == track_name)
                for (auto &t : track.second)
                    switch (t.type)
                    {
                    case vlc::track_type::unknown:  break;
                    case vlc::track_type::audio:    track_ids.audio = t.id; break;
                    case vlc::track_type::video:    track_ids.video = t.id; break;
                    case vlc::track_type::text:     track_ids.text  = t.id; break;
                    }

        std::unique_ptr<vlc::transcode_stream> stream(new vlc::transcode_stream(messageloop, vlc_instance));
        const auto started = std::chrono::system_clock::now();
        stream->on_playback_progress = std::bind(&files::playback_progress, this, item, started, _1);

        const std::string vlc_mux = (protocol.mux == "m2ts") ? "ts" : protocol.mux;

        if ((item.chapter > 0)
                ? stream->open(item.mrl, item.chapter, track_ids, transcode.str(), vlc_mux, rate)
                : stream->open(item.mrl, item.position, track_ids, transcode.str(), vlc_mux, rate))
        {
            std::shared_ptr<pupnp::connection_proxy> proxy;
            if (protocol.mux == "ps")
            {
                std::unique_ptr<mpeg::ps_filter> filter(new mpeg::ps_filter(std::move(stream)));
                proxy = std::make_shared<pupnp::connection_proxy>(std::move(filter));
            }
            else if (protocol.mux == "m2ts")
            {
                std::unique_ptr<mpeg::m2ts_filter> filter(new mpeg::m2ts_filter(std::move(stream)));
                proxy = std::make_shared<pupnp::connection_proxy>(std::move(filter));
            }
            else
                proxy = std::make_shared<pupnp::connection_proxy>(std::move(stream));

            connection_manager.add_output_connection(proxy, protocol, item.mrl, source_address, opt.str());
            response = proxy;
        }
    }

    if (response)
    {
        content_type = protocol.content_format;
        return pupnp::upnp::http_ok;
    }

    return pupnp::upnp::http_not_found;
}

int files::get_image_item(
        const pupnp::content_directory::item &item,
        const pupnp::connection_manager::protocol &protocol,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    auto stream = std::make_shared<vlc::image_stream>(vlc_instance);
    if (stream->open(
                item.mrl,
                protocol.content_format,
                protocol.width,
                protocol.height))
    {
        content_type = protocol.content_format;
        response = stream;

        return pupnp::upnp::http_ok;
    }

    return pupnp::upnp::http_not_found;
}

int files::play_item(
        const std::string &source_address,
        const pupnp::content_directory::item &item,
        const std::string &profile,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    if (item.is_audio())
    {
        auto protocol = connection_manager.get_protocol(profile, item.channels);
        if (!protocol.profile.empty() && correct_protocol(item, protocol))
            return play_audio_video_item(source_address, item, protocol, content_type, response);
    }
    else if (item.is_video())
    {
        auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
        if (!protocol.profile.empty() && correct_protocol(item, protocol))
            return play_audio_video_item(source_address, item, protocol, content_type, response);
    }
    else if (item.is_image())
    {
        auto protocol = connection_manager.get_protocol(profile, item.width, item.height);
        if (!protocol.profile.empty() && correct_protocol(item, protocol))
            return get_image_item(item, protocol, content_type, response);
    }

    return pupnp::upnp::http_not_found;
}

static void fill_item(
        pupnp::content_directory::item &item,
        vlc::media_type media_type,
        const class vlc::media_cache::media_info &media_info,
        const std::vector<vlc::media_cache::track> &tracks,
        ::path_type path_type)
{
    for (auto &track : tracks)
        switch (track.type)
        {
        case vlc::track_type::unknown:
            break;

        case vlc::track_type::audio:
            if (media_type == vlc::media_type::audio)
                switch (path_type)
                {
                case ::path_type::auto_:
                case ::path_type::pictures:
                case ::path_type::videos:
                    if (item.duration >= std::chrono::minutes(50))
                        item.type = pupnp::content_directory::item_type::audio_book;
                    else if (item.duration < std::chrono::minutes(5))
                        item.type = pupnp::content_directory::item_type::music;
                    else
                        item.type = pupnp::content_directory::item_type::audio;

                    break;

                case ::path_type::music:
                    item.type = pupnp::content_directory::item_type::music;
                    break;
                }

            item.sample_rate = track.audio.sample_rate;
            item.channels = track.audio.channels;
            break;

        case vlc::track_type::video:
            if (media_type == vlc::media_type::video)
            {
                switch (path_type)
                {
                case ::path_type::auto_:
                case ::path_type::pictures:
                case ::path_type::videos:
                    if (item.duration >= std::chrono::minutes(50))
                        item.type = pupnp::content_directory::item_type::movie;
                    else
                        item.type = pupnp::content_directory::item_type::video;

                    break;

                case ::path_type::music:
                    item.type = pupnp::content_directory::item_type::music_video;
                    break;
                }

                item.frame_rate = track.video.frame_rate;
            }
            else if (media_type == vlc::media_type::picture)
            {
                switch (path_type)
                {
                case ::path_type::auto_:
                case ::path_type::music:
                case ::path_type::videos:
                    item.type = pupnp::content_directory::item_type::image;
                    break;

                case ::path_type::pictures:
                    item.type = pupnp::content_directory::item_type::photo;
                    break;
                }
            }

            item.width = track.video.width;
            item.height = track.video.height;
            break;

        case vlc::track_type::text:
            break;
        }

    if (item.is_audio() || item.is_video())
    {
        item.duration = media_info.duration;

        for (int i = 0, n = media_info.chapter_count; i < n; i++)
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

                for (auto &track : list_tracks(media_cache.media_info(media)))
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

pupnp::content_directory::item files::make_item(const std::string &client, const std::string &path, bool fast) const
{
    std::string file_path, track_name;
    split_path(path, file_path, track_name);

    struct pupnp::content_directory::item item;
    item.is_dir = !file_path.empty() && (file_path[file_path.length() - 1] == '/');
    item.path = path;

    if (item.is_dir)
    {
        const size_t lsl = std::max(path.find_last_of('/'), path.length() - 1);
        const size_t psl = path.find_last_of('/', lsl - 1);
        item.title = path.substr(psl, lsl - psl);
    }
    else
    {
        const size_t lsl = path.find_last_of('/');
        item.title = path.substr(lsl + 1);

        const auto system_path = to_system_path(file_path);
        auto media = vlc::media::from_file(vlc_instance, system_path.path);

        const auto media_type = media_cache.media_type(media);
        if (media_type != vlc::media_type::unknown)
        {
            if (!fast)
            {
                const auto uuid = media_cache.uuid(media.mrl());
                item.last_position = watchlist.watched_item(uuid).last_position;

                const auto media_info = media_cache.media_info(media);
                auto tracks = list_tracks(media_info);
                if (!track_name.empty())
                {
                    for (auto &track : tracks)
                        if (track.first == track_name)
                        {
                            item.mrl = media.mrl();
                            item.uuid = uuid;

                            fill_item(item, media_type, media_info, track.second, system_path.type);
                            break;
                        }
                }
                else if (tracks.size() > 1)
                {
                    item.is_dir = true;
                    item.path = file_path + "//";
                    item.uuid = uuid;
                    item.duration = media_info.duration;
                }
                else if (tracks.size() == 1)
                {
                    item.mrl = media.mrl();
                    item.uuid = uuid;
                    fill_item(item, media_type, media_info, tracks.front().second, system_path.type);
                }
            }
            else if ((media_type != vlc::media_type::video) ||
                     (system_path.type == path_type::music) ||
                     (system_path.type == path_type::pictures))
            {
                const auto media_info = media_cache.media_info(media);

                item.mrl = media.mrl();
                item.uuid = media_cache.uuid(item.mrl);
                fill_item(item, media_type, media_info, media_info.tracks, system_path.type);
            }
            else
            {
                item.is_dir = true;
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
        std::chrono::system_clock::time_point started,
        std::chrono::milliseconds time)
{
    if (time.count() >= 0)
    {
        static const int increment = 15000;
        static const int delay = increment * 2;

        const auto rounded = ((time.count() / increment) * increment);
        if (rounded > delay)
        {
            const std::chrono::milliseconds time(rounded - delay);
            watchlist.set_watched_item(item.uuid, watchlist::entry { started, time, item.duration, item.mrl });
        }
    }
    else // finished
        watchlist.set_watched_item(item.uuid, watchlist::entry { started, item.duration, item.duration, item.mrl });
}
