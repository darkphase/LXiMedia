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

private:
  class Task : public QRunnable
  {
  public:
    inline                      Task(MediaDatabase *parent, void(MediaDatabase::* func)(void)) : parent(parent), func(func) { }

    virtual void                run(void);

  private:
    MediaDatabase       * const parent;
    void                        (MediaDatabase::* const func)(void);
  };

  struct CategoryFunc
  {
    const char          * const name;
    void                        (MediaDatabase::* const invalidate)(void);
    void                        (MediaDatabase::* const categorize)(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  };

  struct Category
  {
    inline                      Category(const CategoryFunc *func, const QString &path) : func(func), path(path) { }

    const CategoryFunc        * func;
    QString                     path;
  };

  struct QuerySet;

public:
                                MediaDatabase(Plugin *parent, QThreadPool *);
  virtual                       ~MediaDatabase();

  static inline QByteArray      toUidString(UniqueID uid)                       { return QByteArray::number(quint64(uid) | Q_UINT64_C(0x8000000000000000), 16); }
  static inline UniqueID        fromUidString(const QByteArray &str)            { return UniqueID(str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF)); }
  static inline UniqueID        fromUidString(const QString &str)               { return UniqueID(str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF)); }

  UniqueID                      fromPath(const QString &path) const;
  SMediaInfo                    readNode(UniqueID) const;

  void                          setLastPlayed(UniqueID, const QDateTime & = QDateTime::currentDateTime());
  void                          setLastPlayed(const SMediaInfo &, const QDateTime & = QDateTime::currentDateTime());
  QDateTime                     lastPlayed(UniqueID) const;
  QDateTime                     lastPlayed(const SMediaInfo &) const;

  QStringList                   allMovies(void) const;
  QList<UniqueID>               allMovieFiles(const QString &key) const;
  ImdbClient::Entry             getMovieImdbEntry(const QString &key) const;
  QList<UniqueID>               queryMovieFiles(const QStringList &query) const;

  QStringList                   allTvShows(void) const;
  QList<UniqueID>               allTvShowEpisodes(const QString &key) const;
  ImdbClient::Entry             getTvShowImdbEntry(const QString &key) const;
  QList<UniqueID>               queryTvShows(const QStringList &query) const;

  QStringList                   allHomeVideos(void) const;
  QList<UniqueID>               allHomeVideoFiles(const QString &key) const;
  QList<UniqueID>               queryHomeVideos(const QStringList &query) const;

  QStringList                   allVideoClipAlbums(void) const;
  QList<UniqueID>               allVideoClipFiles(const QString &key) const;
  QList<UniqueID>               queryVideoClips(const QStringList &query) const;

  QStringList                   allPhotoAlbums(void) const;
  QList<UniqueID>               allPhotoFiles(const QString &key) const;
  QList<UniqueID>               queryPhotoAlbums(const QStringList &query) const;

  unsigned                      numSongs(void) const;
  UniqueID                      getSong(unsigned) const;
  QList<UniqueID>               latestSongs(unsigned count) const;
  QStringList                   allMusicArtists(void) const;
  QList<UniqueID>               allMusicArtistFiles(const QString &key) const;
  QStringList                   allMusicGenres(void) const;
  QList<UniqueID>               allMusicGenreFiles(const QString &key) const;
  QStringList                   allMusicVideoAlbums(void) const;
  QList<UniqueID>               allMusicVideoFiles(const QString &key) const;
  QList<UniqueID>               queryMusic(const QStringList &query) const;
  static QString                genreName(const QString &);

  QStringList                   allMiscAudioAlbums(void) const;
  QList<UniqueID>               allMiscAudioFiles(const QString &key) const;
  QList<UniqueID>               queryMiscAudioAlbums(const QStringList &query) const;

  QList<UniqueID>               allFilesInDirOf(UniqueID) const;

signals:
  void                          updatedClips(void);
  void                          updatedHomeVideos(void);
  void                          updatedMovies(void);
  void                          updatedMusic(void);
  void                          updatedPhotos(void);
  void                          updatedTvShows(void);

private slots:
  void                          scanRoots(void);

private:
  QString                       findRoot(const QString &, const QStringList &) const;
  void                          scanDirs(void);
  void                          updateDir(const QString &, qint64, QuerySet &);
  void                          probeFiles(void);
  void                          matchImdbItems(void);

  QList<Category>               findCategories(const QString &) const;

  void                          categorizeMovie(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  void                          categorizeTVShow(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  void                          categorizeClip(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  void                          categorizeMusic(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  void                          categorizePhoto(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  void                          categorizeHomeVideo(QSqlDatabase &, qint64, const QString &, const SMediaInfo &);
  static QString                albumPath(const QString &);

  void                          invalidateMovie(void);
  void                          invalidateTVShow(void);
  void                          invalidateClip(void);
  void                          invalidateMusic(void);
  void                          invalidatePhoto(void);
  void                          invalidateHomeVideo(void);

public:
  static const int              maxSongDurationMin;

private:
  static const CategoryFunc     categoryFunc[];

  Plugin                * const plugin;
  QThreadPool           * const threadPool;
  mutable QMutex                mutex;

  QAtomicInt                    invalidatedClips;
  QAtomicInt                    invalidatedHomeVideos;
  QAtomicInt                    invalidatedMovies;
  QAtomicInt                    invalidatedMusic;
  QAtomicInt                    invalidatedPhotos;
  QAtomicInt                    invalidatedTvShows;
  QTimer                        scanRootsTimer;
  QMap<QString, QStringList>    rootPaths;
  QSet<QString>                 dirsToScan;
  QSet<QString>                 filesToProbe;
  QSet<QString>                 imdbItemsToMatch;
  bool                          matchingImdbItems;
};


} // End of namespace

#endif
