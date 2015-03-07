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

#include "recommended.h"
#include "platform/translator.h"

recommended::recommended(
        class pupnp::content_directory &content_directory)
    : content_directory(content_directory),
      basedir('/' + tr("Recommended") + '/')
{
    content_directory.item_source_register(basedir, *this);
}

recommended::~recommended()
{
    content_directory.item_source_unregister(basedir);
}

void recommended::item_source_register(const std::string &path, struct item_source &item_source)
{
    item_sources[path] = &item_source;
}

void recommended::item_source_unregister(const std::string &path)
{
    item_sources.erase(path);
}

std::vector<pupnp::content_directory::item> recommended::list_contentdir_items(
        const std::string &client,
        const std::string &,
        size_t start, size_t &count)
{
    for (auto &i : item_sources)
        return i.second->list_recommended_items(client, start, count);

    return std::vector<pupnp::content_directory::item>();
}

pupnp::content_directory::item recommended::get_contentdir_item(const std::string &, const std::string &path)
{
    pupnp::content_directory::item item;
    return item;
}

bool recommended::correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &)
{
    return true;
}

int recommended::play_item(
        const std::string &,
        const pupnp::content_directory::item &item,
        const std::string &,
        std::string &,
        std::shared_ptr<std::istream> &)
{
    return pupnp::upnp::http_not_found;
}
