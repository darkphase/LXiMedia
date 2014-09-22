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
static std::string filename();
static std::string make_uuid();

settings::settings(class messageloop &messageloop)
    : messageloop(messageloop),
      inifile(filename()),
      general(inifile.open_section("General")),
      dlna(inifile.open_section("DLNA")),
      media_player(inifile.open_section("Media Player")),
      timer(messageloop, std::bind(&inifile::save, &inifile)),
      save_delay(250)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };
}

settings::~settings()
{
}

std::string settings::uuid()
{
    auto value = general.read("UUID");
    if (value.empty())
    {
        value = make_uuid();
        general.write("UUID", value);
    }

    return value;
}

static std::string default_devicename();

std::string settings::devicename() const
{
    return general.read("DeviceName", default_devicename());
}

void settings::set_devicename(const std::string &devicename)
{
    if (devicename != default_devicename())
        return general.write("DeviceName", devicename);
    else
        return general.erase("DeviceName");
}

static const uint16_t default_http_port = 4280;

uint16_t settings::http_port() const
{
    return uint16_t(general.read("HttpPort", default_http_port));
}

void settings::set_http_port(uint16_t http_port)
{
    if (http_port != default_http_port)
        return general.write("HttpPort", http_port);
    else
        return general.erase("HttpPort");
}

bool settings::bind_all_networks() const
{
    return general.read("BindAllNetworks", false);
}

void settings::set_bind_all_networks(bool on)
{
    if (on)
        return general.write("BindAllNetworks", true);
    else
        return general.erase("BindAllNetworks");
}

bool settings::republish_rootdevice() const
{
    return general.read("RepublishRootDevice", true);
}

void settings::set_republish_rootdevice(bool on)
{
    if (on)
        return general.erase("RepublishRootDevice");
    else
        return general.write("RepublishRootDevice", false);
}

bool settings::allow_shutdown() const
{
    return general.read("AllowShutdown", true);
}

void settings::set_allow_shutdown(bool on)
{
    if (on)
        return general.erase("AllowShutdown");
    else
        return general.write("AllowShutdown", false);
}

static const char * to_string(encode_mode e)
{
    switch (e)
    {
    case encode_mode::fast: return "Fast";
    case encode_mode::slow: return "Slow";
    }

    assert(false);
    return nullptr;
}

static encode_mode to_encode_mode(const std::string &e)
{
    if      (e == "Fast")   return encode_mode::fast;
    else if (e == "Slow")   return encode_mode::slow;

    assert(false);
    return encode_mode::slow;
}

static enum encode_mode default_encode_mode()
{
    return (std::thread::hardware_concurrency() > 3) ? encode_mode::slow : encode_mode::fast;
}

enum encode_mode settings::encode_mode() const
{
    return to_encode_mode(dlna.read("EncodeMode", to_string(default_encode_mode())));
}

void settings::set_encode_mode(enum encode_mode encode_mode)
{
    if (encode_mode != default_encode_mode())
        return dlna.write("EncodeMode", to_string(encode_mode));
    else
        return dlna.erase("EncodeMode");
}

static const char * to_string(video_mode e)
{
    switch (e)
    {
    case video_mode::auto_      : return "Auto";
    case video_mode::vcd        : return "VCD";
    case video_mode::dvd        : return "DVD";
    case video_mode::hdtv_720   : return "720p";
    case video_mode::hdtv_1080  : return "1080p";
    }

    assert(false);
    return nullptr;
}

static video_mode to_video_mode(const std::string &e)
{
    if      (e == "Auto")       return video_mode::auto_;
    else if (e == "VCD")        return video_mode::vcd;
    else if (e == "DVD")        return video_mode::dvd;
    else if (e == "720p")       return video_mode::hdtv_720;
    else if (e == "1080p")      return video_mode::hdtv_1080;

    assert(false);
    return video_mode::auto_;
}

static const enum video_mode default_video_mode = video_mode::dvd;

enum video_mode settings::video_mode() const
{
    return to_video_mode(dlna.read("VideoMode", to_string(default_video_mode)));
}

void settings::set_video_mode(enum video_mode video_mode)
{
    if (video_mode != default_video_mode)
        return dlna.write("VideoMode", to_string(video_mode));
    else
        return dlna.erase("VideoMode");
}

static const char * to_string(canvas_mode e)
{
    switch (e)
    {
    case canvas_mode::none  : return "None";
    case canvas_mode::pad   : return "Pad";
    case canvas_mode::crop  : return "Crop";
    }

    assert(false);
    return nullptr;
}

