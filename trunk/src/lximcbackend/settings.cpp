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
#include <fstream>

#if defined(__unix__)
#include <unistd.h>
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
#endif

namespace lximediacenter {

settings::settings()
  : touched(false)
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
    std::ofstream file(filename() + ".test");
    for (auto &section : values)
    {
      file << '[' << section.first << ']' << std::endl;
      for (auto &value : section.second)
        file << value.first << '=' << value.second << std::endl;
    }

    file << std::endl;
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

std::string settings::write(const std::string &section, const std::string &name, const std::string &value)
{
  values[section][name] = value;
  touched = true;

  save();
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

std::string settings::devicename() const
{
  std::string def = "LXiMediaCenter";
  const char *hostname = getenv("HOSTNAME");
  if (hostname)
    def = std::string(hostname) + " : " + def;

  return read("General", "DeviceName", def);
}

uint16_t settings::http_port() const
{
  static const uint16_t default_port = 4280;

  try { return uint16_t(std::stoi(read("General", "DeviceName", std::to_string(default_port)))); }
  catch (const std::invalid_argument &) { return default_port; }
  catch (const std::out_of_range &) { return default_port; }
}

} // End of namespace
