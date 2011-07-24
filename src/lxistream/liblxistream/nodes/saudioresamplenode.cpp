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
  : SInterfaces::Node(parent),
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

unsigned SAudioResampleNode::sampleRate(void) const
{
  if (d->resampler)
    return d->resampler->sampleRate();
  else
    return 0;
}

void SAudioResampleNode::setSampleRate(unsigned rate)
{
  if (d->resampler)
    d->resampler->setSampleRate(rate);
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
  LXI_PROFILE_WAIT(d->future.waitForFinished());
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  if (!audioBuffer.isNull() && d->resampler)
    d->future = QtConcurrent::run(this, &SAudioResampleNode::processTask, audioBuffer);
  else
    emit output(audioBuffer);
}

void SAudioResampleNode::compensate(float frac)
{
  LXI_PROFILE_WAIT(d->future.waitForFinished());
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  d->resampler->compensate(frac);
}

void SAudioResampleNode::processTask(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  emit output(d->resampler->processBuffer(audioBuffer));
}

} // End of namespace
