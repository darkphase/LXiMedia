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
#include "sdebug.h"
#include "sinterfaces.h"
#include "sstringparser.h"

namespace LXiStream {


struct SAudioVideoInputNode::Data
{
  QString                       device;
  SVideoFormat                  format;
  int                           maxBuffers;
  SInterfaces::AudioInput     * audioInput;
  SInterfaces::VideoInput     * videoInput;

  STime                         audioTime;
  STime                         videoTime;
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
  delete d->audioInput;
  d->audioInput = NULL;
  d->audioTime = STime::null;

  delete d->videoInput;
  d->videoInput = SInterfaces::VideoInput::create(this, d->device);
  d->videoTime = STime::null;

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
          connect(d->audioInput, SIGNAL(produce(const SAudioBuffer &)), SLOT(produced(const SAudioBuffer &)), Qt::DirectConnection);
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
      connect(d->videoInput, SIGNAL(produce(const SVideoBuffer &)), SLOT(produced(const SVideoBuffer &)), Qt::DirectConnection);
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
  if (d->audioInput)
  {
    d->audioInput->stop();

    delete d->audioInput;
    d->audioInput = NULL;
  }

  if (d->videoInput)
  {
    d->videoInput->stop();

    delete d->videoInput;
    d->videoInput = NULL;
  }
}

void SAudioVideoInputNode::process(void)
{
  if (d->audioInput && d->videoInput)
  {
    if (d->audioTime <= d->videoTime)
      d->audioInput->process();
    else
      d->videoInput->process();
  }
  else if (d->videoInput)
    d->videoInput->process();
}

void SAudioVideoInputNode::produced(const SAudioBuffer &buffer)
{
  d->audioTime = buffer.timeStamp();
}

void SAudioVideoInputNode::produced(const SVideoBuffer &buffer)
{
  d->videoTime = buffer.timeStamp();
}


} // End of namespace
