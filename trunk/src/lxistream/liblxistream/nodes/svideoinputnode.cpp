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
#include "sdebug.h"
#include "sinterfaces.h"
#include "sgraph.h"

namespace LXiStream {


struct SVideoInputNode::Data
{
  QString                       device;
  SInterfaces::VideoInput     * input;
};

SVideoInputNode::SVideoInputNode(SGraph *parent, const QString &device)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->device = device;
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

bool SVideoInputNode::start(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  delete d->input;
  d->input = SInterfaces::VideoInput::create(this, d->device);

  if (d->input)
    if (d->input->start())
  {
    connect(d->input, SIGNAL(produce(const SVideoBuffer &)), SIGNAL(output(const SVideoBuffer &)));
    return true;
  }

  qWarning() << "Failed to open video input device" << d->device;
  return false;
}

void SVideoInputNode::stop(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  d->input->stop();

  delete d->input;
  d->input = NULL;
}

void SVideoInputNode::process(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  d->input->process();
}


} // End of namespace
