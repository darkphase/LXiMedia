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

#include "nodes/sioinputnode.h"

namespace LXiStream {

struct SIOInputNode::Data
{
  QIODevice                   * ioDevice;
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

  if (ioDevice && ioDevice->isOpen())
  {
    QString filePath;
    QFile * const file = qobject_cast<QFile *>(ioDevice);
    if (file)
      filePath = file->fileName();

    const QByteArray buffer = ioDevice->peek(SInterfaces::FormatProber::defaultProbeSize);

    QMultiMap<int, QString> formats;
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
    {
      foreach (const SInterfaces::FormatProber::Format &format, prober->probeFormat(buffer, filePath))
        formats.insert(-format.confidence, format.name);

      delete prober;
    }

    // Now try to open a parser.
    foreach (const QString &format, formats)
    {
      SInterfaces::BufferReader * const bufferReader = SInterfaces::BufferReader::create(this, format, false);
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
  if (d->ioDevice)
    return SInputNode::start();

  return false;
}

void SIOInputNode::stop(void)
{
  SInterfaces::BufferReader * const bufferReader = static_cast<SInterfaces::BufferReader *>(SInputNode::bufferReader());
  if (bufferReader)
    bufferReader->stop();

  SInputNode::stop();
}

bool SIOInputNode::process(void)
{
  if (d->ioDevice)
  {
    if (!d->ioDevice->atEnd())
      return SInputNode::process();

    endReached();

    return true;
  }

  return false;
}

void SIOInputNode::endReached(void)
{
  while (SInputNode::process())
    continue;

  stop();

  emit output(SEncodedAudioBuffer());
  emit output(SEncodedVideoBuffer());
  emit output(SEncodedDataBuffer());
  emit finished();
}

} // End of namespace
