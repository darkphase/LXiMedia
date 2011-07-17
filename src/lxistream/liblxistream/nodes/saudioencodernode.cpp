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

#include "nodes/saudioencodernode.h"
#include "saudiobuffer.h"

namespace LXiStream {

struct SAudioEncoderNode::Data
{
  SInterfaces::AudioEncoder   * encoder;
  QFuture<void>                 future;
};

SAudioEncoderNode::SAudioEncoderNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->encoder = NULL;
}

SAudioEncoderNode::~SAudioEncoderNode()
{
  d->future.waitForFinished();
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioEncoderNode::codecs(void)
{
  return SInterfaces::AudioEncoder::available();
}

QStringList SAudioEncoderNode::losslessCodecs(void)
{
  static const QSet<QString> losslessCodecs = QSet<QString>()
      << "ALAC" << "ALS" << "FLAC" << "SONICLS"
      << "PCM/S16LE" << "PCM/S16BE" << "PCM/U16LE" << "PCM/U16BE" << "PCM/S8"
      << "PCM/U8" << "PCM/MULAW" << "PCM/ALAW" << "PCM/S32LE" << "PCM/S32BE"
      << "PCM/U32LE" << "PCM/U32BE" << "PCM/S24LE" << "PCM/S24BE" << "PCM/U24LE"
      << "PCM/U24BE" << "PCM/S24DAUD" << "PCM/ZORK" << "PCM/S16LEP" << "PCM/DVD"
      << "PCM/F32BE" << "PCM/F32LE" << "PCM/F64BE" << "PCM/F64LE";

  return (QSet<QString>::fromList(codecs()) & losslessCodecs).toList();
}

bool SAudioEncoderNode::openCodec(const SAudioCodec &codec, SInterfaces::AudioEncoder::Flags flags)
{
  d->future.waitForFinished();

  delete d->encoder;
  d->encoder = SInterfaces::AudioEncoder::create(this, codec, flags, false);

  return d->encoder;
}

SAudioCodec SAudioEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SAudioCodec();
}

bool SAudioEncoderNode::start(void)
{
  return d->encoder;
}

void SAudioEncoderNode::stop(void)
{
  d->future.waitForFinished();
}

void SAudioEncoderNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (d->encoder)
    d->future = QtConcurrent::run(this, &SAudioEncoderNode::processTask, audioBuffer);
  else
    emit output(SEncodedAudioBuffer());
}

void SAudioEncoderNode::processTask(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  foreach (const SEncodedAudioBuffer &buffer, d->encoder->encodeBuffer(audioBuffer))
    emit output(buffer);
}

} // End of namespace