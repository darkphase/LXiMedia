/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef STREAMINPUTNODE_H
#define STREAMINPUTNODE_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace InternetBackend {

class StreamInputNode : public SNetworkInputNode
{
Q_OBJECT
public:
                                StreamInputNode(SGraph *parent);
  virtual                       ~StreamInputNode();
  
  SSize                         size(void) const;
  void                          setSize(const SSize &size);
  SInterval                     frameRate(void) const;
  void                          setFrameRate(const SInterval &);
  SAudioFormat::Channels        channelSetup(void) const;
  void                          setChannelSetup(SAudioFormat::Channels);
  unsigned                      sampleRate(void) const;
  void                          setSampleRate(unsigned);

  void                          setUrl(const QUrl &, bool generateVideo);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);

protected: // From SInterfaces::AbstractBufferReader::ProduceCallback
  virtual void                  produce(const SEncodedAudioBuffer &);
  virtual void                  produce(const SEncodedVideoBuffer &);
  virtual void                  produce(const SEncodedDataBuffer &);

private:
  STime                         correct(const STime &);
  void                          computeBufferingFrame(const STime &);
  void                          startTask(void);

private:
  SSize                         outSize;
  SAudioBuffer                  audioBuffer;
  SInterval                     baseFrameRate;
  SAudioFormat::Channels        baseChannelSetup;
  unsigned                      baseSampleRate;

  SImage                        baseImage;
  QList<QImage>                 bufferingImages;
  STime                         bufferingTime;
  STimer                        bufferingTimer;
  STime                         correctTime;
  STime                         streamTime;

  QSemaphore                    startSem;
};

} } // End of namespaces

#endif
