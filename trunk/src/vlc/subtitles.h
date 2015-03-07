/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#ifndef VLC_SUBTITLES_H
#define VLC_SUBTITLES_H

#include <string>
#include <vector>

namespace vlc {

namespace subtitles {

class file
{
public:
    file();
    file(const std::string &path, const std::string &encoding);
    file(file &&);
    file(const file &) = delete;
    ~file();

    file & operator=(file &&);
    file & operator=(const file &) = delete;

    operator bool() const { return !path.empty(); }
    operator const std::string &() const { return path; }

private:
    std::string path;
    bool own;
};

std::vector<std::string> find_subtitle_files(const std::string &path);

bool determine_subtitle_language(
        const std::string &path,
        const char *&language,
        const char *&encoding);

} // End of namespace

} // End of namespace

#endif
