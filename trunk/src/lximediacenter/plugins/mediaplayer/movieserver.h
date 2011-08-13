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

#ifndef MOVIESERVER_H
#define MOVIESERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include "mediadatabase.h"
#include "mediaplayerserver.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MovieServer : public MediaPlayerServer
{
Q_OBJECT
public:
                                MovieServer(MediaDatabase::Category, QObject *);
  virtual                       ~MovieServer();

protected:
  virtual QList<Item>           listItems(const QString &path, unsigned start, unsigned count);

private:
  Item                          recurseItem(const QString &path, const Item &item);
};

} } // End of namespaces

#endif
