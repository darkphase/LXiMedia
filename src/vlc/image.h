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

#ifndef VLC_IMAGE_H
#define VLC_IMAGE_H

#include <cstdint>
#include <ostream>
#include <vector>

namespace vlc {

class image
{
public:
    image();
    image(unsigned width, unsigned height);
    image(image &&);
    image(const image &) = delete;
    ~image();

    image & operator=(const image &) = delete;
    image & operator=(image &&);

    const uint32_t * scan_line(unsigned y) const;
    uint32_t * scan_line(unsigned y);

    void swap_rb();

    bool save_jpeg(std::ostream &) const;
    bool save_png(std::ostream &) const;

private:
    static const unsigned align = 32;
    unsigned width, height;
    std::vector<uint32_t> pixels;
};

} // End of namespace

#endif
