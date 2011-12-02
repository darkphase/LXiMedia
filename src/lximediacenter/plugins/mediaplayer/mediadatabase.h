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
  static MediaDatabase        * createInstance(BackendServer::MasterServer *);
  static void                   destroyInstance(void);

private:
  explicit                      MediaDatabase(BackendServer::MasterServer *, QObject *parent = NULL);
  virtual                       ~MediaDatabase();

public:
  FileNode                      readNode(const QString &filePath) const;
  QByteArray                    readImage(const QString &filePath, const QSize &maxSize, const QString &format) const;

  void                          setLastPlayed(const QString &filePath, const QDateTime & = QDateTime::currentDateTime());
  QDateTime                     lastPlayed(const QString &filePath) const;

  bool                          hasAlbum(const QString &path) const;
  int                           countAlbums(const QString &path) const;
  QStringList                   getAlbums(const QString &path, unsigned start = 0, unsigned count = 0) const;
  int                           countAlbumFiles(const QString &path) const;
  FileNodeList                  getAlbumFiles(const QString &path, unsigned start = 0, unsigned count = 0) const;

signals:
  void                          modified(void);

private:
  static const char     * const categoryNames[7];
  static MediaDatabase        * self;

  SSandboxClient        * const probeSandbox;

  static const int              cacheSize;
  mutable QMap<QString, QByteArray> nodeCache;
  mutable QQueue<QString>       cacheQueue;
};


} } // End of namespaces

#endif
