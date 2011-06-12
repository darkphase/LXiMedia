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
#include "backendserver.h"
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC MediaStream : public SGraph
{
Q_OBJECT
protected:
  struct Audio
  {
    inline Audio(SGraph *parent)
      : matrix(parent), resampler(parent), encoder(parent)
    {
    }

    SAudioMatrixNode            matrix;
    SAudioResampleNode          resampler;
    SAudioEncoderNode           encoder;
  };

  struct Video
  {
    inline Video(SGraph *parent)
      : deinterlacer(parent), subpictureRenderer(parent),
        letterboxDetectNode(parent), resizer(parent), box(parent),
        subtitleRenderer(parent), encoder(parent)
    {
    }

    SVideoDeinterlaceNode       deinterlacer;
    SSubpictureRenderNode       subpictureRenderer;
    SVideoLetterboxDetectNode   letterboxDetectNode;
    SVideoResizeNode            resizer;
    SVideoBoxNode               box;
    SSubtitleRenderNode         subtitleRenderer;
    SVideoEncoderNode           encoder;
  };

public:
  explicit                      MediaStream(void);
  virtual                       ~MediaStream();

  bool                          setup(const SHttpServer::RequestMessage &,
                                      QAbstractSocket *,
                                      STime duration,
                                      SInterval frameRate,
                                      SSize size,
                                      SAudioFormat::Channels,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None,
                                      SInterfaces::VideoEncoder::Flags = SInterfaces::VideoEncoder::Flag_None);
  bool                          setup(const SHttpServer::RequestMessage &,
                                      QAbstractSocket *,
                                      STime duration,
                                      SAudioFormat::Channels,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None);

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
                                      QAbstractSocket *,
                                      SInterfaces::BufferReaderNode *,
                                      STime duration = STime(),
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None,
                                      SInterfaces::VideoEncoder::Flags = SInterfaces::VideoEncoder::Flag_None);

public:
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
  STimeStampResamplerNode       timeStampResampler;
};

} // End of namespace

#endif
