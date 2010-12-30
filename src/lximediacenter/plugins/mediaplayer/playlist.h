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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtCore>
#include <LXiStream>
#include "mediadatabase.h"

namespace LXiMediaCenter {

class Playlist : public QObject
{
Q_OBJECT
private:
  struct Entry
  {
    inline Entry(MediaDatabase::UniqueID uid = 0, bool played = false)
      : uid(uid), played(played)
    {
    }
    
    MediaDatabase::UniqueID     uid;
    bool                        played;
  };

public:
                                Playlist(MediaDatabase *, QObject * = NULL);
  virtual                       ~Playlist();

  void                          append(MediaDatabase::UniqueID);
  void                          remove(MediaDatabase::UniqueID);
  void                          clear(void);
  int                           count(void) const;
  MediaDatabase::UniqueID       checkout(void);
  QList<MediaDatabase::UniqueID> next(void);

  QByteArray                    serialize(void) const;
  bool                          deserialize(const QByteArray &);

  static Playlist             * createAllFiles(MediaDatabase *, QObject * = NULL);

private:
  MediaDatabase::UniqueID       random(void);

private:
  MediaDatabase         * const mediaDatabase;

  mutable QMutex                mutex;
  QVector<Entry>                list;
  QList<MediaDatabase::UniqueID> pending;
  bool                          allFiles;
  int                           played;
};

} // End of namespace

#endif
