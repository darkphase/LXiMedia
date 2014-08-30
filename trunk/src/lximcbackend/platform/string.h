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

#ifndef STRING_H
#define STRING_H

#include <string>

bool starts_with(const std::string &, const std::string &);
bool ends_with(const std::string &, const std::string &);

std::string to_upper(const std::string &);
std::string to_lower(const std::string &);

std::string from_base64(const std::string &);
std::string to_base64(const std::string &, bool pad = false);

std::string from_percent(const std::string &);
std::string to_percent(const std::string &);
std::string escape_xml(const std::string &);

#if defined(WIN32)
std::wstring to_utf16(const std::string &);
std::string from_utf16(const std::wstring &);
#endif

#endif
