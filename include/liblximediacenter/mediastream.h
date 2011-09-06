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

#ifndef LXIMEDIACENTER_MEDIASTREAM_H
#define LXIMEDIACENTER_MEDIASTREAM_H

#include <QtCore>
#include <QtNetwork>
#include <LXiServer>
#include <LXiStream>
#include <LXiStreamGui>
#include "backendserver.h"
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC MediaStream : public SGraph
{
Q_OBJECT
protected:
  struct LXIMEDIACENTER_PUBLIC Audio
  {
    explicit                    Audio(SGraph *parent);
                                ~Audio();

    SAudioFormat::Channels      outChannels;
    SAudioMatrixNode            matrix;
    SAudioResampleNode          resampler;
    SAudioGapRemoverNode        gapRemover;
    SAudioNormalizeNode         audioNormalizer;
    SAudioEncoderNode           encoder;
  };

  struct LXIMEDIACENTER_PUBLIC Video
  {
    explicit                    Video(SGraph *parent);
                                ~Video();

    SVideoDeinterlaceNode       deinterlacer;
    SVideoLetterboxDetectNode   letterboxDetectNode;
    SSubpictureRenderNode       subpictureRenderer;
    SVideoResizeNode            resizer;
    SVideoBoxNode               box;
    SSubtitleRenderNode         subtitleRenderer;
    SVideoEncoderNode           encoder;
  };

public:
                                MediaStream(void);
  virtual                       ~MediaStream();

  bool                          setup(const SHttpServer::RequestMessage &,
                                      QIODevice *,
                                      STime position, STime duration,
                                      const SAudioFormat &,
                                      const SVideoFormat &,
                                      bool musicPlaylist = false,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None,
                                      SInterfaces::VideoEncoder::Flags = SInterfaces::VideoEncoder::Flag_None);
  bool                          setup(const SHttpServer::RequestMessage &,
                                      QIODevice *,
                                      STime position, STime duration,
                                      const SAudioFormat &,
                                      bool musicPlaylist = false,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None);

  static SSize                  decodeSize(const QUrl &);
  static void                   decodeSize(const QUrl &, SVideoFormat &, Qt::AspectRatioMode &);
  static SAudioFormat::Channels decodeChannels(const QUrl &);
  static void                   decodeChannels(const QUrl &, SAudioFormat &);

protected:
  Audio                       * audio;
  Video                       * video;
  STimeStampSyncNode            sync;
  SIOOutputNode                 output;
};

class LXIMEDIACENTER_PUBLIC MediaTranscodeStream : public MediaStream
{
public:
  explicit                      MediaTranscodeStream(void);

  bool                          setup(const SHttpServer::RequestMessage &,
                                      QIODevice *,
                                      SInterfaces::AbstractBufferReader *,
                                      STime duration = STime(),
                                      bool musicPlaylist = false,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None,
                                      SInterfaces::VideoEncoder::Flags = SInterfaces::VideoEncoder::Flag_None);

public:
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
  SVideoGeneratorNode           videoGenerator;
  STimeStampResamplerNode       timeStampResampler;
};

} // End of namespace

#endif
