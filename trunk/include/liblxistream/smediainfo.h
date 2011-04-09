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

#ifndef LXSTREAM_SMEDIAINFO_H
#define LXSTREAM_SMEDIAINFO_H

#include <QtCore>
#include <QtXml>
#include <LXiCore>
#include "sinterfaces.h"
#include "export.h"

namespace LXiStream {

class SMediaInfo;
typedef QList<SMediaInfo> SMediaInfoList;

class LXISTREAM_PUBLIC SMediaInfo : public SSerializable
{
friend class SDiscInfo;
public:
  typedef SInterfaces::FormatProber::ProbeInfo::Program Program;
  typedef SInterfaces::FormatProber::Chapter            Chapter;
  typedef SInterfaces::FormatProber::AudioStreamInfo    AudioStreamInfo;
  typedef SInterfaces::FormatProber::VideoStreamInfo    VideoStreamInfo;
  typedef SInterfaces::FormatProber::DataStreamInfo     DataStreamInfo;

public:
                                SMediaInfo(void);
                                SMediaInfo(const SMediaInfo &);
  explicit                      SMediaInfo(const QString &path);
  explicit                      SMediaInfo(const QSharedDataPointer<SInterfaces::FormatProber::ProbeInfo> &);
  virtual                       ~SMediaInfo();

  SMediaInfo                  & operator=(const SMediaInfo &);

  virtual QDomNode              toXml(QDomDocument &) const;
  virtual void                  fromXml(const QDomNode &);

  bool                          isNull(void) const;                             //!< Returns true if the SMediaInfo is empty.

  QString                       filePath(void) const;                           //!< Returns the file path, including the file name.
  QString                       fileName(void) const;                           //!< Returns the name of the file, without the path.
  QString                       baseName(void) const;                           //!< Returns the name of the file, without the path and extension.
  QString                       path(void) const;                               //!< Returns the path to the file, without the file name.
  qint64                        size(void) const;                               //!< Returns the file size.
  QDateTime                     lastModified(void) const;                       //!< Returns the date the file was last modified.

  QString                       format(void) const;                             //!< The format name of the media (e.g. matroska, mpeg, dvd, etc.)

  STime                         totalDuration(void) const;                      //!< Returns the total duration of all programs, if available.
  bool                          containsAudio(void) const;                      //!< True if the resource contains audio data.
  bool                          containsVideo(void) const;                      //!< True if the resource contains video data.
  bool                          containsImage(void) const;                      //!< True if the resource contains an image.
  bool                          isProbed(void) const;                           //!< True if the resource was deep-probed (i.e. the resource was opened and scanned).
  bool                          isReadable(void) const;                         //!< True if the resource is readable (is set to true if deep-probe can read it).

  QString                       fileTypeName(void) const;                       //!< A user-friendly description of the file type.

  const QList<Program>        & programs(void) const;                           //!< The programs available in the resource, if applicable.

  QString                       title(void) const;                              //!< The title (e.g. from ID3).
  QString                       author(void) const;                             //!< The author (e.g. from ID3).
  QString                       copyright(void) const;                          //!< The copyright (e.g. from ID3).
  QString                       comment(void) const;                            //!< The comment (e.g. from ID3).
  QString                       album(void) const;                              //!< The album (e.g. from ID3).
  QString                       genre(void) const;                              //!< The genre (e.g. from ID3).
  unsigned                      year(void) const;                               //!< The year (e.g. from ID3).
  unsigned                      track(void) const;                              //!< The track (e.g. from ID3).

public:
  static const unsigned         tvShowSeason;

private:
  __internal static QDomNode    toXml(const SInterfaces::FormatProber::ProbeInfo &pi, QDomDocument &);
  __internal static void        fromXml(SInterfaces::FormatProber::ProbeInfo &pi, const QDomNode &);

  __internal void               probe(const QString &);
  __internal void               probeDataStreams(void);

private:
  QSharedDataPointer<SInterfaces::FormatProber::ProbeInfo> pi;
};

} // End of namespace

#endif
