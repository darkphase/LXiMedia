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

#include "nodes/saudiogapremovernode.h"
#include "../../algorithms/audioprocess.h"

namespace LXiStream {

struct SAudioGapRemoverNode::Data
{
  static const int              delay = 3000;
  QQueue<SAudioBuffer>          audioDelay;
};

SAudioGapRemoverNode::SAudioGapRemoverNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
}

SAudioGapRemoverNode::~SAudioGapRemoverNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SAudioGapRemoverNode::start(void)
{
  return true;
}

void SAudioGapRemoverNode::stop(void)
{
  d->audioDelay.clear();
}

void SAudioGapRemoverNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  if (!d->audioDelay.isEmpty() &&
      (qAbs(audioBuffer.timeStamp() - d->audioDelay.last().timeStamp()).toMSec() > (d->delay * 2)))
  { // New stream (timestamp gap).
    if (Algorithms::AudioProcess::avg(
            reinterpret_cast<const qint16 *>(audioBuffer.data()),
            audioBuffer.numSamples() * audioBuffer.format().numChannels()) >= 512)
    {
      while (!d->audioDelay.isEmpty() &&
             (Algorithms::AudioProcess::avg(
                  reinterpret_cast<const qint16 *>(d->audioDelay.last().data()),
                  d->audioDelay.last().numSamples() * d->audioDelay.last().format().numChannels()) < 512))
      {
        d->audioDelay.takeLast();
      }

      while (!d->audioDelay.isEmpty())
        emit output(d->audioDelay.dequeue());

      d->audioDelay.enqueue(audioBuffer);
    }
  }
  else
  {
    d->audioDelay.enqueue(audioBuffer);

    while (!d->audioDelay.isEmpty() &&
           ((d->audioDelay.last().timeStamp() - d->audioDelay.first().timeStamp()).toMSec() > d->delay))
    {
      emit output(d->audioDelay.dequeue());
    }
  }
}

} // End of namespace
