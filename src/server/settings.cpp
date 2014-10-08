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

#include "settings.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include "platform/string.h"
#include "vlc/instance.h"
#include <algorithm>
#include <cassert>
#include <sstream>
#include <thread>

struct user_dirs { std::string download, music, pictures, videos; };
static struct user_dirs user_dirs();
static std::string make_uuid();

settings::settings(class platform::messageloop_ref &messageloop)
    : messageloop(messageloop),
      inifile(platform::config_dir() + "/settings"),
      general(inifile.open_section()),
      codecs(inifile.open_section("codecs")),
      formats(inifile.open_section("formats")),
      paths(inifile.open_section("paths")),
      timer(messageloop, std::bind(&settings::save, this)),
      save_delay(250)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };
}

settings::~settings()
{
}

void settings::save()
{
    timer.stop();
    inifile.save();
}

static const char is_configure_required_name[] = "is_configure_required";

bool settings::is_configure_required() const
{
    return general.read(is_configure_required_name, true);
}

void settings::set_configure_required(bool on)
{
    if (on)
        return general.erase(is_configure_required_name);
    else
        return general.write(is_configure_required_name, false);
}

static const char uuid_name[] = "uuid";

std::string settings::uuid()
{
    auto value = general.read(uuid_name);
    if (value.empty())
    {
        value = make_uuid();
        general.write(uuid_name, value);
    }

    return value;
}

static const char upnp_devicename_name[] = "upnp_devicename";

static std::string default_upnp_devicename();

std::string settings::upnp_devicename() const
{
    return general.read(upnp_devicename_name, default_upnp_devicename());
}

void settings::set_upnp_devicename(const std::string &upnp_devicename)
{
    if (upnp_devicename != default_upnp_devicename())
        return general.write(upnp_devicename_name, upnp_devicename);
    else
        return general.erase(upnp_devicename_name);
}

static const char http_port_name[] = "http_port";

static const uint16_t default_http_port = 4280;

uint16_t settings::http_port() const
{
    return uint16_t(general.read(http_port_name, default_http_port));
}

void settings::set_http_port(uint16_t http_port)
{
    if (http_port != default_http_port)
        return general.write(http_port_name, http_port);
    else
        return general.erase(http_port_name);
}

static const char bind_all_networks_name[] = "bind_all_networks";

bool settings::bind_all_networks() const
{
    return general.read(bind_all_networks_name, false);
}

void settings::set_bind_all_networks(bool on)
{
    if (on)
        return general.write(bind_all_networks_name, true);
    else
        return general.erase(bind_all_networks_name);
}

static const char republish_rootdevice_name[] = "republish_rootdevice";

bool settings::republish_rootdevice() const
{
    return general.read(republish_rootdevice_name, true);
}

void settings::set_republish_rootdevice(bool on)
{
    if (on)
        return general.erase(republish_rootdevice_name);
    else
        return general.write(republish_rootdevice_name, false);
}

static const char encode_mode_name[] = "encode_mode";

static const char * to_string(encode_mode e)
{
    switch (e)
    {
    case encode_mode::fast: return "fast";
    case encode_mode::slow: return "slow";
    }

    assert(false);
    return nullptr;
}

static encode_mode to_encode_mode(const std::string &e)
{
    if      (e == to_string(encode_mode::fast))   return encode_mode::fast;
    else if (e == to_string(encode_mode::slow))   return encode_mode::slow;

    assert(false);
    return encode_mode::slow;
}

static enum encode_mode default_encode_mode()
{
    return (std::thread::hardware_concurrency() > 3) ? encode_mode::slow : encode_mode::fast;
}

enum encode_mode settings::encode_mode() const
{
    return to_encode_mode(general.read(encode_mode_name, to_string(default_encode_mode())));
}

void settings::set_encode_mode(enum encode_mode encode_mode)
{
    if (encode_mode != default_encode_mode())
        return general.write(encode_mode_name, to_string(encode_mode));
    else
        return general.erase(encode_mode_name);
}

static const char video_mode_name[] = "video_mode";

static const char * to_string(video_mode e)
{
    switch (e)
    {
    case video_mode::auto_      : return "auto";
    case video_mode::vcd        : return "vcd";
    case video_mode::dvd        : return "dvd";
    case video_mode::hdtv_720   : return "720p";
    case video_mode::hdtv_1080  : return "1080p";
    }

    assert(false);
    return nullptr;
}

