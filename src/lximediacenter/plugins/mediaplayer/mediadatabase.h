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

#ifndef MEDIADATABASE_H
#define MEDIADATABASE_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiMediaCenter>
#include <LXiStream>
#include "filenode.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaDatabase : public QObject
{
Q_OBJECT
public:
  typedef qint64 UniqueID;

  enum Category
  {
    Category_None             =  0,
    Category_Movies,
    Category_TVShows,
    Category_Clips,
    Category_HomeVideos,
    Category_Photos,
    Category_Music
    // Keep in sync with categoryNames.
  };

public:
  static MediaDatabase        * createInstance(BackendServer::MasterServer *);
  static void                   destroyInstance(void);

private:
  explicit                      MediaDatabase(BackendServer::MasterServer *, QObject *parent = NULL);
  virtual                       ~MediaDatabase();

public:
  static QByteArray             toUidString(UniqueID uid);
  static UniqueID               fromUidString(const QByteArray &str);
  static UniqueID               fromUidString(const QString &str);

  UniqueID                      fromPath(const QString &path) const;
  FileNode                      readNode(UniqueID) const;

  void                          setLastPlayed(UniqueID, const QDateTime & = QDateTime::currentDateTime());
  void                          setLastPlayed(const QString &filePath, const QDateTime & = QDateTime::currentDateTime());
  QDateTime                     lastPlayed(UniqueID) const;
  QDateTime                     lastPlayed(const QString &filePath) const;

  bool                          hasAlbum(Category, const QString &path) const;
  int                           countAlbums(Category, const QString &path) const;
  QStringList                   getAlbums(Category, const QString &path, unsigned start = 0, unsigned count = 0) const;
  int                           countAlbumFiles(Category, const QString &path) const;
  QVector<UniqueID>             getAlbumFiles(Category, const QString &path, unsigned start = 0, unsigned count = 0) const;

  QStringList                   rootPaths(Category) const;
  QStringList                   rootPaths(const QString &) const;
  void                          setRootPaths(Category, const QStringList &paths);
  void                          setRootPaths(const QString &, const QStringList &paths);

signals:
  void                          modified(void);

private:
  QByteArray                    readNodeData(UniqueID) const;

private:
  static const char     * const categoryNames[7];
  static MediaDatabase        * self;

  ImdbClient            * const imdbClient;
  SSandboxClient        * const probeSandbox;
};


} } // End of namespaces

#endif
