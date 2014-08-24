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
#include <cassert>
#include <fstream>
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

static encode_mode from_string(const std::string &e)
{
    if (e == "Fast")
        return encode_mode::fast;
    else if (e == "Slow")
        return encode_mode::slow;

    assert(false);
    return encode_mode::slow;
}

enum encode_mode settings::encode_mode() const
{
    const enum encode_mode default_encode_mode =
            (std::thread::hardware_concurrency() > 3) ? ::encode_mode::slow : ::encode_mode::fast;

    return from_string(read("DLNA", "EncodeMode", to_string(default_encode_mode)));
}

enum canvas_mode settings::canvas_mode() const
{
    // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
    if (vlc::instance::compare_version(2, 1) == 0)
        return ::canvas_mode::none;

    return ::canvas_mode::pad;
}

enum surround_mode settings::surround_mode() const
{
    // Workaround for ticket https://trac.videolan.org/vlc/ticket/1897
    if (vlc::instance::compare_version(2, 2) < 0)
        return ::surround_mode::stereo;

    return ::surround_mode::surround51;
}

enum video_mode settings::video_mode() const
{
    return ::video_mode::hdtv_720;
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
            {
                if (type == "Auto")
                    result.emplace_back(root_path { path_type::auto_, path.substr(7) });
                else if (type == "Music")
                    result.emplace_back(root_path { path_type::music, path.substr(7) });
            }
        }
    }

    return result;
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
