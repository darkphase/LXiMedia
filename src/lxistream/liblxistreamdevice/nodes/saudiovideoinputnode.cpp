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

struct SAudioVideoInputNode::Data
{
  QString                       device;
  SVideoFormat                  format;
  int                           maxBuffers;

  SInterfaces::AudioInput     * audioInput;
  SInterfaces::VideoInput     * videoInput;
  Thread<SInterfaces::AudioInput> * audioThread;
  Thread<SInterfaces::VideoInput> * videoThread;
};

SAudioVideoInputNode::SAudioVideoInputNode(SGraph *parent, const QString &device)
  : QObject(parent),
    SGraph::SourceNode(parent),
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

void SAudioVideoInputNode::setFormat(const SVideoFormat &format)
{
  d->format = format;
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

    if (!d->format.isNull())
      d->videoInput->setFormat(d->format);

    if (d->maxBuffers > 0)
      d->videoInput->setMaxBuffers(d->maxBuffers);

    if (d->videoInput->start())
    {
      connect(d->videoInput, SIGNAL(produce(const SVideoBuffer &)), SIGNAL(output(const SVideoBuffer &)));
      d->videoThread = new Thread<SInterfaces::VideoInput>(this, d->videoInput);
      return true;
    }
  }

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
  {
    d->audioInput->stop();

    delete d->audioInput;
    d->audioInput = NULL;
  }

  delete d->videoThread;
  d->videoThread = NULL;

  if (d->videoInput)
  {
    d->videoInput->stop();

    delete d->videoInput;
    d->videoInput = NULL;
  }
}

void SAudioVideoInputNode::process(void)
{
  // The threads do all the work.
}

} // End of namespace
