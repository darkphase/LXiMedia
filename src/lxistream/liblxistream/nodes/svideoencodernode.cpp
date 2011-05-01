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

#include "nodes/svideoencodernode.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoEncoderNode::Data
{
  SInterfaces::VideoEncoder   * encoder;
  QFuture<void>                 future;
};

SVideoEncoderNode::SVideoEncoderNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
}

SVideoEncoderNode::~SVideoEncoderNode()
{
  d->future.waitForFinished();
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoEncoderNode::codecs(void)
{
  return SInterfaces::VideoEncoder::available();
}

QStringList SVideoEncoderNode::losslessCodecs(void)
{
  static const QSet<QString> losslessCodecs = QSet<QString>()
      << "LJPEG" << "MLP" << "MSZH" << "ZLIB";

  return (QSet<QString>::fromList(codecs()) & losslessCodecs).toList();
}

bool SVideoEncoderNode::openCodec(const SVideoCodec &codec, SInterfaces::VideoEncoder::Flags flags)
{
  d->future.waitForFinished();

  delete d->encoder;
  d->encoder = SInterfaces::VideoEncoder::create(this, codec, flags, false);

  return d->encoder;
}

SVideoCodec SVideoEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SVideoCodec();
}

bool SVideoEncoderNode::start(void)
{
  return d->encoder;
}

void SVideoEncoderNode::stop(void)
{
  d->future.waitForFinished();
}

void SVideoEncoderNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (d->encoder)
    d->future = QtConcurrent::run(this, &SVideoEncoderNode::processTask, videoBuffer);
  else
    emit output(SEncodedVideoBuffer());
}

void SVideoEncoderNode::processTask(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  foreach (const SEncodedVideoBuffer &buffer, d->encoder->encodeBuffer(videoBuffer))
    emit output(buffer);
}


} // End of namespace
