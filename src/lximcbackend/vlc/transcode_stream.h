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

#ifndef VLC_TRANSCODE_STREAM_H
#define VLC_TRANSCODE_STREAM_H

#include <istream>
#include <memory>
#include <string>

namespace lximediacenter {
namespace vlc {

class instance;

class transcode_stream : public std::istream
{
public:
  explicit transcode_stream(class instance &);
  ~transcode_stream();
  transcode_stream(const transcode_stream &) = delete;
  transcode_stream & operator=(const transcode_stream &) = delete;

  bool open(const std::string &file);
  void close();

private:
  class instance &instance;
  struct data;
  std::unique_ptr<data> d;
};

} // End of namespace
} // End of namespace

#endif
