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
private:
  struct Info : SMediaFilesystem::Info
  {
    QUrl                        path;
  };

public:
  static MediaDatabase        * createInstance(BackendServer::MasterServer *);
  static void                   destroyInstance(void);

private:
  explicit                      MediaDatabase(BackendServer::MasterServer *, QObject *parent = NULL);
  virtual                       ~MediaDatabase();

public:
  void                          queueReadNode(const QUrl &filePath) const;
  FileNode                      readNode(const QUrl &filePath) const;
  QByteArray                    readImage(const QUrl &filePath, const QSize &maxSize, const QColor &backgroundColor, const QString &format) const;

  void                          setLastPlayed(const FileNode &, const QDateTime & = QDateTime::currentDateTime());
  QDateTime                     lastPlayed(const FileNode &) const;

  FileNodeList                  listItems(const QUrl &path, int start, int &count) const;
  FileNodeList                  representativeItems(const QUrl &path) const;

signals:
  void                          nodeRead(const FileNode &);
  void                          aborted(void);

private slots:
  void                          handleResponse(const SHttpEngine::ResponseMessage &);
  void                          flushCache(void) const;

private:
  QList<Info>                   listFiles(const QUrl &dirPath, int start, int &count) const;
  FileNodeList                  readNodeFormat(const QList<Info> &filePaths) const;
  FileNode                      readNodeCache(const Info &file) const;
  FileNode                      readNodeCache(const QUrl &filePath) const;
  void                          writeNodeCache(const QUrl &filePath, const QByteArray &) const;

  static QString                lastPlayedFile(void);

private:
  static MediaDatabase        * self;

  const QString                 lastPlayedFileName;
  SSandboxClient        * const probeSandbox;

  static const int              cacheTimeout = 150 * 60000; // Flush cache after 2.5 hrs.
  mutable QTimer                cacheTimer;
  mutable QFile                 cacheFile;
  mutable QMultiMap<uint, qint64> cachePos;
};


} } // End of namespaces

#endif
