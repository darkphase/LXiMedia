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

#ifndef RECOMMENDED_H
#define RECOMMENDED_H

#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"

class watchlist;

class recommended : private pupnp::content_directory::item_source
{
public:
    struct item_source
    {
        virtual std::vector<pupnp::content_directory::item> list_recommended_items(const std::string &client, size_t start, size_t &count) = 0;
        virtual pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) = 0;
        virtual bool correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) = 0;
        virtual int play_item(const std::string &source_address, const pupnp::content_directory::item &, const std::string &profile, std::string &, std::shared_ptr<std::istream> &) = 0;
    };

public:
    recommended(
            class pupnp::content_directory &);

    virtual ~recommended();

    void item_source_register(const std::string &path, struct item_source &);
    void item_source_unregister(const std::string &path);

private: // From content_directory::item_source
    std::vector<pupnp::content_directory::item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
    pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) override;
    bool correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) override;
    int play_item(const std::string &, const pupnp::content_directory::item &, const std::string &, std::string &, std::shared_ptr<std::istream> &) override;

private:
    class pupnp::content_directory &content_directory;
    const std::string basedir;

    std::map<std::string, item_source *> item_sources;
};

#endif
