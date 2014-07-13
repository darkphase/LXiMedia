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
#include <set>
#include <sstream>

static std::vector<std::string> list_files(const std::string &path);

mediaplayer::mediaplayer(class messageloop &messageloop, class vlc::instance &vlc_instance, pupnp::connection_manager &connection_manager, pupnp::content_directory &content_directory)
  : messageloop(messageloop),
    vlc_instance(vlc_instance),
    connection_manager(connection_manager),
    content_directory(content_directory),
    root_path('/' + tr("Media Player") + '/')
{
  content_directory.item_source_register(root_path, *this);

  root_paths.push_back("/data/Audio/");
}

mediaplayer::~mediaplayer()
{
  content_directory.item_source_unregister(root_path);
}

std::vector<pupnp::content_directory::item> mediaplayer::list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count)
{
  const bool return_all = count == 0;
  std::vector<pupnp::content_directory::item> result;

  std::vector<std::string> files;
  if (path == root_path)
  {
    for (auto &i : root_paths)
    {
      const size_t lsl = std::max(i.find_last_of('/'), i.length() - 1);
      const size_t psl = i.find_last_of('/', lsl - 1);
      if (psl < lsl)
        files.emplace_back(i.substr(psl + 1, lsl - psl - 1) + '/');
    }
  }
  else if (starts_with(path, root_path))
    files = std::move(list_files(to_system_path(path)));

  for (auto &file : files)
  if (return_all || (count > 0))
  {
    if (start == 0)
    {
      result.emplace_back(get_contentdir_item(client, path + file));
      if (count > 0)
        count--;
    }
    else
      start--;
  }

  count = result.size();
  return result;
}

pupnp::content_directory::item mediaplayer::get_contentdir_item(const std::string &, const std::string &path)
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

    vlc::media media(vlc_instance, to_system_path(path));
    for (auto &track : media.tracks())
    {
      switch (track.type)
      {
      case vlc::media::track_type::audio:
        if (!item.is_audio() && !item.is_video())
          item.type = pupnp::content_directory::item_type::audio;

        item.sample_rate = track.audio.sample_rate;
        item.channels = track.audio.channels;
        break;

      case vlc::media::track_type::video:
        if (!item.is_video())
          item.type = pupnp::content_directory::item_type::video;

        item.width = track.video.width;
        item.height = track.video.height;
        item.frame_rate = track.video.frame_rate;
        break;
      }
    }

    if (item.is_audio() || item.is_video())
      item.mrl = media.mrl();
  }

  return item;
}

int mediaplayer::play_item(const pupnp::content_directory::item &item, const std::string &profile, std::string &content_type, std::shared_ptr<std::istream> &response)
{
  const auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
  if (!protocol.profile.empty())
  {
    std::ostringstream transcode;
    if (!protocol.acodec.empty() || !protocol.vcodec.empty())
    {
      transcode << "#lximedia_transcode{";

      if (!protocol.vcodec.empty())
      {
        transcode
            << "vfilter=canvas{width=" << protocol.width
            << ",height=" << protocol.height
            << ",aspect=16:9},"
            << protocol.vcodec
            << ",width=" << protocol.width
            << ",height=" << protocol.height
            << ",fps=" << protocol.frame_rate;
      }

      if (!protocol.acodec.empty())
      {
        if (!protocol.vcodec.empty())
          transcode << ",audio-sync=true,";

        transcode
            << protocol.acodec
            << ",samplerate=" << protocol.sample_rate
            << ",channels=" << protocol.channels;
      }

      transcode << '}';
    }

    struct transcode_stream : vlc::transcode_stream
    {
      transcode_stream(
            class vlc::instance &instance,
            pupnp::connection_manager &connection_manager,
            const pupnp::connection_manager::protocol &protocol)
        : vlc::transcode_stream(instance),
          connection_manager(connection_manager)
      {
        connection_manager.output_connection_add(*this, protocol);
      }

      ~transcode_stream()
      {
        connection_manager.output_connection_remove(*this);
      }

      pupnp::connection_manager &connection_manager;
    };

    auto stream = std::make_shared<transcode_stream>(vlc_instance, connection_manager, protocol);
    if (stream->open(item.mrl, transcode.str(), protocol.mux))
    {
      content_type = protocol.content_format;
      response = stream;
      return pupnp::upnp::http_ok;
    }
  }

  return pupnp::upnp::http_not_found;
}

std::string mediaplayer::to_system_path(const std::string &virtual_path) const
{
  if (starts_with(virtual_path, root_path))
  {
    const std::string path = virtual_path.substr(root_path.length());
    const std::string root = path.substr(0, path.find_first_of('/'));
    for (auto &i : root_paths)
    {
      const size_t lsl = std::max(i.find_last_of('/'), i.length() - 1);
      const size_t psl = i.find_last_of('/', lsl - 1);
      const std::string name = i.substr(psl + 1, lsl - psl - 1);
      if (root == name)
        return i + path.substr(root.length() + 1);
    }
  }

  return std::string();
}

std::string mediaplayer::to_virtual_path(const std::string &system_path) const
{
  for (auto &i : root_paths)
  if (starts_with(system_path, i))
  {
    const size_t lsl = std::max(i.find_last_of('/'), i.length() - 1);
    const size_t psl = i.find_last_of('/', lsl - 1);
    const std::string name = i.substr(psl + 1, lsl - psl - 1);

    return root_path + '/' + name + '/' + system_path.substr(i.length());
  }

  return std::string();
}

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static std::vector<std::string> list_files(const std::string &path)
{
  std::set<std::string> dirs, files;

  auto dir = ::opendir(path.c_str());
  if (dir)
  {
    for (auto dirent = ::readdir(dir); dirent; dirent = ::readdir(dir))
    {
      struct stat stat;
      if ((dirent->d_name[0] != '.') &&
          (::stat((path + '/' + dirent->d_name).c_str(), &stat) == 0))
      {
        if (S_ISDIR(stat.st_mode))
          dirs.emplace(std::string(dirent->d_name) + '/');
        else
          files.emplace(dirent->d_name);
      }
    }

    ::closedir(dir);
  }

  std::vector<std::string> result;
  result.reserve(dirs.size() + files.size());
  for (auto &i : dirs) result.emplace_back(std::move(i));
  for (auto &i : files) result.emplace_back(std::move(i));
  return result;
}
#endif
