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
#include "sgraph.h"
#include "stimer.h"

namespace LXiStream {


struct SAudioOutputNode::Data
{
  QString                       device;
  SInterfaces::AudioOutput    * output;
  STimer                      * timer;
};

SAudioOutputNode::SAudioOutputNode(SGraph *parent, const QString &device)
  : QObject(parent),
    SInterfaces::SinkNode(parent),
    d(new Data())
{
  d->device = device;
  d->output = NULL;
  d->timer = NULL;
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

void SAudioOutputNode::input(const SAudioBuffer &audioBuffer)
{
  if (d->output)
  {
    if (d->timer)
      d->timer->correctOffset(audioBuffer.timeStamp() + d->output->latency());

    d->output->consume(audioBuffer);
  }
}


} // End of namespace
