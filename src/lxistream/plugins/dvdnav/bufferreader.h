/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __BUFFERREADER_H
#define __BUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#define this self
#include <dvdnav/dvdnav.h>
#undef this

namespace LXiStream {
namespace DVDNavBackend {

class BufferReader : public SInterfaces::BufferReader,
                     public SInterfaces::BufferReader::ReadCallback
{
Q_OBJECT
public:
  static const char             formatName[];

  static QString                discPath(const QString &path);
  static bool                   isExtractedDiscPath(const QString &path);
  static bool                   isDiscPath(const QString &path);

  explicit                      BufferReader(const QString &, QObject *);
  virtual                       ~BufferReader();

  bool                          openFile(const QString &);
  QString                       discTitle(void) const;
  unsigned                      numTitles(void) const;
  bool                          selectTitle(quint16);
  bool                          reopenBufferReader(void);

  QList<AudioStreamInfo>        filterAudioStreams(const QList<AudioStreamInfo> &) const;
  QList<VideoStreamInfo>        filterVideoStreams(const QList<VideoStreamInfo> &) const;
  QList<DataStreamInfo>         filterDataStreams(const QList<DataStreamInfo> &) const;

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(SInterfaces::BufferReader::ReadCallback *, SInterfaces::BufferReader::ProduceCallback *, quint16 programId, bool streamed);
  virtual void                  stop(void);
  virtual bool                  process(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QVector<StreamId> &);

public: // From SInterfaces::BufferReader::ReadCallback
  virtual qint64                read(uchar *buffer, qint64 size);
  virtual qint64                seek(qint64 offset, int whence);

private:
  mutable QMutex                mutex;
  ::dvdnav_t                  * dvdHandle;

  unsigned                      currentTitle;
  int                           currentChapter;
  QList<STime>                  titleChapters;
  STime                         titleDuration;

  SInterfaces::BufferReader::ProduceCallback * produceCallback;
  SInterfaces::BufferReader   * bufferReader;
  QVector<StreamId>             selectedStreams;

  static const unsigned         blockSize = 2048;
  bool                          seekEnabled, flushing;
  bool                          playing, skipStill, skipWait;
};

} } // End of namespaces

#endif
