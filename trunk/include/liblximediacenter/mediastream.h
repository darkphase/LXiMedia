/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#ifndef LXIMEDIACENTER_MEDIASTREAM_H
#define LXIMEDIACENTER_MEDIASTREAM_H

#include <QtCore>
#include <QtNetwork>
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

  bool                          setup(const QUrl &,
                                      STime duration,
                                      const SAudioFormat &,
                                      const SVideoFormat &,
                                      bool enableNormalize = false,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None,
                                      SInterfaces::VideoEncoder::Flags = SInterfaces::VideoEncoder::Flag_None,
                                      int videoGopSize = -1);
  bool                          setup(const QUrl &,
                                      STime duration,
                                      const SAudioFormat &,
                                      bool enableNormalize = false,
                                      SInterfaces::AudioEncoder::Flags = SInterfaces::AudioEncoder::Flag_None);

  inline const QUrl           & getRequest() const { return request; }
  inline const QByteArray     & getContentType() const { return contentType; }
  inline QIODevice            * createReader(QObject *parent = NULL) { return proxy.createReader(parent); }
  inline bool                   isActive() const { return proxy.isActive(); }
  inline bool                   isReusable() const { return proxy.isReusable(); }

  static SSize                  decodeSize(const QUrl &);
  static void                   decodeSize(const QUrl &, SVideoFormat &, Qt::AspectRatioMode &);
  static SAudioFormat::Channels decodeChannels(const QUrl &);
  static void                   decodeChannels(const QUrl &, SAudioFormat &);

protected:
  static SSize                  toStandardVideoSize(const SSize &);

protected:
  QUrl                          request;
  Audio                       * audio;
  Video                       * video;
  STimeStampSyncNode            sync;
  SIOOutputNode                 output;
  SIODeviceProxy                proxy;
  QByteArray                    contentType;
};

class LXIMEDIACENTER_PUBLIC MediaTranscodeStream : public MediaStream
{
public:
  explicit                      MediaTranscodeStream(void);
  virtual                       ~MediaTranscodeStream();

  bool                          setup(const QUrl &,
                                      SInputNode *,
                                      STime duration = STime(),
                                      bool enableNormalize = false,
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
