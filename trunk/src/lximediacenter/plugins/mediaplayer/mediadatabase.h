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
  struct UniqueID
  {
    inline                      UniqueID(qint64 fid = 0, quint16 pid = 0) : fid(fid), pid(pid) { }

    inline bool                 operator==(const UniqueID &other) const         { return (fid == other.fid) && (pid == other.pid); }
    inline bool                 operator!=(const UniqueID &other) const         { return !operator==(other); }

    qint64                      fid;  //!< Uniquely identifies a file.
    quint16                     pid;  //!< Uniquely identifies a program in a file.
  };

  struct File
  {
    inline                      File(UniqueID uid, const QString &name) : uid(uid), name(name) { }

    UniqueID                    uid;
    QString                     name;
  };

  enum Category
  {
    Category_None             =  0,
    Category_Movies           = 10,
    Category_TVShows          = 20,
    Category_Clips            = 30,
    Category_HomeVideos       = 40,
    Category_Photos           = 50,
    Category_Music            = 60
  };

private:
  struct CatecoryDesc
  {
    const char          * const name;
    const Category              category;
  };

  struct QuerySet;
  class ScanDirEvent;
  class QueryImdbItemEvent;

public:
  static MediaDatabase        * createInstance(BackendServer::MasterServer *);
  static void                   destroyInstance(void);

private:
                                MediaDatabase(BackendServer::MasterServer *, QObject *parent = NULL);
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

  QStringList                   allAlbums(Category) const;
  bool                          hasAlbum(Category, const QString &album) const;
  int                           countAlbumFiles(Category, const QString &album) const;
  QList<File>                   getAlbumFiles(Category, const QString &album, unsigned start = 0, unsigned count = 0) const;
  QList<File>                   queryAlbums(Category, const QStringList &query, unsigned start = 0, unsigned count = 0) const;

  ImdbClient::Entry             getImdbEntry(UniqueID) const;
  QList<UniqueID>               allFilesInDirOf(UniqueID) const;

  void                          rescanRoots(void);

signals:
  void                          modified(void);

protected:
  virtual void                  customEvent(QEvent *);

private slots:
  void                          scanRoots(void);
  void                          directoryChanged(const QString &);
  void                          probeFinished(const SHttpEngine::ResponseMessage &);

private:
  QByteArray                    readNodeData(UniqueID) const;
  QString                       findRoot(const QString &, const QStringList &) const;

  void                          updateDir(const QString &, qint64, QuerySet &);
  void                          probeFile(const QString &);

  bool                          isHidden(const QString &);
  QMap<Category, QString>       findCategories(const QString &) const;

public:
  static const int              maxSongDurationMin;
  static const int              scanDirPriority = INT_MIN;
  static const int              matchImdbItemPriority = scanDirPriority + 1;

private:
  static const QEvent::Type     scanDirEventType;
  static const QEvent::Type     queryImdbItemEventType;
  static const QEvent::Type     scanRootsEventType;

  static const CatecoryDesc     categories[];
  static MediaDatabase        * self;

  ImdbClient            * const imdbClient;
  SSandboxClient        * const probeSandbox;

  QFileSystemWatcher            fileSystemWatcher;
  QMap<QString, QStringList>    rootPaths;
  QTimer                        scanRootTimer;
  QTimer                        scanRootSingleTimer;
  QAtomicInt                    scanning;
};


} } // End of namespaces

#endif
