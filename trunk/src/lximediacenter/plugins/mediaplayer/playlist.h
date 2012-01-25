/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtCore>
#include <LXiStream>
#include "mediadatabase.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class Playlist : public QObject
{
Q_OBJECT
private:
  struct Entry
  {
    inline explicit Entry(const QString &filePath = QString::null, bool played = false)
      : filePath(filePath), played(played)
    {
    }
    
    QString                     filePath;
    bool                        played;
  };

public:
                                Playlist(MediaDatabase *, QObject * = NULL);
  virtual                       ~Playlist();

  void                          append(const QString &filePath);
  void                          remove(const QString &filePath);
  void                          clear(void);
  int                           count(void) const;
  QString                       checkout(void);
  QStringList                   next(void);

  QByteArray                    serialize(void) const;
  bool                          deserialize(const QByteArray &);

private:
  QString                       random(void);

private:
  MediaDatabase         * const mediaDatabase;

  QVector<Entry>                list;
  QStringList                   pending;
  int                           played;
};

} } // End of namespaces

#endif