static canvas_mode to_canvas_mode(const std::string &e)
{
    if      (e == "None")   return canvas_mode::none;
    else if (e == "Pad")    return canvas_mode::pad;
    else if (e == "Crop")   return canvas_mode::crop;

    assert(false);
    return canvas_mode::none;
}

bool settings::canvas_mode_enabled() const
{
    // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
    return vlc::instance::compare_version(2, 1) != 0;
}

static enum canvas_mode default_canvas_mode = canvas_mode::pad;

enum canvas_mode settings::canvas_mode() const
{
    if (!canvas_mode_enabled())
        return ::canvas_mode::none;

    return to_canvas_mode(dlna.read("CanvasMode", to_string(default_canvas_mode)));
}

void settings::set_canvas_mode(enum canvas_mode canvas_mode)
{
    if (canvas_mode != default_canvas_mode)
        return dlna.write("CanvasMode", to_string(canvas_mode));
    else
        return dlna.erase("CanvasMode");
}

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
    if      (e == "Stereo") return surround_mode::stereo;
    else if (e == "5.1")    return surround_mode::surround51;

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

    return to_surround_mode(dlna.read("SurroundMode", to_string(default_surround_mode)));
}

void settings::set_surround_mode(enum surround_mode surround_mode)
{
    if (surround_mode != default_surround_mode)
        return dlna.write("SurroundMode", to_string(surround_mode));
    else
        return dlna.erase("SurroundMode");
}

bool settings::mpeg2_enabled() const
{
    return dlna.read("CODEC_MPEG2", true);
}

void settings::set_mpeg2_enabled(bool on)
{
    if (on)
        return dlna.erase("CODEC_MPEG2");
    else
        return dlna.write("CODEC_MPEG2", false);
}

static bool default_mpeg4_enabled()
{
    return std::thread::hardware_concurrency() > 3;
}

bool settings::mpeg4_enabled() const
{
    return dlna.read("CODEC_H264", default_mpeg4_enabled());
}

void settings::set_mpeg4_enabled(bool on)
{
    if (on == default_mpeg4_enabled())
        return dlna.erase("CODEC_H264");
    else
        return dlna.write("CODEC_H264", on);
}

bool settings::video_mpegm2ts_enabled() const
{
    return dlna.read("FORMAT_M2TS", true);
}

void settings::set_video_mpegm2ts_enabled(bool on)
{
    if (on)
        return dlna.erase("FORMAT_M2TS");
    else
        return dlna.write("FORMAT_M2TS", false);
}

bool settings::video_mpegts_enabled() const
{
    return dlna.read("FORMAT_TS", true);
}

void settings::set_video_mpegts_enabled(bool on)
{
    if (on)
        return dlna.erase("FORMAT_TS");
    else
        return dlna.write("FORMAT_TS", false);
}

bool settings::video_mpeg_enabled() const
{
    return dlna.read("FORMAT_PS", true);
}

void settings::set_video_mpeg_enabled(bool on)
{
    if (on)
        return dlna.erase("FORMAT_PS");
    else
        return dlna.write("FORMAT_PS", false);
}

bool settings::verbose_logging_enabled() const
{
    return dlna.read("VerboseLogging", false);
}

void settings::set_verbose_logging_enabled(bool on)
{
    if (on)
        return dlna.write("VerboseLogging", true);
    else
        return dlna.erase("VerboseLogging");
}

static const char * to_string(path_type e)
{
    switch (e)
    {
    case path_type::auto_   : return "Auto";
    case path_type::music   : return "Music";
    case path_type::pictures: return "Pictures";
    case path_type::videos  : return "Videos";
    }

    assert(false);
    return nullptr;
}

static path_type to_path_type(const std::string &e)
{
    if      (e == "Auto")       return path_type::auto_;
    else if (e == "Music")      return path_type::music;
    else if (e == "Pictures")   return path_type::pictures;
    else if (e == "Videos")     return path_type::videos;

    assert(false);
    return path_type::auto_;
}

static std::string to_string(const std::vector<root_path> &paths)
{
    std::ostringstream str;
    for (auto &i : paths)
    {
#if defined(__unix__)
        str << ", \"" << to_string(i.type) << ",file://" << i.path << "\"";
#elif defined(WIN32)
        std::string path = i.path;
        std::replace(path.begin(), path.end(), '/', '\\');
        str << ", \"" << to_string(i.type) << ",file:" << path << "\"";
#endif
    }

    const std::string string = str.str();
    return !string.empty() ? string.substr(2) : string;
}

