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

#include "nodes/saudiovideoinputnode.h"
#include "sinterfaces.h"

namespace LXiStreamDevice {

template <class _input>
class SAudioVideoInputNode::Thread : public QThread
{
public:
  inline Thread(QObject *parent, _input *input)
    : QThread(parent), input(input), running(true)
  {
    start();
  }

  virtual ~Thread()
  {
    running = false;
    QThread::wait();
  }

protected:
  virtual void run(void)
  {
    while (running)
      input->process();
  }

private:
  _input                * const input;
  volatile bool                 running;
};

class SAudioVideoInputNode::SilentAudioInput : public SInterfaces::AudioInput
{
public:
                                SilentAudioInput(QObject *parent);
  virtual                       ~SilentAudioInput();

  virtual void                  setFormat(const SAudioFormat &);
  virtual SAudioFormat          format(void);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

private:
  SInterval                     bufferRate;
  STimer                        timer;
  STime                         lastBuffer;
  SAudioFormat                  audioFormat;
  SAudioBuffer                  audioBuffer;
};

struct SAudioVideoInputNode::Data
{
  QString                       device;
  SAudioFormat                  audioFormat;
  SVideoFormat                  videoFormat;
  int                           maxBuffers;

  SInterfaces::AudioInput     * audioInput;
  SInterfaces::VideoInput     * videoInput;
  Thread<SInterfaces::AudioInput> * audioThread;
  Thread<SInterfaces::VideoInput> * videoThread;
};

SAudioVideoInputNode::SAudioVideoInputNode(SGraph *parent, const QString &device)
  : ::LXiStream::SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->device = device;
  d->maxBuffers = 0;
  d->audioInput = NULL;
  d->videoInput = NULL;
  d->audioThread = NULL;
  d->videoThread = NULL;
}

SAudioVideoInputNode::~SAudioVideoInputNode()
{
  delete d->audioInput;
  delete d->videoInput;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioVideoInputNode::devices(void)
{
  return SInterfaces::VideoInput::available();
}

void SAudioVideoInputNode::setFormat(const SAudioFormat &audioFormat, const SVideoFormat &videoFormat)
{
  d->audioFormat = audioFormat;
  d->videoFormat = videoFormat;
}

void SAudioVideoInputNode::setMaxBuffers(int maxBuffers)
{
  d->maxBuffers = maxBuffers;
}

bool SAudioVideoInputNode::start(void)
{
  delete d->audioThread;
  d->audioThread = NULL;

  delete d->audioInput;
  d->audioInput = NULL;

  delete d->videoThread;
  d->videoThread = NULL;

  delete d->videoInput;
  d->videoInput = SInterfaces::VideoInput::create(this, d->device);

  if (d->videoInput)
  {
    const QString myName = SStringParser::toRawName(d->device);
    QString bestMatch = QString::null;
    qreal match = 0.0, weight = 1.0;

    foreach (const QString &audioDevice, SInterfaces::AudioInput::available())
    {
      const qreal m = SStringParser::computeBidirMatch(SStringParser::toRawName(audioDevice), myName);
      if (m > match)
      {
        bestMatch = audioDevice;
        match = m * weight;
        weight *= 0.9;
      }
    }

    if (!bestMatch.isEmpty())
    { // Found one
      d->audioInput = SInterfaces::AudioInput::create(this, bestMatch);
      if (d->audioInput)
      {
        if (!d->audioFormat.isNull())
          d->audioInput->setFormat(d->audioFormat);

        if (d->audioInput->start())
        {
          connect(d->audioInput, SIGNAL(produce(const SAudioBuffer &)), SIGNAL(output(const SAudioBuffer &)));
          d->audioThread = new Thread<SInterfaces::AudioInput>(this, d->audioInput);
        }
        else
        {
          delete d->audioInput;
          d->audioInput = NULL;
        }
      }
    }

    if (d->audioInput == NULL)
    {
      d->audioInput = new SilentAudioInput(this);

      if (!d->audioFormat.isNull())
        d->audioInput->setFormat(d->audioFormat);

      d->audioInput->start();

      connect(d->audioInput, SIGNAL(produce(const SAudioBuffer &)), SIGNAL(output(const SAudioBuffer &)));
      d->audioThread = new Thread<SInterfaces::AudioInput>(this, d->audioInput);
    }

    if (!d->videoFormat.isNull())
      d->videoInput->setFormat(d->videoFormat);

    if (d->maxBuffers > 0)
      d->videoInput->setMaxBuffers(d->maxBuffers);

    if (d->videoInput->start())
    {
      connect(d->videoInput, SIGNAL(produce(const SVideoBuffer &)), SIGNAL(output(const SVideoBuffer &)));
      d->videoThread = new Thread<SInterfaces::VideoInput>(this, d->videoInput);
      return true;
    }
  }

  delete d->audioInput;
  d->audioInput = NULL;

  delete d->videoInput;
  d->videoInput = NULL;

  qWarning() << "Failed to open audio/video input device" << d->device;
  return false;
}

void SAudioVideoInputNode::stop(void)
{
  delete d->audioThread;
  d->audioThread = NULL;

  if (d->audioInput)
    d->audioInput->stop();

  delete d->videoThread;
  d->videoThread = NULL;

  if (d->videoInput)
    d->videoInput->stop();
}

bool SAudioVideoInputNode::process(void)
{
  // The threads do all the work.
  return false;
}


SAudioVideoInputNode::SilentAudioInput::SilentAudioInput(QObject *parent)
  : SInterfaces::AudioInput(parent),
    bufferRate(SInterval::fromFrequency(25)),
    audioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 48000)
{
}

SAudioVideoInputNode::SilentAudioInput::~SilentAudioInput()
{
}

void SAudioVideoInputNode::SilentAudioInput::setFormat(const SAudioFormat &format)
{
  audioFormat = format;
}

SAudioFormat SAudioVideoInputNode::SilentAudioInput::format(void)
{
  return audioFormat;
}

bool SAudioVideoInputNode::SilentAudioInput::start(void)
{
  // Create a silent audio buffer
  audioBuffer.setFormat(audioFormat);
  audioBuffer.setNumSamples(audioFormat.sampleRate() / bufferRate.toFrequency());
  memset(audioBuffer.data(), 0, audioBuffer.size());

  lastBuffer = timer.timeStamp();

  return true;
}

void SAudioVideoInputNode::SilentAudioInput::stop(void)
{
}

bool SAudioVideoInputNode::SilentAudioInput::process(void)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  const STime now = timer.timeStamp();
  const STime next = lastBuffer + bufferRate;
  const qint64 wait = (next - now).toMSec();

  if (qAbs(wait) >= 1000)
    lastBuffer = now;
  else if (wait > 0)
    T::msleep(wait);

  lastBuffer += bufferRate;

  audioBuffer.setTimeStamp(lastBuffer);
  emit produce(audioBuffer);

  return true;
}

} // End of namespace
