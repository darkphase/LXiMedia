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
#include "mediaserver.h"

namespace LXiMediaCenter {

class MovieServer : public MediaServer
{
Q_OBJECT
public:
                                MovieServer(MediaDatabase *, Plugin *, MasterServer *);
  virtual                       ~MovieServer();

  virtual QByteArray            frontPageWidget(void) const;
  virtual SearchResultList      search(const QStringList &) const;

  virtual void                  updateDlnaTask(void);

private:
  static const char     * const htmlMovieWidget;
  static const char     * const htmlMovieWidgetItem;
  static const char     * const htmlRatingStar;
};

} // End of namespace

#endif
