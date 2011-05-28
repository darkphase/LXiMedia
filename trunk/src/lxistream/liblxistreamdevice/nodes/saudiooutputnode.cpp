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

#include "nodes/saudiooutputnode.h"
#include "sinterfaces.h"

namespace LXiStreamDevice {

struct SAudioOutputNode::Data
{
  QString                       device;
  SInterfaces::AudioOutput    * output;
  STimer                      * timer;
  STime                         delay;
  SAudioBufferList              delayList;
};

SAudioOutputNode::SAudioOutputNode(SGraph *parent, const QString &device)
  : QObject(parent),
    SGraph::SinkNode(parent),
    d(new Data())
{
  d->device = device;
  d->output = NULL;
  d->timer = NULL;
  d->delay = STime::null;
}

SAudioOutputNode::~SAudioOutputNode()
{
  delete d->output;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioOutputNode::devices(void)
{
  return SInterfaces::AudioOutput::available();
}

bool SAudioOutputNode::start(STimer *timer)
{
  delete d->output;
  d->output = SInterfaces::AudioOutput::create(this, d->device);

  if (d->output)
  {
    d->timer = timer;
    return true;
  }
  else
  {
    qWarning() << "Failed to open audio output device" << d->device;
    return false;
  }
}

void SAudioOutputNode::stop(void)
{
  d->timer = NULL;

  delete d->output;
  d->output = NULL;
}

void SAudioOutputNode::setDelay(STime delay)
{
  d->delay = delay;
}

STime SAudioOutputNode::delay(void) const
{
  return d->delay;
}

void SAudioOutputNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  if (d->output)
  {
    if (!audioBuffer.isNull())
    {
      const STime latency = d->output->latency();

      d->delayList.append(audioBuffer);
      while (!d->delayList.isEmpty())
      if ((latency + audioBuffer.timeStamp() - d->delayList.first().timeStamp()) >= d->delay)
      {
        const SAudioBuffer buffer = d->delayList.takeFirst();

        if (d->timer)
          d->timer->correctOffset(buffer.timeStamp() + latency);

        d->output->consume(buffer);
      }
      else
        break;
    }
    else // Flush
    {
      foreach (const SAudioBuffer &buffer, d->delayList)
        d->output->consume(buffer);

      d->delayList.clear();
    }
  }
}

} // End of namespace