static video_mode to_video_mode(const std::string &e)
{
    if      (e == to_string(video_mode::auto_))     return video_mode::auto_;
    else if (e == to_string(video_mode::vcd))       return video_mode::vcd;
    else if (e == to_string(video_mode::dvd))       return video_mode::dvd;
    else if (e == to_string(video_mode::hdtv_720))  return video_mode::hdtv_720;
    else if (e == to_string(video_mode::hdtv_1080)) return video_mode::hdtv_1080;

    assert(false);
    return video_mode::auto_;
}

static const enum video_mode default_video_mode = video_mode::dvd;

enum video_mode settings::video_mode() const
{
    return to_video_mode(general.read(video_mode_name, to_string(default_video_mode)));
}

void settings::set_video_mode(enum video_mode video_mode)
{
    if (video_mode != default_video_mode)
        return general.write(video_mode_name, to_string(video_mode));
    else
        return general.erase(video_mode_name);
}

static const char canvas_mode_name[] = "canvas_mode";

static const char * to_string(canvas_mode e)
{
    switch (e)
    {
    case canvas_mode::none  : return "none";
    case canvas_mode::pad   : return "pad";
    case canvas_mode::crop  : return "crop";
    }

    assert(false);
    return nullptr;
}

static canvas_mode to_canvas_mode(const std::string &e)
{
    if      (e == to_string(canvas_mode::none)) return canvas_mode::none;
    else if (e == to_string(canvas_mode::pad))  return canvas_mode::pad;
    else if (e == to_string(canvas_mode::crop)) return canvas_mode::crop;

    assert(false);
    return canvas_mode::none;
}

static enum canvas_mode default_canvas_mode = canvas_mode::pad;

enum canvas_mode settings::canvas_mode() const
{
    return to_canvas_mode(general.read(canvas_mode_name, to_string(default_canvas_mode)));
}

void settings::set_canvas_mode(enum canvas_mode canvas_mode)
{
    if (canvas_mode != default_canvas_mode)
        return general.write(canvas_mode_name, to_string(canvas_mode));
    else
        return general.erase(canvas_mode_name);
}

static const char surround_mode_name[] = "surround_mode";

static const char * to_string(surround_mode e)
{
    switch (e)
    {
    case surround_mode::stereo      : return "Stereo";
    case surround_mode::surround51  : return "5.1";
    }

    assert(false);
    return nullptr;
}

static surround_mode to_surround_mode(const std::string &e)
{
    if      (e == to_string(surround_mode::stereo))     return surround_mode::stereo;
    else if (e == to_string(surround_mode::surround51)) return surround_mode::surround51;

    assert(false);
    return surround_mode::stereo;
}

bool settings::surround_mode_enabled() const
{
    // Workaround for ticket https://trac.videolan.org/vlc/ticket/1897
    return vlc::instance::compare_version(2, 2) >= 0;
}

static enum surround_mode default_surround_mode = surround_mode::surround51;

enum surround_mode settings::surround_mode() const
{
    if (!surround_mode_enabled())
        return ::surround_mode::stereo;

    return to_surround_mode(general.read(surround_mode_name, to_string(default_surround_mode)));
}

void settings::set_surround_mode(enum surround_mode surround_mode)
{
    if (surround_mode != default_surround_mode)
        return general.write(surround_mode_name, to_string(surround_mode));
    else
        return general.erase(surround_mode_name);
}

static const char share_removable_media_name[] = "share_removable_media";

bool settings::share_removable_media() const
{
    return general.read(share_removable_media_name, true);
}

void settings::set_share_removable_media(bool on)
{
    if (!on)
        return general.write(share_removable_media_name, false);
    else
        return general.erase(share_removable_media_name);
}

static const char verbose_logging_name[] = "verbose_logging";

bool settings::verbose_logging_enabled() const
{
    return general.read(verbose_logging_name, false);
}

void settings::set_verbose_logging_enabled(bool on)
{
    if (on)
        return general.write(verbose_logging_name, true);
    else
        return general.erase(verbose_logging_name);
}

static const char mp2v_name[] = "mp2v";

bool settings::mpeg2_enabled() const
{
    return codecs.read(mp2v_name, true);
}

void settings::set_mpeg2_enabled(bool on)
{
    if (on)
        return codecs.erase(mp2v_name);
    else
        return codecs.write(mp2v_name, false);
}

