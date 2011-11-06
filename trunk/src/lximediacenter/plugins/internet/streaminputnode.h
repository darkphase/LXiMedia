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

#ifndef STREAMINPUTNODE_H
#define STREAMINPUTNODE_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace InternetBackend {

class StreamInputNode : public SInterfaces::SourceNode
{
Q_OBJECT
public:
                                StreamInputNode(SGraph *parent, const QUrl &url);
  virtual                       ~StreamInputNode();

  SSize                         size(void) const;
  void                          setSize(const SSize &size);
  SInterval                     frameRate(void) const;
  void                          setFrameRate(const SInterval &);
  SAudioFormat::Channels        channelSetup(void) const;
  void                          setChannelSetup(SAudioFormat::Channels);
  unsigned                      sampleRate(void) const;
  void                          setSampleRate(unsigned);

  bool                          open(bool hasVideo, bool generateVideo);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          output(const SSubpictureBuffer &);
  void                          output(const SSubtitleBuffer &);
  void                          finished(void);

private slots:
  void                          setBufferState(bool, float);
  void                          correct(SAudioBuffer);
  void                          correct(SVideoBuffer);
  void                          correct(SSubpictureBuffer);
  void                          correct(SSubtitleBuffer);

private:
  STime                         correct(const STime &);
  void                          computeBufferingFrame(const STime &);

private:
  SSize                         outSize;
  SAudioBuffer                  audioBuffer;
  SInterval                     baseFrameRate;
  SAudioFormat::Channels        baseChannelSetup;
  unsigned                      baseSampleRate;

  SNetworkInputNode             networkInput;
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
  SVideoGeneratorNode           videoGenerator;

  SImage                        baseImage;
  bool                          bufferState;
  float                         bufferProgress;
  QList<QImage>                 bufferingImages;
  STime                         bufferingTime;
  STimer                        bufferingTimer;
  static const qint64           minBufferingTimeMs;
  STime                         correctTime;

  QSemaphore                    startSem;
};

} } // End of namespaces

#endif
