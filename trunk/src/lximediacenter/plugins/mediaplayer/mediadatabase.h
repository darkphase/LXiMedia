/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef MEDIADATABASE_H
#define MEDIADATABASE_H

#define MEDIADATABASE_USE_SANDBOX

#include <QtCore>
#include <QtNetwork>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaDatabase : public QThread
{
Q_OBJECT
private:
  struct Info : SMediaFilesystem::Info
  {
    inline                      Info() { }
    inline                      Info(const Info &info) : SMediaFilesystem::Info(info), path(info.path) { }
    inline                      Info(const SMediaFilesystem::Info &info, const QUrl &path) : SMediaFilesystem::Info(info), path(path) { }

    QUrl                        path;
  };

  struct PreProbe
  {
    inline PreProbe()
      : info(), start(0), content(false)
    {
    }

    inline PreProbe(const Info &info, int start, bool content = false)
      : info(info), start(start), content(content)
    {
    }

    Info                        info;
    int                         start;
    bool                        content;
  };

#ifdef MEDIADATABASE_USE_SANDBOX
  class Sandbox : public QProcess
  {
  public:
    explicit                    Sandbox(bool lowprio, QObject *parent = NULL);
    virtual                     ~Sandbox();

    SMediaInfo                  probeFormat(const QUrl &);
    SMediaInfoList              probeFormat(const QList<QUrl> &);
    SMediaInfo                  probeContent(const QUrl &);
    SMediaInfoList              probeContent(const QList<QUrl> &);
    SImage                      readThumbnail(const QUrl &filePath, const QSize &maxSize);

  private:
    void                        restart();
    SMediaInfo                  probe(const QByteArray &, const QUrl &);
    SMediaInfoList              probe(const QByteArray &, const QList<QUrl> &);
    SMediaInfoList              probe(const QByteArray &);

  private:
    const bool                  lowprio;
  };
#endif

public:
  static MediaDatabase        * createInstance();
  static void                   destroyInstance(void);

private:
  explicit                      MediaDatabase(QObject *parent = NULL);
  virtual                       ~MediaDatabase();

public:
  SMediaInfo                    readNodeFormat(const QUrl &filePath) const;
  SMediaInfo                    readNodeContent(const QUrl &filePath) const;
  SImage                        readThumbnail(const QUrl &filePath, const QSize &maxSize) const;
  QImage                        readImage(const QUrl &filePath, const QSize &maxSize, const QColor &backgroundColor) const;

  void                          setLastPlaybackPosition(const QUrl &filePath, int position);
  int                           getLastPlaybackPosition(const QUrl &filePath) const;

  bool                          isEmpty(const QUrl &dirPath) const;
  SMediaInfoList                listItems(const QUrl &dirPath, int start, int &count) const;
  SMediaInfoList                representativeItems(const QUrl &dirPath, int &count) const;

signals:
  void                          itemChanged(const QUrl &);

protected:
  virtual void                  run();

private slots:
  void                          directoryChanged(const QString &);
  void                          flushCache(void) const;
#ifdef MEDIADATABASE_USE_SANDBOX
  void                          startSandbox(void) const;
  void                          stopSandbox(void);
#endif

private:
  QList<Info>                   listFiles(const QUrl &dirPath, int start, int &count) const;
  SMediaInfoList                readNodeFormat(const QList<Info> &filePaths) const;
  SMediaInfo                    readNodeCache(const Info &file) const;
  SMediaInfo                    readNodeCache(const QUrl &filePath) const;
  void                          writeNodeCache(const SMediaInfo &) const;

  void                          preProbeDir(const QUrl &dirPath, int start, bool content) const;
#ifdef MEDIADATABASE_USE_SANDBOX
  void                          preProbeItems(Sandbox *&, QTime &, QList<QUrl> &, bool);
#else
  void                          preProbeItems(QList<QUrl> &, bool);
#endif

  static QString                lastPlayedFile(void);

private:
  static MediaDatabase        * self;

  const QString                 lastPlayedFileName;

  mutable QMutex                mutex;

  mutable QFileSystemWatcher    fileSystemWatcher;

#ifdef MEDIADATABASE_USE_SANDBOX
  mutable Sandbox             * sandbox;
  mutable QTimer                sandboxTimer;
  static const int              sandboxTimeout = 3000;
#endif

  volatile bool                 preProbeRunning;
  const int                     preProbeItemCount;
  mutable QMultiMap<int, PreProbe> preProbeQueue;
  mutable QWaitCondition        preProbeWaiting;
  const int                     preProbingCount;
  mutable int                   preProbing;

  static const int              cacheTimeout = 150 * 60000; // Flush cache after 2.5 hrs.
  mutable QFile                 cacheFile;
  mutable QMultiMap<uint, qint64> cachePos;

  mutable QMutex                itemCacheMutex;
  mutable QMap<QUrl, QPair<QStringList, QTime> > itemCache;
  static const int              itemCacheTimeout;
};

} } // End of namespaces

#endif
