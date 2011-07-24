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

#include "nodes/svideogeneratornode.h"
#include "simage.h"

namespace LXiStreamGui {

struct SVideoGeneratorNode::Data
{
  SImage                        image;
  SInterval                     frameRate;
  SVideoBuffer                  videoBuffer;
  STime                         videoTime;
};

SVideoGeneratorNode::SVideoGeneratorNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->frameRate = SInterval::fromFrequency(25);
}

SVideoGeneratorNode::~SVideoGeneratorNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SVideoGeneratorNode::setImage(const SImage &image)
{
  d->image = image;
}

const SImage & SVideoGeneratorNode::image(void) const
{
  return d->image;
}

void SVideoGeneratorNode::setFrameRate(const SInterval &frameRate)
{
  d->frameRate = frameRate;
}

const SInterval & SVideoGeneratorNode::frameRate(void) const
{
  return d->frameRate;
}

bool SVideoGeneratorNode::start(void)
{
  if (!d->image.isNull())
  {
    d->videoBuffer = d->image.toVideoBuffer(SInterval::fromFrequency(25));
    d->videoBuffer.setTimeStamp(STime::null);
  }

  return true;
}

void SVideoGeneratorNode::stop(void)
{
  d->videoBuffer = SVideoBuffer();
}

void SVideoGeneratorNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (!audioBuffer.isNull())
  {
    if (!d->videoTime.isValid() || (qAbs(d->videoTime - audioBuffer.timeStamp()).toSec() >= 3))
      d->videoTime = audioBuffer.timeStamp();

    while (d->videoTime <= audioBuffer.timeStamp())
    {
      if (!d->videoBuffer.isNull())
      {
        d->videoBuffer.setTimeStamp(d->videoTime);
        emit output(d->videoBuffer);
      }

      d->videoTime += d->frameRate;
    }
  }
}

} // End of namespace
