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

#ifndef VLC_MEDIA_H
#define VLC_MEDIA_H

#include <string>
#include <vector>

struct libvlc_media_t;

namespace vlc {

class instance;

class media
{
public:
  enum class track_type { unknown, audio, video, text };

  struct track
  {
    std::string language;
    std::string description;

    track_type type;
    union
    {
      struct { unsigned sample_rate, channels; } audio;
      struct { unsigned width, height; float frame_rate; } video;
    };
  };

public:
  media(class instance &, const std::string &path);
  media(const media &);
  ~media();

  media & operator=(const media &);
  inline operator libvlc_media_t *() { return libvlc_media; }

  void parse() const;

  std::string mrl() const;
  std::vector<track> tracks() const;

private:
  libvlc_media_t *libvlc_media;
};

} // End of namespace

#endif