static std::vector<root_path> to_root_paths(const std::string &str)
{
    std::vector<std::string> entries;
    bool open = false;
    for (char c : str)
    {
        if (c == '\"')
        {
            if (!open)
            {
                open = true;
                entries.emplace_back();
            }
            else
                open = false;
        }
        else if (open)
            entries.back().push_back(c);
    }

    std::vector<root_path> result;
    for (const std::string &i : entries)
    {
        const size_t comma = i.find_first_of(',');
        if (comma != i.npos)
        {
            const std::string type = i.substr(0, comma);
            const std::string path = i.substr(comma + 1);

            if (starts_with(path, "file://"))
                result.emplace_back(root_path { to_path_type(type), path.substr(7) });
            else if (starts_with(path, "file:"))
                result.emplace_back(root_path { to_path_type(type), path.substr(5) });

#if defined(WIN32)
            std::replace(result.back().path.begin(), result.back().path.end(), '\\', '/');
#endif
        }
    }

    return result;
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
    return to_root_paths(media_player.read("RootPaths", to_string(default_root_paths())));
}

void settings::set_root_paths(const std::vector<root_path> &paths)
{
    const std::string string = to_string(paths);
    if (!string.empty() && (string != to_string(default_root_paths())))
        return media_player.write("RootPaths", string);
    else
        return media_player.erase("RootPaths");
}

#if defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static std::string home_dir()
{
    const char *home = getenv("HOME");
    if (home)
        return clean_path(home);

    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir)
        return clean_path(pw->pw_dir);

    return std::string();
}

static std::string unquote_and_replace_home(const std::string &src, const std::string &home)
{
    std::string result;
    const size_t fq = src.find_first_of('\"');
    const size_t sq = src.find_first_of('\"', fq + 1);
    if ((fq != src.npos) && (sq != src.npos))
        result = clean_path(src.substr(fq + 1, sq - fq - 1));
    else
        result = clean_path(src);

    const size_t h = result.find("$HOME");
    if (h != result.npos)
        result.replace(h, 5, home);

    if (!result.empty() && (result[result.length() - 1] != '/'))
        result.push_back('/');

    return result;
}

static struct user_dirs user_dirs()
{
    const std::string home = home_dir();
    const class inifile inifile(home + "/.config/user-dirs.dirs");
    const auto section = inifile.open_section();

    struct user_dirs user_dirs;
    user_dirs.download  = unquote_and_replace_home(section.read("XDG_DOWNLOAD_DIR"), home);
    user_dirs.music     = unquote_and_replace_home(section.read("XDG_MUSIC_DIR"), home);
    user_dirs.pictures  = unquote_and_replace_home(section.read("XDG_PICTURES_DIR"), home);
    user_dirs.videos    = unquote_and_replace_home(section.read("XDG_VIDEOS_DIR"), home);

    return user_dirs;
}

#ifndef TEST_H
static std::string filename()
{
    return home_dir() + "/.config/LeX-Interactive/LXiMediaCenter.conf";
}
#endif // TEST_H

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

static std::string default_devicename()
{
    std::string default_devicename = "LXiMediaCenter";

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
                user_dirs.download = clean_path(from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &path) == S_OK)
            {
                user_dirs.music = clean_path(from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &path) == S_OK)
            {
                user_dirs.pictures = clean_path(from_windows_path(path)) + '/';
                CoTaskMemFree(path);
                path = NULL;
            }

            if (SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, &path) == S_OK)
            {
                user_dirs.videos = clean_path(from_windows_path(path)) + '/';
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
                    user_dirs.music = clean_path(from_windows_path(path)) + '/';

                if (SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
                    user_dirs.pictures = clean_path(from_windows_path(path)) + '/';

                if (SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
                    user_dirs.videos = clean_path(from_windows_path(path)) + '/';
            }
        }

        FreeLibrary(shell32);
    }

    return user_dirs;
}

#ifndef TEST_H
static std::string filename()
{
    static const wchar_t confdir[] = L"\\LeX-Interactive\\";
    static const wchar_t conffile[] = L"LXiMediaCenter.conf";

    const wchar_t * const appdata = _wgetenv(L"APPDATA");
    if (appdata)
    {
        _wmkdir((std::wstring(appdata) + confdir).c_str());
        return from_windows_path(std::wstring(appdata) + confdir + conffile);
    }

    return std::string();
}
#endif // TEST_H

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

static std::string default_devicename()
{
    std::string default_devicename = "LXiMediaCenter";

    const wchar_t *hostname = _wgetenv(L"COMPUTERNAME");
    if (hostname)
        default_devicename = from_utf16(hostname) + " : " + default_devicename;

    return default_devicename;
}
#endif
