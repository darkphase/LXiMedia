/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "nodes/sioinputnode.h"

namespace LXiStream {

struct SIOInputNode::Data
{
  QIODevice                   * ioDevice;
  bool                          endReached;
};

SIOInputNode::SIOInputNode(SGraph *parent, QIODevice *ioDevice)
  : SInputNode(parent),
    d(new Data())
{
  setIODevice(ioDevice);
}

SIOInputNode::~SIOInputNode()
{
  delete bufferReader();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SIOInputNode::setIODevice(QIODevice *ioDevice)
{
  if (bufferReader())
  {
    delete bufferReader();
    setBufferReader(NULL);
  }

  d->ioDevice = ioDevice;
  d->endReached = false;

  if (ioDevice && ioDevice->isOpen())
  {
    QUrl filePath;
    QFile * const file = qobject_cast<QFile *>(ioDevice);
    if (file)
    {
      filePath.setScheme("file");
      filePath.setPath(file->fileName());
    }

    const QByteArray buffer = ioDevice->peek(SInterfaces::FormatProber::defaultProbeSize);

    SInterfaces::FormatProber::ProbeInfo pi;
    pi.filePath = filePath;
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
    {
      prober->readFormat(pi, buffer);
      delete prober;
    }

    if (!pi.format.format.isEmpty())
    {
      SInterfaces::BufferReader * const bufferReader =
          SInterfaces::BufferReader::create(this, pi.format.format, false);

      if (bufferReader)
      {
        if (bufferReader->start(d->ioDevice, this, ioDevice->isSequential()))
        {
          setBufferReader(bufferReader);
          return;
        }

        delete bufferReader;
      }
    }
  }

  d->ioDevice = NULL;
}

const QIODevice * SIOInputNode::ioDevice(void) const
{
  return d->ioDevice;
}

QIODevice * SIOInputNode::ioDevice(void)
{
  return d->ioDevice;
}

bool SIOInputNode::start(void)
{
  if (d->ioDevice && !d->endReached)
    return SInputNode::start();

  return false;
}

void SIOInputNode::stop(void)
{
  SInputNode::stop();

  SInterfaces::BufferReader * const bufferReader = static_cast<SInterfaces::BufferReader *>(SInputNode::bufferReader());
  if (bufferReader)
    bufferReader->stop();
}

bool SIOInputNode::process(void)
{
  if (d->ioDevice && !d->endReached)
  {
    if (SInputNode::process())
      return true;

    endReached();

    return true;
  }

  return false;
}

void SIOInputNode::endReached(void)
{
  while (SInputNode::process())
    continue;

  emit output(SEncodedAudioBuffer());
  emit output(SEncodedVideoBuffer());
  emit output(SEncodedDataBuffer());
  emit finished();

  d->endReached = true;
}

} // End of namespace
