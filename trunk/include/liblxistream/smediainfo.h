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
#include "sinterfaces.h"

namespace LXiStream {

class SMediaInfo;
typedef QList<SMediaInfo> SMediaInfoList;

class SMediaInfo : public SSerializable
{
friend class SDiscInfo;
public:
  typedef SInterfaces::FormatProber::AudioStreamInfo  AudioStreamInfo;
  typedef SInterfaces::FormatProber::VideoStreamInfo  VideoStreamInfo;
  typedef SInterfaces::FormatProber::DataStreamInfo   DataStreamInfo;
  typedef SInterfaces::FormatProber::Chapter          Chapter;

public:
                                SMediaInfo(void);
                                SMediaInfo(const SMediaInfo &);
  explicit                      SMediaInfo(const QString &path);
  explicit                      SMediaInfo(const SInterfaces::FormatProber::ProbeInfo &);
  virtual                       ~SMediaInfo();

  SMediaInfo                  & operator=(const SMediaInfo &);

  virtual QDomNode              toXml(QDomDocument &) const;
  void                          fromXml(const QDomNode &node, const QString &path);
  void                          fromByteArray(const QByteArray &data, const QString &path);

  QString                       format(void) const;                             //!< The format name of the media (e.g. matroska, mpeg, dvd, etc.)
  bool                          isDisc(void) const;                             //!< True if the resource is a disc (e.g. DVD).

  bool                          containsAudio(void) const;                      //!< True if the resource contains audio data.
  bool                          containsVideo(void) const;                      //!< True if the resource contains video data.
  bool                          containsImage(void) const;                      //!< True if the resource contains an image.
  bool                          isProbed(void) const;                           //!< True if the resource was deep-probed (i.e. the resource was opened and scanned).
  bool                          isReadable(void) const;                         //!< True if the resource is readable (is set to true if deep-probe can read it).

  QString                       fileTypeName(void) const;                       //!< A user-friendly description of the file type.

  STime                         duration(void) const;                           //!< The duration of the resource, if applicable.
  QList<AudioStreamInfo>        audioStreams(void) const;                      //!< The audio streams, if applicable.
  QList<VideoStreamInfo>        videoStreams(void) const;                      //!< The video streams, if applicable.
  QList<DataStreamInfo>         dataStreams(void) const;                        //!< The data streams, if applicable.
  SVideoCodec                   imageCodec(void) const;                         //!< The image codec, if applicable.
  QList<Chapter>                chapters(void) const;                           //!< A list of chapters, if applicable.

  SMediaInfoList                titles(void) const;                             //!< A list of titles, if applicable.

  QString                       title(void) const;                              //!< The title (e.g. from ID3).
  QString                       author(void) const;                             //!< The author (e.g. from ID3).
  QString                       copyright(void) const;                          //!< The copyright (e.g. from ID3).
  QString                       comment(void) const;                            //!< The comment (e.g. from ID3).
  QString                       album(void) const;                              //!< The album (e.g. from ID3).
  QString                       genre(void) const;                              //!< The genre (e.g. from ID3).
  unsigned                      year(void) const;                               //!< The year (e.g. from ID3).
  unsigned                      track(void) const;                              //!< The track (e.g. from ID3).

  QList<QByteArray>             thumbnails(void) const;                         //!< Thumbnail(s) of the resource, if applicable. In case of an image it is of at most 256x256.

public:
  static const unsigned         tvShowSeason;

private:
  virtual void                  fromXml(const QDomNode &);
  static QDomNode               toXml(const SInterfaces::FormatProber::ProbeInfo &pi, QDomDocument &);
  static void                   fromXml(SInterfaces::FormatProber::ProbeInfo &pi, const QDomNode &);

  void                          probe(void);
  void                          probeDataStreams(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
