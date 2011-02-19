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

#include "nodes/svideodeinterlacenode.h"
#include "sgraph.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoDeinterlaceNode::Data
{
  SInterfaces::VideoDeinterlacer * deinterlacer;
};

SVideoDeinterlaceNode::SVideoDeinterlaceNode(SGraph *parent, const QString &algo)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->deinterlacer = SInterfaces::VideoDeinterlacer::create(this, algo);
}

SVideoDeinterlaceNode::~SVideoDeinterlaceNode()
{
  delete d->deinterlacer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoDeinterlaceNode::algorithms(void)
{
  return SInterfaces::VideoDeinterlacer::available();
}

void SVideoDeinterlaceNode::input(const SVideoBuffer &videoBuffer)
{
  if (d->deinterlacer)
  {
    const SVideoFormat::FieldMode fieldMode = videoBuffer.format().fieldMode();
    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst))
    {
      if (graph)
        graph->runTask(this, &SVideoDeinterlaceNode::processTask, videoBuffer);
      else
        processTask(videoBuffer);
    }
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

void SVideoDeinterlaceNode::processTask(const SVideoBuffer &videoBuffer)
{
  foreach (const SVideoBuffer &buffer, d->deinterlacer->processBuffer(videoBuffer))
    emit output(buffer);
}

} // End of namespace
