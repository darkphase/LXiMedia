/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "movieserver.h"
#include "mediaplayersandbox.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

MovieServer::MovieServer(MediaDatabase::Category category, QObject *parent)
  : MediaPlayerServer(category, parent)
{
}

MovieServer::~MovieServer()
{
}

QList<MovieServer::Item> MovieServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> items = MediaPlayerServer::listItems(path, start, count);

  for (QList<Item>::Iterator i=items.begin(); i!=items.end(); i++)
  if (i->isDir)
    *i = recurseItem(path, *i);

  return items;
}

/*! This removes directories with only one item and puts the item in the list..
 */
MovieServer::Item MovieServer::recurseItem(const QString &path, const Item &item)
{
  const QString subPath = path + item.title + '/';

  const QList<Item> items = MediaPlayerServer::listItems(subPath, 0, 2);
  if (items.count() == 1)
  {
    if (items.first().isDir)
    {
      const Item subItem = recurseItem(subPath, items.first());
      if (!subItem.isDir)
        return subItem;
    }
    else
      return items.first();
  }

  return item;
}

} } // End of namespaces
