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

#include "nodes/svideoinputnode.h"
#include "sinterfaces.h"

namespace LXiStreamDevice {

struct SVideoInputNode::Data
{
  QString                       device;
  SVideoFormat                  format;
  int                           maxBuffers;
  SInterfaces::VideoInput     * input;
};

SVideoInputNode::SVideoInputNode(SGraph *parent, const QString &device)
  : ::LXiStream::SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->device = device;
  d->maxBuffers = 0;
  d->input = NULL;
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
  d->format = format;
}

void SVideoInputNode::setMaxBuffers(int maxBuffers)
{
  d->maxBuffers = maxBuffers;
}

bool SVideoInputNode::start(void)
{
  delete d->input;
  d->input = SInterfaces::VideoInput::create(this, d->device);

  if (d->input)
  {
    if (!d->format.isNull())
      d->input->setFormat(d->format);

    if (d->maxBuffers > 0)
      d->input->setMaxBuffers(d->maxBuffers);

    if (d->input->start())
    {
      connect(d->input, SIGNAL(produce(const SVideoBuffer &)), SIGNAL(output(const SVideoBuffer &)));
      return true;
    }
  }

  delete d->input;
  d->input = NULL;

  qWarning() << "Failed to open video input device" << d->device;
  return false;
}

void SVideoInputNode::stop(void)
{
  if (d->input)
  {
    d->input->stop();

    delete d->input;
    d->input = NULL;
  }
}

bool SVideoInputNode::process(void)
{
  if (d->input)
    return d->input->process();

  return false;
}

} // End of namespace
