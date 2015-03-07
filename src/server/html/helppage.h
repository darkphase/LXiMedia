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

#ifndef HTML_HELPPAGE_H
#define HTML_HELPPAGE_H

#include "mainpage.h"
#include <ostream>

class settings;

namespace html {

class helppage
{
public:
    explicit helppage(class mainpage &);
    ~helppage();

private:
    void render_headers(const struct pupnp::upnp::request &, std::ostream &);
    int render_page(const struct pupnp::upnp::request &, std::ostream &);

private:
    class mainpage &mainpage;
};

} // End of namespace

#endif
