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

#include "nodes/sfileoutputnode.h"
#include "smediafilesystem.h"

namespace LXiStream {


struct SFileOutputNode::Data
{
  QIODevice                   * ioDevice;
  QUrl                          filePath;
};

SFileOutputNode::SFileOutputNode(SGraph *parent, const QUrl &filePath)
  : SIOOutputNode(parent),
    d(new Data())
{
  d->ioDevice = NULL;

  setFilePath(filePath);
}

SFileOutputNode::~SFileOutputNode()
{
  SIOOutputNode::setIODevice(NULL);
  delete d->ioDevice;

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SFileOutputNode::setFilePath(const QUrl &filePath)
{
  SIOOutputNode::setIODevice(NULL);
  delete d->ioDevice;

  d->filePath = filePath;
  d->ioDevice = SMediaFilesystem::open(filePath, QIODevice::WriteOnly);

  if (d->ioDevice)
    SIOOutputNode::setIODevice(d->ioDevice);
}

QUrl SFileOutputNode::filePath(void) const
{
  return d->filePath;
}


} // End of namespace
