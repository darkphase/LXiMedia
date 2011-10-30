/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
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

#include "nodes/saudioformatconvertnode.h"
#include "sinterfaces.h"

namespace LXiStream {

struct SAudioFormatConvertNode::Data
{
  SAudioFormat::Format          sourceFormat;
  SAudioFormat::Format          destFormat;
  SInterfaces::AudioFormatConverter * converter;
};

SAudioFormatConvertNode::SAudioFormatConvertNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->sourceFormat = SAudioFormat::Format_Invalid;
  d->destFormat = SAudioFormat::Format_Invalid;
  d->converter = NULL;
}

SAudioFormatConvertNode::~SAudioFormatConvertNode()
{
  delete d->converter;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SAudioFormatConvertNode::setDestFormat(SAudioFormat::Format destFormat)
{
  d->destFormat = destFormat;
  delete d->converter;
  d->converter = NULL;
}

SAudioFormat::Format SAudioFormatConvertNode::destFormat(void) const
{
  return d->destFormat;
}

/*! Converts the video buffer to the format specified by setDestFormat().
 */
SAudioBuffer SAudioFormatConvertNode::convert(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  {
    if (audioBuffer.format() != d->sourceFormat)
    {
      d->sourceFormat = audioBuffer.format();
      delete d->converter;
      d->converter = NULL;
    }

    if (d->sourceFormat == d->destFormat)
    {
      return audioBuffer;
    }
    else if ((d->converter == NULL) &&
             (d->sourceFormat != SAudioFormat::Format_Invalid) &&
             (d->destFormat != SAudioFormat::Format_Invalid))
    {
      d->converter = SInterfaces::AudioFormatConverter::create(NULL, d->sourceFormat, d->destFormat, false);
    }

    if (d->converter)
      return d->converter->convertBuffer(audioBuffer);
  }

  return SAudioBuffer();
}

/*! Converts the audio buffer to the format specified by destFormat. Note that
    this static method will create a new SInterfaces::AudioFormatConverter each
    time it is invoked. Use a seperate SAudioFormatConvertNode object to convert
    multiple buffers to/from the same format.
 */
SAudioBuffer SAudioFormatConvertNode::convert(const SAudioBuffer &audioBuffer, SAudioFormat::Format destFormat)
{
  if (!audioBuffer.isNull())
  {
    const SAudioFormat::Format sourceFormat = audioBuffer.format();
    if (sourceFormat == destFormat)
    {
      return audioBuffer;
    }
    else if ((sourceFormat != SAudioFormat::Format_Invalid) &&
             (destFormat != SAudioFormat::Format_Invalid))
    {
      SInterfaces::AudioFormatConverter * const converter =
          SInterfaces::AudioFormatConverter::create(NULL, sourceFormat, destFormat, false);

      SAudioBuffer result = converter ? converter->convertBuffer(audioBuffer) : SAudioBuffer();

      delete converter;

      return result;
    }
  }

  return SAudioBuffer();
}

bool SAudioFormatConvertNode::start(void)
{
  return true;
}

void SAudioFormatConvertNode::stop(void)
{
}

void SAudioFormatConvertNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  {
    if (audioBuffer.format() == d->destFormat)
      emit output(audioBuffer);
    else
      emit output(convert(audioBuffer));
  }
  else
    emit output(audioBuffer);
}

} // End of namespace
