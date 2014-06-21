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

#include "nodes/saudioresamplenode.h"
#include "sinterfaces.h"

namespace LXiStream {

struct SAudioResampleNode::Data
{
  SInterfaces::AudioResampler * resampler;
};

SAudioResampleNode::SAudioResampleNode(SGraph *parent, const QString &algo)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->resampler = SInterfaces::AudioResampler::create(this, algo);
}

SAudioResampleNode::~SAudioResampleNode()
{
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
}

void SAudioResampleNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && d->resampler)
    emit output(d->resampler->processBuffer(audioBuffer));
  else
    emit output(audioBuffer);
}

void SAudioResampleNode::compensate(float frac)
{
  d->resampler->compensate(frac);
}

} // End of namespace
