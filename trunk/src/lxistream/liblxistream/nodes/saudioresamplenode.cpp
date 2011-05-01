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

#include "nodes/saudioresamplenode.h"
#include "sinterfaces.h"

namespace LXiStream {

struct SAudioResampleNode::Data
{
  SInterfaces::AudioResampler * resampler;
  QFuture<void>                 future;
};

SAudioResampleNode::SAudioResampleNode(SGraph *parent, const QString &algo)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->resampler = SInterfaces::AudioResampler::create(this, algo);
}

SAudioResampleNode::~SAudioResampleNode()
{
  d->future.waitForFinished();
  delete d->resampler;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioResampleNode::algorithms(void)
{
  return SInterfaces::AudioResampler::available();
}

SAudioFormat::Channels SAudioResampleNode::channels(void) const
{
  if (d->resampler)
    return d->resampler->format().channelSetup();
  else
    return SAudioFormat::Channel_None;
}

void SAudioResampleNode::setChannels(SAudioFormat::Channels c)
{
  if (d->resampler)
  {
    SAudioFormat format = d->resampler->format();
    format.setChannelSetup(c);
    d->resampler->setFormat(format);
  }
}

unsigned SAudioResampleNode::sampleRate(void) const
{
  if (d->resampler)
    return d->resampler->format().sampleRate();
  else
    return 0;
}

void SAudioResampleNode::setSampleRate(unsigned s)
{
  if (d->resampler)
  {
    SAudioFormat format = d->resampler->format();
    format.setSampleRate(s);
    d->resampler->setFormat(format);
  }
}

bool SAudioResampleNode::start(void)
{
  return true;
}

void SAudioResampleNode::stop(void)
{
  d->future.waitForFinished();
}

void SAudioResampleNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (!audioBuffer.isNull() && d->resampler)
    d->future = QtConcurrent::run(this, &SAudioResampleNode::processTask, audioBuffer);
  else
    emit output(audioBuffer);
}

void SAudioResampleNode::processTask(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  emit output(d->resampler->processBuffer(audioBuffer));
}

} // End of namespace
