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

#ifndef MEDIADATABASE_H
#define MEDIADATABASE_H

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
  QImage readImage(const QUrl &filePath, const QSize &maxSize, const QColor &backgroundColor) const;

  void                          setLastPlayed(const QUrl &filePath, const QDateTime & = QDateTime::currentDateTime());
  QPair<QDateTime, int>         lastPlayed(const QUrl &filePath) const;

  bool                          isEmpty(const QUrl &dirPath) const;
  SMediaInfoList                listItems(const QUrl &dirPath, int start, int &count) const;
  SMediaInfoList                representativeItems(const QUrl &dirPath, int &count) const;

protected:
  virtual void                  run();

private slots:
  void                          flushCache(void) const;

private:
  QList<Info>                   listFiles(const QUrl &dirPath, int start, int &count) const;
  SMediaInfoList                readNodeFormat(const QList<Info> &filePaths) const;
  SMediaInfo                    readNodeCache(const Info &file) const;
  SMediaInfo                    readNodeCache(const QUrl &filePath) const;
  void                          writeNodeCache(const SMediaInfo &) const;

  void                          preProbeDir(const QUrl &dirPath, int start, bool content) const;

  static SMediaInfoList         deserializeNodes(const QByteArray &);
  static QList<Info>            deserializeFiles(const QByteArray &, int &total);

  static QString                lastPlayedFile(void);

private:
  static MediaDatabase        * self;

  const QString                 lastPlayedFileName;

  mutable QMutex                mutex;

  volatile bool                 preProbeRunning;
  const int                     preProbeItemCount;
  mutable QMultiMap<int, PreProbe> preProbeQueue;
  mutable QWaitCondition        preProbeWaiting;
  const int                     preProbingCount;
  mutable int                   preProbing;

  static const int              cacheTimeout = 150 * 60000; // Flush cache after 2.5 hrs.
  mutable QFile                 cacheFile;
  mutable QMultiMap<uint, qint64> cachePos;
};


} } // End of namespaces

#endif
