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

namespace LXiMediaCenter {

class MediaDatabase : public QObject
{
Q_OBJECT
public:
  typedef qint64                UniqueID;

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

public:
                                MediaDatabase(Plugin *parent, ImdbClient *);
  virtual                       ~MediaDatabase();

  static inline QByteArray      toUidString(UniqueID uid)                       { return QByteArray::number(quint64(uid) | Q_UINT64_C(0x8000000000000000), 16); }
  static inline UniqueID        fromUidString(const QByteArray &str)            { return UniqueID(str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF)); }
  static inline UniqueID        fromUidString(const QString &str)               { return UniqueID(str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF)); }

  UniqueID                      fromPath(const QString &path) const;
  SMediaInfo                    readNode(UniqueID) const;

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

signals:
  void                          modified(void);

private slots:
  void                          scanRoots(void);

private:
  QByteArray                    readNodeData(UniqueID) const;
  QString                       findRoot(const QString &, const QStringList &) const;
  void                          scanDir(const QString &);
  void                          updateDir(const QString &, qint64, QuerySet &);
  void                          probeFile(const QString &);
  void                          insertFile(const SMediaInfo &, const QByteArray &);
  void                          delayFile(const QString &);
  void                          queryImdbItem(const QString &, Category);
  void                          matchImdbItem(const QString &, const QString &, const QStringList &, Category);
  void                          storeImdbItem(const QString &, const QString &, Category);

  bool                          isHidden(const QString &);
  QMap<Category, QString>       findCategories(const QString &) const;

public:
  static const int              maxSongDurationMin;
  static const SScheduler::Priority basePriority = SScheduler::Priority_Idle;
  static const SScheduler::Priority scanDirPriority = SScheduler::Priority(basePriority + 1);
  static const SScheduler::Priority probeFilePriority = SScheduler::Priority(basePriority + 2);
  static const SScheduler::Priority insertFilePriority = SScheduler::Priority(probeFilePriority + 1);
  static const SScheduler::Priority matchImdbItemPriority = basePriority;
  static const SScheduler::Priority storeImdbItemPriority = SScheduler::Priority(matchImdbItemPriority + 1);

private:
  static const CatecoryDesc     categories[];

  Plugin                * const plugin;
  ImdbClient            * const imdbClient;

  QTimer                        scanRootsTimer;
  QMap<QString, QStringList>    rootPaths;
};


} // End of namespace

#endif
