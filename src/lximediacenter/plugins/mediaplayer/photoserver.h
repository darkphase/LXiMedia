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

#ifndef PHOTOSERVER_H
#define PHOTOSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include "mediadatabase.h"
#include "mediaplayerserver.h"
#include "slideshownode.h"

namespace LXiMediaCenter {

class PhotoServer : public MediaPlayerServer
{
Q_OBJECT
protected:
  class SlideShowStream : public Stream
  {
  public:
                                SlideShowStream(PhotoServer *, const QHostAddress &peer, const QString &url, const QStringList &fileNames);

    bool                        setup(const QHttpRequestHeader &, QAbstractSocket *, STime, const QString &, const QImage & = QImage());

  public:
    SlideShowNode               slideShow;
  };

  class Dir : public MediaPlayerServerDir
  {
  public:
    explicit                    Dir(MediaPlayerServer *, const QString &albumPath);

    virtual QStringList         listFiles(void);

    inline PhotoServer        * server(void)                                    { return static_cast<PhotoServer *>(MediaPlayerServerDir::server()); }
    inline const PhotoServer  * server(void) const                              { return static_cast<const PhotoServer *>(MediaPlayerServerDir::server()); }

  protected:
    virtual MediaPlayerServerDir * createDir(MediaPlayerServer *, const QString &albumPath);
  };

public:
                                PhotoServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, BackendServer::MasterServer *);
  virtual                       ~PhotoServer();

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *);

private:
  bool                          sendPhoto(QAbstractSocket *, MediaDatabase::UniqueID, unsigned = 0, unsigned = 0) const;
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

private:
  static const char     * const htmlView;
};

} // End of namespace

#endif
