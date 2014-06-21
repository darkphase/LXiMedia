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

#include "nodes/svideodeinterlacenode.h"
#include "sinterfaces.h"

namespace LXiStream {

struct SVideoDeinterlaceNode::Data
{
  SInterfaces::VideoDeinterlacer * deinterlacer;
};

SVideoDeinterlaceNode::SVideoDeinterlaceNode(SGraph *parent, const QString &algo)
  : SInterfaces::Node(parent),
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

bool SVideoDeinterlaceNode::start(void)
{
  return true;
}

void SVideoDeinterlaceNode::stop(void)
{
}

void SVideoDeinterlaceNode::input(const SVideoBuffer &videoBuffer)
{
  if (d->deinterlacer)
  {
    const SSize size = videoBuffer.format().size();
    const SVideoFormat::FieldMode fieldMode = videoBuffer.format().fieldMode();
    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst) ||
        ((size.width() >= 1920) || (size.height() >= 1080)))
    {
      foreach (const SVideoBuffer &buffer, d->deinterlacer->processBuffer(videoBuffer))
        emit output(buffer);
    }
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

} // End of namespace
