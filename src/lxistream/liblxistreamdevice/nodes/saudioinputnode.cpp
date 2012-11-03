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

#include "nodes/saudioinputnode.h"
#include "sinterfaces.h"

namespace LXiStreamDevice {

struct SAudioInputNode::Data
{
  SInterfaces::AudioInput     * input;
};

SAudioInputNode::SAudioInputNode(SGraph *parent, const QString &device)
  : ::LXiStream::SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->input = SInterfaces::AudioInput::create(this, device);
}

SAudioInputNode::~SAudioInputNode()
{
  delete d->input;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioInputNode::devices(void)
{
  return SInterfaces::AudioInput::available();
}

void SAudioInputNode::setFormat(const SAudioFormat &format)
{
  if (d->input)
    d->input->setFormat(format);
}

SAudioFormat SAudioInputNode::format() const
{
  if (d->input)
    return d->input->format();

  return SAudioFormat();
}

bool SAudioInputNode::start(void)
{
  if (d->input && d->input->start())
  {
    connect(d->input, SIGNAL(produce(const SAudioBuffer &)), SIGNAL(output(const SAudioBuffer &)));
    return true;
  }

  qWarning() << "Failed to open audio input device";
  return false;
}

void SAudioInputNode::stop(void)
{
  if (d->input)
    d->input->stop();
}

bool SAudioInputNode::process(void)
{
  if (d->input)
    return d->input->process();

  return false;
}

} // End of namespace
