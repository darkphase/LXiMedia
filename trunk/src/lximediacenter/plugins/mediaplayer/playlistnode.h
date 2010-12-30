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

#ifndef PLAYLISTNODE_H
#define PLAYLISTNODE_H

#include <QtCore>
#include <LXiStream>
#include "mediadatabase.h"
#include "playlist.h"

namespace LXiMediaCenter {

class PlaylistNode : public QObject,
                     public SInterfaces::SourceNode
{
Q_OBJECT
Q_PROPERTY(quint32 channels READ __internal_channels WRITE __internal_setChannels)
Q_PROPERTY(unsigned sampleRate READ sampleRate WRITE setSampleRate)
protected:
  class Input
  {
  public:
    inline Input(MediaDatabase::UniqueID song, const QString &fileName)
      : song(song), file(NULL, fileName), audioDecoder(NULL),
        audioResampler(NULL, "linear")
    {
    }

    const MediaDatabase::UniqueID song;
    STime                       duration;
    SFileInputNode              file;
    SAudioDecoderNode           audioDecoder;
    SAudioResampleNode          audioResampler;
  };

public:
                                PlaylistNode(SGraph *, MediaDatabase *, Playlist *);
  virtual                       ~PlaylistNode();

  SAudioFormat::Channels        channels(void) const;
  void                          setChannels(SAudioFormat::Channels);
  unsigned                      sampleRate(void) const;
  void                          setSampleRate(unsigned);
  void                          enableVideo(const SSize &, SInterval);

  Playlist                    * playlist(void);
  MediaDatabase::UniqueID       currentSong(void) const;

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

public slots:
  void                          nextSong(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          finished(void);

private:
  inline quint32                __internal_channels(void) const                 { return quint32(channels()); }
  inline void                   __internal_setChannels(quint32 c)               { setChannels(SAudioFormat::Channels(c)); }

  void                          computeBuffer(Input *);
  void                          closeInput(Input *);
  void                          playSong(const MediaDatabase::Node &);
  void                          computeSplash(const MediaDatabase::Node &);

  static const QImage         & defaultBackground(void) __attribute__((pure));

private slots:
  void                          decoded(const SAudioBuffer &);

private:
  MediaDatabase         * const mediaDatabase;
  Playlist              * const activePlaylist;
  SAudioFormat                  outFormat;

  mutable QMutex                mutex;
  Input                       * input;
  SAudioBufferList              inputQueue;
  STime                         timeStamp;

  bool                          videoEnabled;
  SSize                         videoSize;
  SInterval                     videoFrameRate;
  STime                         videoTime;
  SVideoBuffer                  videoBuffer, videoBufferSub;
  QStringList                   videoBufferText;
};

} // End of namespace

#endif
