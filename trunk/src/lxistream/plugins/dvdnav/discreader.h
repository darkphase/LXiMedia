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

#ifndef __DISCREADER_H
#define __DISCREADER_H

#include <QtCore>
#include <LXiStream>
#include <dvdnav/dvdnav.h>

namespace LXiStream {
namespace DVDNavBackend {


class DiscReader : public SInterfaces::DiscReader
{
Q_OBJECT
public:
  static const char     * const formatName;

  static QString                discPath(const QString &path);
  static bool                   isDiscPath(const QString &path);

  explicit                      DiscReader(const QString &, QObject *);
  virtual                       ~DiscReader();

  QString                       title(void) const;

public: // From SInterfaces::DiscReader
  virtual bool                  openPath(const QString &format, const QString &path);

  virtual unsigned              numTitles(void) const;
  virtual bool                  playTitle(unsigned);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual void                  annotateChapters(QList<Chapter> &) const;

  virtual void                  annotateAudioStreams(QList<AudioStreamInfo> &) const;
  virtual void                  annotateVideoStreams(QList<VideoStreamInfo> &) const;
  virtual void                  annotateDataStreams(QList<DataStreamInfo> &) const;

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

  static const unsigned         blockSize = 2048;
  bool                          playing, skipStill, skipWait;
};


} } // End of namespaces

#endif