static const char h264_name[] = "h264";

static bool default_mpeg4_enabled()
{
    return std::thread::hardware_concurrency() > 3;
}

bool settings::mpeg4_enabled() const
{
    return codecs.read(h264_name, default_mpeg4_enabled());
}

void settings::set_mpeg4_enabled(bool on)
{
    if (on == default_mpeg4_enabled())
        return codecs.erase(h264_name);
    else
        return codecs.write(h264_name, on);
}

static const char mpeg_m2ts_name[] = "mpeg_m2ts";

bool settings::video_mpegm2ts_enabled() const
{
    return formats.read(mpeg_m2ts_name, true);
}

void settings::set_video_mpegm2ts_enabled(bool on)
{
    if (on)
        return formats.erase(mpeg_m2ts_name);
    else
        return formats.write(mpeg_m2ts_name, false);
}

static const char mpeg_ts_name[] = "mpeg_ts";

bool settings::video_mpegts_enabled() const
{
    return formats.read(mpeg_ts_name, true);
}

void settings::set_video_mpegts_enabled(bool on)
{
    if (on)
        return formats.erase(mpeg_ts_name);
    else
        return formats.write(mpeg_ts_name, false);
}

static const char mpeg_ps_name[] = "mpeg_ps";

bool settings::video_mpeg_enabled() const
{
    return formats.read(mpeg_ps_name, true);
}

void settings::set_video_mpeg_enabled(bool on)
{
    if (on)
        return formats.erase(mpeg_ps_name);
    else
        return formats.write(mpeg_ps_name, false);
}

static const char * to_string(path_type e)
{
    switch (e)
    {
    case path_type::auto_   : return "auto";
    case path_type::music   : return "music";
    case path_type::pictures: return "pictures";
    case path_type::videos  : return "videos";
    }

    assert(false);
    return nullptr;
}

static path_type to_path_type(const std::string &e)
{
    if      (e == to_string(path_type::auto_))      return path_type::auto_;
    else if (e == to_string(path_type::music))      return path_type::music;
    else if (e == to_string(path_type::pictures))   return path_type::pictures;
    else if (e == to_string(path_type::videos))     return path_type::videos;

    assert(false);
    return path_type::auto_;
}

static std::vector<root_path> default_root_paths()
{
    std::vector<root_path> result;

    const auto user_dirs = ::user_dirs();
    if (!user_dirs.download .empty()) result.emplace_back(root_path { path_type::auto_      , user_dirs.download  });
    if (!user_dirs.music    .empty()) result.emplace_back(root_path { path_type::music      , user_dirs.music     });
    if (!user_dirs.pictures .empty()) result.emplace_back(root_path { path_type::pictures   , user_dirs.pictures  });
    if (!user_dirs.videos   .empty()) result.emplace_back(root_path { path_type::videos     , user_dirs.videos    });

    return result;
}

std::vector<root_path> settings::root_paths() const
{
    const auto names = paths.names();
    if (!names.empty())
    {
        std::vector<root_path> result;
        for (auto &i : paths.names())
            result.emplace_back(root_path { to_path_type(paths.read(i, to_string(path_type::auto_))), i });

        return result;
    }

    return default_root_paths();
}

void settings::set_root_paths(const std::vector<root_path> &root_paths)
{
    const auto default_paths = default_root_paths();

    size_t defaults = 0;
    std::set<std::string> new_names;
    for (auto &i : root_paths)
        if (new_names.find(i.path) == new_names.end())
        {
            new_names.insert(i.path);
            paths.write(i.path, to_string(i.type));

            for (auto &j : default_paths)
                if ((j.type == i.type) && (j.path == i.path))
                {
                    defaults++;
                    break;
                }
        }

    if ((defaults == default_paths.size()) && (defaults == root_paths.size()))
        new_names.clear();

    for (auto &i : paths.names())
        if (new_names.find(i) == new_names.end())
            paths.erase(i);
}

#if defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static std::string unquote_and_replace_home(const std::string &src, const std::string &home)
{
    std::string result;
    const size_t fq = src.find_first_of('\"');
    const size_t sq = src.find_first_of('\"', fq + 1);
    if ((fq != src.npos) && (sq != src.npos))
        result = platform::clean_path(src.substr(fq + 1, sq - fq - 1));
    else
        result = platform::clean_path(src);

    const size_t h = result.find("$HOME");
    if (h != result.npos)
        result.replace(h, 5, home);

    if (!result.empty() && (result[result.length() - 1] != '/'))
        result.push_back('/');

    return result;
}

