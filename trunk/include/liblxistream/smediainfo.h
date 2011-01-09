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

class SDiscInfo;

class SMediaInfo : public SSerializable
{
friend class SDiscInfo;
public:
  typedef SInterfaces::FileFormatProber::AudioStreamInfo  AudioStreamInfo;
  typedef SInterfaces::FileFormatProber::VideoStreamInfo  VideoStreamInfo;
  typedef SInterfaces::FileFormatProber::DataStreamInfo   DataStreamInfo;
  typedef SInterfaces::FileFormatProber::Chapter          Chapter;

public:
  inline                        SMediaInfo(void) : path(), pi()                 { }
  inline explicit               SMediaInfo(const QString &path) : path(path), pi() { probe(); }
  inline                        SMediaInfo(const SMediaInfo &c) : path(c.path), pi(c.pi) { probeDataStreams(); }
  inline explicit               SMediaInfo(const SInterfaces::FileFormatProber::ProbeInfo &pi) : path(), pi(pi) { }

  virtual QDomNode              toXml(QDomDocument &) const;
  inline void                   fromXml(const QDomNode &node, const QString &path) { this->path = path; SMediaInfo::fromXml(node); probeDataStreams(); }
  inline void                   fromByteArray(const QByteArray &data, const QString &path) { this->path = path; SSerializable::fromByteArray(data); probeDataStreams(); }

  inline QString                format(void) const                              { return pi.format; }

  inline bool                   containsAudio(void) const                       { return !pi.audioStreams.isEmpty(); } //!< True if the resource contains audio data.
  inline bool                   containsVideo(void) const                       { return !pi.videoStreams.isEmpty(); } //!< True if the resource contains video data.
  inline bool                   containsImage(void) const                       { return !pi.imageCodec.isNull(); } //!< True if the resource contains an image.
  inline bool                   isProbed(void) const                            { return pi.isProbed; } //!< True if the resource was deep-probed (i.e. the resource was opened and scanned).
  inline bool                   isReadable(void) const                          { return pi.isReadable; } //!< True if the resource is readable (is set to true if deep-probe can read it).

  inline const QString        & fileTypeName(void) const                        { return pi.fileTypeName; } //!< A user-friendly description of the file type.

  inline STime                  duration(void) const                            { return pi.duration; } //!< The duration of the resource, if applicable.
  inline const QList<AudioStreamInfo> & audioStreams(void) const                { return pi.audioStreams; } //!< The audio streams, if applicable.
  inline const QList<VideoStreamInfo> & videoStreams(void) const                { return pi.videoStreams; } //!< The video streams, if applicable.
  inline const QList<DataStreamInfo> & dataStreams(void) const                  { return pi.dataStreams; } //!< The data streams, if applicable.
  inline const SVideoCodec    & imageCodec(void) const                          { return pi.imageCodec; } //!< The image codec, if applicable.
  inline const QList<Chapter> & chapters(void) const                            { return pi.chapters; } //!< A list of chapters, if applicable.

  inline const QString        & title(void) const                               { return pi.title; } //!< The title (e.g. from ID3).
  inline const QString        & author(void) const                              { return pi.author; } //!< The author (e.g. from ID3).
  inline const QString        & copyright(void) const                           { return pi.copyright; } //!< The copyright (e.g. from ID3).
  inline const QString        & comment(void) const                             { return pi.comment; } //!< The comment (e.g. from ID3).
  inline const QString        & album(void) const                               { return pi.album; } //!< The album (e.g. from ID3).
  inline const QString        & genre(void) const                               { return pi.genre; } //!< The genre (e.g. from ID3).
  inline unsigned               year(void) const                                { return pi.year; } //!< The year (e.g. from ID3).
  inline unsigned               track(void) const                               { return pi.track; } //!< The track (e.g. from ID3).

  inline const QList<QByteArray> & thumbnails(void) const                       { return pi.thumbnails; } //!< A thumbnail of the resource, if applicable. In case of an image it is of at most 256x256.

public:
  static const unsigned         tvShowSeason;

private:
  virtual void                  fromXml(const QDomNode &);

  void                          probe(void);
  void                          probeDataStreams(void);

private:
  QString                       path;
  SInterfaces::FileFormatProber::ProbeInfo pi;
};

typedef QList<SMediaInfo> SMediaInfoList;

} // End of namespace

#endif
