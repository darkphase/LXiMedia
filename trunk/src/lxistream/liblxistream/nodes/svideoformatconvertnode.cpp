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

#include "nodes/svideoformatconvertnode.h"
#include "sinterfaces.h"

namespace LXiStream {

struct SVideoFormatConvertNode::Data
{
  SScheduler::Dependency      * dependency;
  SVideoFormat::Format          sourceFormat;
  SVideoFormat::Format          destFormat;
  SInterfaces::VideoFormatConverter * converter;
};

SVideoFormatConvertNode::SVideoFormatConvertNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SScheduler::Dependency(parent) : NULL;
  d->sourceFormat = SVideoFormat::Format_Invalid;
  d->destFormat = SVideoFormat::Format_Invalid;
  d->converter = NULL;
}

SVideoFormatConvertNode::~SVideoFormatConvertNode()
{
  delete d->dependency;
  delete d->converter;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SVideoFormatConvertNode::setDestFormat(SVideoFormat::Format destFormat)
{
  d->destFormat = destFormat;
  delete d->converter;
  d->converter = NULL;
}

SVideoFormat::Format SVideoFormatConvertNode::destFormat(void) const
{
  return d->destFormat;
}

/*! Converts the video buffer to the format specified by setDestFormat().
 */
SVideoBuffer SVideoFormatConvertNode::convert(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    if (videoBuffer.format() != d->sourceFormat)
    {
      d->sourceFormat = videoBuffer.format();
      delete d->converter;
      d->converter = NULL;
    }

    if (d->sourceFormat == d->destFormat)
    {
      return videoBuffer;
    }
    else if ((d->converter == NULL) &&
             (d->sourceFormat != SVideoFormat::Format_Invalid) &&
             (d->destFormat != SVideoFormat::Format_Invalid))
    {
      d->converter = SInterfaces::VideoFormatConverter::create(NULL, d->sourceFormat, d->destFormat, false);
    }

    if (d->converter)
      return d->converter->convertBuffer(videoBuffer);
  }

  return SVideoBuffer();
}

/*! Converts the video buffer to the format specified by destFormat. Note that
    this static method will create a new SInterfaces::VideoFormatConverter each
    time it is invoked. Use a seperate SVideoFormatConvertNode object to convert
    multiple buffers to/from the same format.
 */
SVideoBuffer SVideoFormatConvertNode::convert(const SVideoBuffer &videoBuffer, SVideoFormat::Format destFormat)
{
  if (!videoBuffer.isNull())
  {
    const SVideoFormat::Format sourceFormat = videoBuffer.format();
    if (sourceFormat == destFormat)
    {
      return videoBuffer;
    }
    else if ((sourceFormat != SVideoFormat::Format_Invalid) &&
             (destFormat != SVideoFormat::Format_Invalid))
    {
      SInterfaces::VideoFormatConverter * const converter =
          SInterfaces::VideoFormatConverter::create(NULL, sourceFormat, destFormat, false);

      SVideoBuffer result = converter ? converter->convertBuffer(videoBuffer) : SVideoBuffer();

      delete converter;

      return result;
    }
  }

  return SVideoBuffer();
}

void SVideoFormatConvertNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    if (videoBuffer.format() == d->destFormat)
      emit output(videoBuffer);
    else
      schedule(&SVideoFormatConvertNode::processTask, videoBuffer, d->dependency);
  }
}

void SVideoFormatConvertNode::processTask(const SVideoBuffer &videoBuffer)
{
  emit output(convert(videoBuffer));
}

} // End of namespace