static struct user_dirs user_dirs()
{
    const std::string home = platform::home_dir();
    const class platform::inifile inifile(home + "/.config/user-dirs.dirs");
    const auto section = inifile.open_section();

    struct user_dirs user_dirs;
    user_dirs.download  = unquote_and_replace_home(section.read("XDG_DOWNLOAD_DIR"), home);
    user_dirs.music     = unquote_and_replace_home(section.read("XDG_MUSIC_DIR"), home);
    user_dirs.pictures  = unquote_and_replace_home(section.read("XDG_PICTURES_DIR"), home);
    user_dirs.videos    = unquote_and_replace_home(section.read("XDG_VIDEOS_DIR"), home);

    return user_dirs;
}

#include <cstring>
#include <uuid/uuid.h>

static std::string make_uuid()
{
    uuid_t uuid;
    uuid_generate(uuid);

    std::string result;
    result.resize(64);
    uuid_unparse(uuid, &result[0]);
    result.resize(strlen(&result[0]));

    return result;
}

static std::string default_upnp_devicename()
{
    std::string default_devicename = "LXiMediaServer";

    char hostname[256] = { 0 };
    if (gethostname(hostname, sizeof(hostname) - 1) == 0)
        default_devicename = std::string(hostname) + " : " + default_devicename;

    return default_devicename;
}

#elif defined(WIN32)
#include <cstdlib>
#include <direct.h>
#include <shlobj.h>

static struct user_dirs user_dirs()
{
    struct user_dirs user_dirs;

    HMODULE shell32 = LoadLibrary(L"shell32.dll");
    if (shell32 != NULL)
    {
        typedef HRESULT WINAPI (* SHGetKnownFolderPathFunc)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
        auto SHGetKnownFolderPath = SHGetKnownFolderPathFunc(GetProcAddress(shell32, "SHGetKnownFolderPath"));
        if (SHGetKnownFolderPath)
        {
            PWSTR path = NULL;

            if (SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &path) == S_OK)
            {
                user_dirs.download = platform::clean_path(platform::from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &path) == S_OK)
            {
                user_dirs.music = platform::clean_path(platform::from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &path) == S_OK)
            {
                user_dirs.pictures = platform::clean_path(platform::from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, &path) == S_OK)
            {
                user_dirs.videos = platform::clean_path(platform::from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }
        }
        else // Windows XP and older
        {
            typedef HRESULT WINAPI(* SHGetFolderPathFunc)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
            auto SHGetFolderPath = SHGetFolderPathFunc(GetProcAddress(shell32, "SHGetFolderPathW"));
            if (SHGetFolderPath)
            {
                wchar_t path[MAX_PATH];

                if (SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
                    user_dirs.music = platform::clean_path(platform::from_windows_path(path)) + '/';

                if (SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
                    user_dirs.pictures = platform::clean_path(platform::from_windows_path(path)) + '/';

                if (SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
                    user_dirs.videos = platform::clean_path(platform::from_windows_path(path)) + '/';
            }
        }

        FreeLibrary(shell32);
    }

    return user_dirs;
}

#include <rpc.h>

static std::string make_uuid()
{
    UUID uuid;
    if (UuidCreate(&uuid) == RPC_S_UUID_NO_ADDRESS)
    {
        uuid.Data1 = (rand() << 16) ^ rand();
        uuid.Data2 = rand();
        uuid.Data3 = (rand() & 0x0FFF) | 0x4000;
        for (auto &i : uuid.Data4) i = rand();
        uuid.Data4[0] = (uuid.Data4[0] & 0x3F) | 0x80;
    }

    RPC_CSTR rpc_string;
    if (UuidToStringA(&uuid, &rpc_string) != RPC_S_OK)
        throw std::runtime_error("out of memory");

    const std::string result = reinterpret_cast<const char *>(rpc_string);
    RpcStringFreeA(&rpc_string);

    return result;
}

static std::string default_upnp_devicename()
{
    std::string default_devicename = "LXiMediaServer";

    const wchar_t *hostname = _wgetenv(L"COMPUTERNAME");
    if (hostname)
        default_devicename = from_utf16(hostname) + " : " + default_devicename;

    return default_devicename;
}
#endif
