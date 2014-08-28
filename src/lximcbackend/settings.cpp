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
#include "string.h"
#include "vlc/instance.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <thread>

static std::string filename();
static std::string make_uuid();

settings::settings(class messageloop &messageloop)
    : messageloop(messageloop),
      timer(messageloop, std::bind(&settings::save, this)),
      save_delay(250),
      touched(false)
{
    std::ifstream file(filename());
    for (std::string line, section; std::getline(file, line); )
    {
        auto o = line.find_first_of('[');
        if (o == 0)
        {
            auto c = line.find_first_of(']', o);
            if (c != line.npos)
                section = line.substr(o + 1, c - 1);

            continue;
        }

        auto e = line.find_first_of('=');
        if (e != line.npos)
            values[section][line.substr(0, e)] = line.substr(e + 1);
    }
}

settings::~settings()
{
    save();
}

void settings::save()
{
    if (touched)
    {
        std::ofstream file(filename());
        for (auto &section : values)
        {
            file << '[' << section.first << ']' << std::endl;
            for (auto &value : section.second)
                file << value.first << '=' << value.second << std::endl;

            file << std::endl;
        }
    }

    touched = false;
}

std::string settings::read(const std::string &section, const std::string &name, const std::string &def) const
{
    auto i = values.find(section);
    if (i != values.end())
    {
        auto j = i->second.find(name);
        if (j != i->second.end())
            return j->second;
    }

    return def;
}

void settings::write(const std::string &section, const std::string &name, const std::string &value)
{
    values[section][name] = value;
    touched = true;

    timer.start(save_delay, true);
}

void settings::erase(const std::string &section, const std::string &name)
{
    auto i = values.find(section);
    if (i != values.end())
    {
        auto j = i->second.find(name);
        if (j != i->second.end())
        {
            i->second.erase(j);
            if (i->second.empty())
                values.erase(i);

            touched = true;
            timer.start(save_delay, true);
        }
    }
}

std::string settings::uuid()
{
    auto value = read("General", "UUID", std::string());
    if (value.empty())
    {
        value = make_uuid();
        write("General", "UUID", value);
    }

    return value;
}

static std::string default_devicename();

std::string settings::devicename() const
{
    return read("General", "DeviceName", default_devicename());
}

void settings::set_devicename(const std::string &devicename)
{
    if (devicename != default_devicename())
        return write("General", "DeviceName", devicename);
    else
        return erase("General", "DeviceName");
}

static const uint16_t default_http_port = 4280;

uint16_t settings::http_port() const
{
    try { return uint16_t(std::stoi(read("General", "HttpPort", std::to_string(default_http_port)))); }
    catch (const std::invalid_argument &) { return default_http_port; }
    catch (const std::out_of_range &) { return default_http_port; }
}

void settings::set_http_port(uint16_t http_port)
{
    if (http_port != default_http_port)
        return write("General", "HttpPort", std::to_string(http_port));
    else
        return erase("General", "HttpPort");
}

bool settings::bindallnetworks() const
{
    return read("General", "BindAllNetworks", "false") != "false";
}

void settings::set_bindallnetworks(bool on)
{
    if (on)
        return write("General", "BindAllNetworks", "true");
    else
        return erase("General", "BindAllNetworks");
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
    return to_encode_mode(read("DLNA", "EncodeMode", to_string(default_encode_mode())));
}

void settings::set_encode_mode(enum encode_mode encode_mode)
{
    if (encode_mode != default_encode_mode())
        return write("DLNA", "EncodeMode", to_string(encode_mode));
    else
        return erase("DLNA", "EncodeMode");
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
    if      (e == "Auto")   return video_mode::auto_;
    else if (e == "VCD")    return video_mode::vcd;
    else if (e == "DVD")    return video_mode::dvd;
    else if (e == "720p")   return video_mode::hdtv_720;
    else if (e == "1080p")  return video_mode::hdtv_1080;

    assert(false);
    return video_mode::auto_;
}

static enum video_mode default_video_mode()
{
    return (std::thread::hardware_concurrency() > 1) ? video_mode::hdtv_720 : video_mode::dvd;
}

enum video_mode settings::video_mode() const
{
    return to_video_mode(read("DLNA", "VideoMode", to_string(default_video_mode())));
}

void settings::set_video_mode(enum video_mode video_mode)
{
    if (video_mode != default_video_mode())
        return write("DLNA", "VideoMode", to_string(video_mode));
    else
        return erase("DLNA", "VideoMode");
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

    return to_canvas_mode(read("DLNA", "CanvasMode", to_string(default_canvas_mode)));
}

void settings::set_canvas_mode(enum canvas_mode canvas_mode)
{
    if (canvas_mode != default_canvas_mode)
        return write("DLNA", "CanvasMode", to_string(canvas_mode));
    else
        return erase("DLNA", "CanvasMode");
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

    return to_surround_mode(read("DLNA", "SurroundMode", to_string(default_surround_mode)));
}

void settings::set_surround_mode(enum surround_mode surround_mode)
{
    if (surround_mode != default_surround_mode)
        return write("DLNA", "SurroundMode", to_string(surround_mode));
    else
        return erase("DLNA", "SurroundMode");
}

static const char * to_string(path_type e)
{
    switch (e)
    {
    case path_type::auto_   : return "Auto";
    case path_type::music   : return "Music";
    }

    assert(false);
    return nullptr;
}

static path_type to_path_type(const std::string &e)
{
    if      (e == "Auto")   return path_type::auto_;
    else if (e == "Music")  return path_type::music;

    assert(false);
    return path_type::auto_;
}

std::vector<root_path> settings::root_paths() const
{
    std::vector<std::string> entries;
    bool open = false;
    for (char c : read("Media%20Player", "RootPaths", std::string()))
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

void settings::set_root_paths(const std::vector<root_path> &paths)
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
    if (!string.empty())
        return write("Media%20Player", "RootPaths", string.substr(2));
    else
        return erase("Media%20Player", "RootPaths");
}


#if defined(__unix__)
#include <unistd.h>

#ifndef TEST_H
#include <sys/types.h>
#include <pwd.h>

static std::string filename()
{
    static const char conffile[] = "/.config/LeX-Interactive/LXiMediaCenter.conf";

    const char *home = getenv("HOME");
    if (home)
        return std::string(home) + conffile;

    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir)
        return std::string(pw->pw_dir) + conffile;

    return std::string();
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

#ifndef TEST_H
#include <cstdlib>

static std::string filename()
{
    static const char conffile[] = "\\LeX-Interactive\\LXiMediaCenter.conf";

    const char * const appdata = getenv("APPDATA");
    if (appdata)
        return std::string(appdata) + conffile;

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
        uuid.Data3 = rand();
        for (auto &i : uuid.Data4) i = rand();
    }

    RPC_CSTR rpc_string;
    if (UuidToStringA(&uuid, &rpc_string) != RPC_S_OK)
        throw std::runtime_error("out of memory");

    const std::string result = reinterpret_cast<const char *>(rpc_string);
    RpcStringFree(&rpc_string);

    return result;
}

static std::string default_devicename()
{
    std::string default_devicename = "LXiMediaCenter";

    const char *hostname = getenv("COMPUTERNAME");
    if (hostname)
        default_devicename = std::string(hostname) + " : " + default_devicename;

    return default_devicename;
}
#endif
