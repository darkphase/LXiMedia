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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "messageloop.h"
#include <chrono>
#include <cstdint>
#include <string>
#include <map>

class settings
{
public:
  explicit settings(class messageloop &);
  ~settings();

  std::string uuid();
  std::string devicename() const;
  uint16_t http_port() const;

private:
  void save();
  std::string read(const std::string &, const std::string &, const std::string &) const;
  std::string write(const std::string &, const std::string &, const std::string &);

  static std::string filename();
  static std::string make_uuid();

private:
  class messageloop &messageloop;
  class timer timer;
  const std::chrono::milliseconds save_delay;

  std::map<std::string, std::map<std::string, std::string>> values;
  bool touched;
};

#endif
