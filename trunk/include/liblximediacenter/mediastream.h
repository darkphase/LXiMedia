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

namespace LXiMediaCenter {

class MediaStream : public SGraph
{
Q_OBJECT
public:
  explicit                      MediaStream(void);

  bool                          setup(const SHttpServer::RequestHeader &, QIODevice *, STime duration, SInterval frameRate, SSize size, SAudioFormat::Channels channels);
  bool                          setup(const SHttpServer::RequestHeader &, QIODevice *, STime duration, SAudioFormat::Channels channels);

protected:
  STimeStampResamplerNode       timeStampResampler;
  SAudioResampleNode            audioResampler;
  SVideoDeinterlaceNode         deinterlacer;
  SSubpictureRenderNode         subpictureRenderer;
  SVideoLetterboxDetectNode     letterboxDetectNode;
  SVideoResizeNode              videoResizer;
  SVideoBoxNode                 videoBox;
  SSubtitleRenderNode           subtitleRenderer;
  STimeStampSyncNode            sync;
  SAudioEncoderNode             audioEncoder;
  SVideoEncoderNode             videoEncoder;
  SIOOutputNode                 output;
};

class MediaTranscodeStream : public MediaStream
{
public:
  explicit                      MediaTranscodeStream(void);

  bool                          setup(const SHttpServer::RequestHeader &, QIODevice *, SInterfaces::BufferReaderNode *, STime duration = STime());

public:
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
};

} // End of namespace

#endif
