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

#include "nodes/sfileoutputnode.h"

namespace LXiStream {


struct SFileOutputNode::Data
{
  inline Data(const QString &fileName) : mediaFile(fileName) { }

  QFile                         mediaFile;
};

SFileOutputNode::SFileOutputNode(SGraph *parent, const QString &fileName)
  : SIOOutputNode(parent),
    d(new Data(fileName))
{
  SIOOutputNode::setIODevice(&d->mediaFile);
}

SFileOutputNode::~SFileOutputNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SFileOutputNode::start(STimer *timer)
{
  if (d->mediaFile.open(QIODevice::WriteOnly))
    return SIOOutputNode::start(timer);

  return false;
}

void SFileOutputNode::stop(void)
{
  SIOOutputNode::stop();

  d->mediaFile.close();
}


} // End of namespace
