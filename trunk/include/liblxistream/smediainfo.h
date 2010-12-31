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

class SMediaInfo : public SSerializable
{
public:
  typedef SInterfaces::FormatProber::AudioStreamInfo  AudioStreamInfo;
  typedef SInterfaces::FormatProber::VideoStreamInfo  VideoStreamInfo;
  typedef SInterfaces::FormatProber::DataStreamInfo   DataStreamInfo;
  typedef SInterfaces::FormatProber::Chapter          Chapter;

  /*  class FingerPrint : public SSerializable
  {
  public:
                                FingerPrint(void);
                                FingerPrint(const FingerPrint &);

    FingerPrint               & operator=(const FingerPrint &c);

    float                       delta(const FingerPrint &) const;
    inline float                operator-(const FingerPrint &c) const           { return delta(c); }

    void                        add(const FingerPrint &, float = 1.0f);
    inline FingerPrint          operator+(const FingerPrint &c) const           { FingerPrint f(*this); f.add(c); return f; }
    inline FingerPrint        & operator+=(const FingerPrint &c)                { add(c); return *this; }

    bool                        isNull(void) const;
    const float               * getBins(void) const;

    virtual QDomNode            toXml(QDomDocument &) const;
    virtual void                fromXml(const QDomNode &);

  public:
    static const unsigned       numBins = 128;
    float                       bins[numBins];

  private:
    unsigned                    summed;
  };
*/
public:
  inline                        SMediaInfo(void)                                { d.deepProbe = true; }
  inline explicit               SMediaInfo(const QString &file)                 { d.file = file; d.deepProbe = true; }
  inline                        SMediaInfo(const SMediaInfo &c)                 { d.file = c.d.file; d.deepProbe = c.d.deepProbe; d.pi = c.d.pi; }

  virtual QDomNode              toXml(QDomDocument &) const;
  virtual void                  fromXml(const QDomNode &);

  inline void                   setFile(const QString &file)                    { d.file = file; d.pi.isProbed = false; }
  inline void                   setDeepProbe(bool p)                            { d.deepProbe = p; d.pi.isProbed = false; }

  inline QString                file(void) const                                { return d.file; }

  inline QString                format(void) const                              { return d.pi.format; }

  inline bool                   containsAudio(void) const                       { probe(); return !d.pi.audioStreams.isEmpty(); } //!< True if the resource contains audio data.
  inline bool                   containsVideo(void) const                       { probe(); return !d.pi.videoStreams.isEmpty(); } //!< True if the resource contains video data.
  inline bool                   containsImage(void) const                       { probe(); return !d.pi.imageCodec.isNull(); } //!< True if the resource contains an image.
  inline bool                   isProbed(void) const                            { probe(); return d.pi.isProbed; } //!< True if the resource was deep-probed (i.e. the resource was opened and scanned).
  inline bool                   isReadable(void) const                          { probe(); return d.pi.isReadable; } //!< True if the resource is readable (is set to true if deep-probe can read it).

  inline const QString        & fileTypeName(void) const                        { probe(); return d.pi.fileTypeName; } //!< A user-friendly description of the file type.

  inline STime                  duration(void) const                            { probe(); return d.pi.duration; } //!< The duration of the resource, if applicable.
  inline const QList<AudioStreamInfo> & audioStreams(void) const                { probe(); return d.pi.audioStreams; } //!< The audio streams, if applicable.
  inline const QList<VideoStreamInfo> & videoStreams(void) const                { probe(); return d.pi.videoStreams; } //!< The video streams, if applicable.
  inline const QList<DataStreamInfo> & dataStreams(void) const                  { probeDataStreams(); return d.pi.dataStreams; } //!< The data streams, if applicable.
  inline const SVideoCodec    & imageCodec(void) const                          { probe(); return d.pi.imageCodec; } //!< The image codec, if applicable.
  inline const QList<Chapter> & chapters(void) const                            { probe(); return d.pi.chapters; } //!< A list of chapters, if applicable.

  inline const QString        & title(void) const                               { probe(); return d.pi.title; } //!< The title (e.g. from ID3).
  inline const QString        & author(void) const                              { probe(); return d.pi.author; } //!< The author (e.g. from ID3).
  inline const QString        & copyright(void) const                           { probe(); return d.pi.copyright; } //!< The copyright (e.g. from ID3).
  inline const QString        & comment(void) const                             { probe(); return d.pi.comment; } //!< The comment (e.g. from ID3).
  inline const QString        & album(void) const                               { probe(); return d.pi.album; } //!< The album (e.g. from ID3).
  inline const QString        & genre(void) const                               { probe(); return d.pi.genre; } //!< The genre (e.g. from ID3).
  inline unsigned               year(void) const                                { probe(); return d.pi.year; } //!< The year (e.g. from ID3).
  inline unsigned               track(void) const                               { probe(); return d.pi.track; } //!< The track (e.g. from ID3).

  inline const QList<QByteArray> & thumbnails(void) const                       { probe(); return d.pi.thumbnails; } //!< A thumbnail of the resource, if applicable. In case of an image it is of at most 256x256.

  //FingerPrint                   fingerprint(void) const;

public:
  static const unsigned         tvShowSeason;

private:
  void                          probe(void) const;
  void                          probeDataStreams(void) const;

private:
  struct
  {
    QString                     file;
    bool                        deepProbe;

    mutable SInterfaces::FormatProber::ProbeInfo pi;
  }                             d;
};


} // End of namespace

#endif
