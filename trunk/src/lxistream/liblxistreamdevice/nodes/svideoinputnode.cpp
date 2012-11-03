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

#include "nodes/svideoinputnode.h"
#include "sinterfaces.h"

namespace LXiStreamDevice {

struct SVideoInputNode::Data
{
  SInterfaces::VideoInput     * input;
};

SVideoInputNode::SVideoInputNode(SGraph *parent, const QString &device)
  : ::LXiStream::SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->input = SInterfaces::VideoInput::create(this, device);
}

SVideoInputNode::~SVideoInputNode()
{
  delete d->input;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoInputNode::devices(void)
{
  return SInterfaces::VideoInput::available();
}

void SVideoInputNode::setFormat(const SVideoFormat &format)
{
  if (d->input)
    d->input->setFormat(format);
}

SVideoFormat SVideoInputNode::format() const
{
  if (d->input)
    return d->input->format();

  return SVideoFormat();
}

void SVideoInputNode::setMaxBuffers(int maxBuffers)
{
  if (d->input)
    d->input->setMaxBuffers(maxBuffers);
}

bool SVideoInputNode::start(void)
{
  if (d->input && d->input->start())
  {
    connect(d->input, SIGNAL(produce(const SVideoBuffer &)), SIGNAL(output(const SVideoBuffer &)));
    return true;
  }

  qWarning() << "Failed to open video input device";
  return false;
}

void SVideoInputNode::stop(void)
{
  if (d->input)
    d->input->stop();
}

bool SVideoInputNode::process(void)
{
  if (d->input)
    return d->input->process();

  return false;
}

} // End of namespace
