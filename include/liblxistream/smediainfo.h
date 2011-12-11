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
#include <LXiCore>
#include "sinterfaces.h"
#include "export.h"

namespace LXiStream {

class SMediaInfo;
typedef QList<SMediaInfo> SMediaInfoList;

class LXISTREAM_PUBLIC SMediaInfo
{
public:
  typedef SInterfaces::FormatProber::ProbeInfo          ProbeInfo;
  typedef SInterfaces::FormatProber::Chapter            Chapter;
  typedef SInterfaces::FormatProber::StreamId           StreamId;
  typedef SInterfaces::FormatProber::AudioStreamInfo    AudioStreamInfo;
  typedef SInterfaces::FormatProber::VideoStreamInfo    VideoStreamInfo;
  typedef SInterfaces::FormatProber::DataStreamInfo     DataStreamInfo;

public:
                                SMediaInfo(void);
                                SMediaInfo(const SMediaInfo &);
  explicit                      SMediaInfo(const QString &filePath);
  explicit                      SMediaInfo(const QSharedDataPointer<ProbeInfo> &);
  virtual                       ~SMediaInfo();

  SMediaInfo                  & operator=(const SMediaInfo &);

  bool                          isNull(void) const;                             //!< Returns true if the SMediaInfo is empty.

  QString                       filePath(void) const;                           //!< Returns the file path, including the file name.
  QString                       fileName(void) const;                           //!< Returns the name of the file, without the path.
  QString                       baseName(void) const;                           //!< Returns the name of the file, without the path and extension.
  QString                       path(void) const;                               //!< Returns the path to the file, without the file name.

  bool                          isReadable(void) const;                         //!< Returns true if the file can be read.
  qint64                        size(void) const;                               //!< Returns the file size.
  QDateTime                     lastModified(void) const;                       //!< Returns the date the file was last modified.

  QString                       format(void) const;                             //!< The format name of the media (e.g. matroska, mpeg, dvd, etc.)
  ProbeInfo::FileType           fileType(void) const;                           //!< The detected file type.
  QString                       fileTypeName(void) const;                       //!< A user-friendly description of the file type.
  QByteArray                    quickHash(void) const;                          //!< Returns a hash of a part of the file.
  QVariant                      metadata(const QString &key) const;             //!< Returns the requested metadata value or an empty QVariant if not present.

  const QList<ProbeInfo::Title> & titles(void) const;                           //!< The titles in the file.

protected:
  inline const ProbeInfo      & probeInfo(void) const                           { return *pi; }
  inline ProbeInfo            & probeInfo(void)                                 { return *pi; }

  void                          readFileInfo(void);
  void                          probeFormat(void);
  void                          probeContent(void);
  void                          probeDataStreams(void);

public:
  static const int              tvShowSeason;

private:
  QSharedDataPointer<ProbeInfo> pi;
};

} // End of namespace

#endif
