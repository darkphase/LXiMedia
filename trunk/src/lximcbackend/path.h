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

#ifndef PATH_H
#define PATH_H

#include <string>
#include <vector>

std::vector<std::string> list_root_directories();

std::vector<std::string> list_files(
        const std::string &path,
        bool directories_only = false,
        const size_t min_file_size = 0);

#ifdef WIN32
std::wstring to_windows_path(const std::string &);
std::string from_windows_path(const std::wstring &);
#endif

#endif