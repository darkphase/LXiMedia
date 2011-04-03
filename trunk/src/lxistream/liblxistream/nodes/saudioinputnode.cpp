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

#include "nodes/saudioinputnode.h"
#include "sinterfaces.h"

namespace LXiStream {


struct SAudioInputNode::Data
{
  QString                       device;
  SInterfaces::AudioInput     * input;
};

SAudioInputNode::SAudioInputNode(SGraph *parent, const QString &device)
  : QObject(parent),
    SGraph::SourceNode(parent),
    d(new Data())
{
  d->device = device;
  d->input = NULL;
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

bool SAudioInputNode::start(void)
{
  delete d->input;
  d->input = SInterfaces::AudioInput::create(this, d->device);

  if (d->input)
  if (d->input->start())
  {
    connect(d->input, SIGNAL(produce(const SAudioBuffer &)), SIGNAL(output(const SAudioBuffer &)));
    return true;
  }

  delete d->input;
  d->input = NULL;

  qWarning() << "Failed to open audio input device" << d->device;
  return false;
}

void SAudioInputNode::stop(void)
{
  if (d->input)
  {
    d->input->stop();

    delete d->input;
    d->input = NULL;
  }
}

void SAudioInputNode::process(void)
{
  if (d->input)
    d->input->process();
}


} // End of namespace
