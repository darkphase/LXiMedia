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

#ifndef TVSHOWSERVER_H
#define TVSHOWSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include "mediadatabase.h"
#include "mediaplayerserver.h"

namespace LXiMediaCenter {

class TvShowServer : public MediaPlayerServer
{
Q_OBJECT
protected:
  class Dir : public MediaPlayerServerDir
  {
  public:
    explicit                    Dir(MediaPlayerServer *, const QString &albumPath);

    virtual QStringList         listDirs(void);
    virtual QStringList         listFiles(void);

    inline TvShowServer       * server(void)                                    { return static_cast<TvShowServer *>(MediaPlayerServerDir::server()); }
    inline const TvShowServer * server(void) const                              { return static_cast<const TvShowServer *>(MediaPlayerServerDir::server()); }

  private:
    void                        categorizeSeasons(void);

  protected:
    virtual MediaPlayerServerDir * createDir(MediaPlayerServer *, const QString &albumPath);

  private:
    QMap<unsigned, QMap<QString, File> > seasons;
  };

  class SeasonDir : public MediaServerDir
  {
  public:
    explicit                    SeasonDir(TvShowServer *, const QMap<QString, File> &episodes);

    virtual QStringList         listFiles(void);

    inline TvShowServer       * server(void)                                    { return static_cast<TvShowServer *>(MediaServerDir::server()); }
    inline const TvShowServer * server(void) const                              { return static_cast<const TvShowServer *>(MediaServerDir::server()); }

  private:
    const QMap<QString, File>   episodes;
  };

public:
                                TvShowServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, BackendServer::MasterServer *);
  virtual                       ~TvShowServer();

private:
  static QString                toTvShowNumber(unsigned);
};

} // End of namespace

#endif
