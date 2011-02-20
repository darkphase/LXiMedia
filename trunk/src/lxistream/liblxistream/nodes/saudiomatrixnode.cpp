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

#include "nodes/saudiomatrixnode.h"
#include "sgraph.h"

// Implemented in channelmatrix.mix.c
extern "C" void LXiStream_SAudioMatrixNode_mixMatrix(const qint16 *, unsigned, unsigned, unsigned, qint16 *, const float *, unsigned);

namespace LXiStream {

struct SAudioMatrixNode::Data
{
  SDependency                 * dependency;
  QList<qreal>                  requestedMatrix;
  float                       * appliedMatrix;
  SAudioFormat::Channels        inChannelSetup;
  unsigned                      inNumChannels;
  SAudioFormat::Channels        outChannelSetup;
  unsigned                      outNumChannels;
};

SAudioMatrixNode::SAudioMatrixNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;
  d->appliedMatrix = NULL;
  d->inChannelSetup = SAudioFormat::Channel_Stereo;
  d->inNumChannels = SAudioFormat::numChannels(d->inChannelSetup);
  d->outChannelSetup = SAudioFormat::Channel_Stereo;
  d->outNumChannels = SAudioFormat::numChannels(d->outChannelSetup);
}

SAudioMatrixNode::~SAudioMatrixNode()
{
  delete d->dependency;
  delete [] d->appliedMatrix;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

quint32 SAudioMatrixNode::inChannels(void) const
{
  return quint32(d->inChannelSetup);
}

void SAudioMatrixNode::setInChannels(quint32 c)
{
  d->inChannelSetup = SAudioFormat::Channels(c);
  d->inNumChannels = SAudioFormat::numChannels(d->inChannelSetup);
}

quint32 SAudioMatrixNode::outChannels(void) const
{
  return quint32(d->outChannelSetup);
}

void SAudioMatrixNode::setOutChannels(quint32 c)
{
  d->outChannelSetup = SAudioFormat::Channels(c);
  d->outNumChannels = SAudioFormat::numChannels(d->outChannelSetup);
}

const QList<qreal> & SAudioMatrixNode::matrix(void) const
{
  return d->requestedMatrix;
}

void SAudioMatrixNode::setMatrix(const QList<qreal> &m)
{
  d->requestedMatrix = m;

  if ((d->requestedMatrix.count() == int(d->inNumChannels * d->outNumChannels)) &&
      (d->appliedMatrix != NULL))
  {
    for (unsigned i=0; i<d->outNumChannels; i++)
    for (unsigned j=0; j<d->inNumChannels; j++)
      d->appliedMatrix[i * d->inNumChannels + j] = float(d->requestedMatrix[i * d->inNumChannels + j]);
  }
}

void SAudioMatrixNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() &&
      (audioBuffer.format() == SAudioFormat::Format_PCM_S16) &&
      (d->inNumChannels * d->outNumChannels > 0))
  {
    if (d->appliedMatrix == NULL)
    {
      if (d->inNumChannels * d->outNumChannels > 0)
      {
        d->appliedMatrix = new float[d->inNumChannels * d->outNumChannels];

        if (d->requestedMatrix.count() == int(d->inNumChannels * d->outNumChannels))
        {
          for (unsigned i=0; i<d->outNumChannels; i++)
          for (unsigned j=0; j<d->inNumChannels; j++)
            d->appliedMatrix[i * d->inNumChannels + j] = float(d->requestedMatrix[i * d->inNumChannels + j]);
        }
        else
        {
          for (unsigned i=0; i<d->outNumChannels; i++)
          for (unsigned j=0; j<d->inNumChannels; j++)
            d->appliedMatrix[i * d->inNumChannels + j] = (i == j) ? 1.0f : 0.0f;
        }
      }
    }

    if (graph)
      graph->queue(this, &SAudioMatrixNode::processTask, audioBuffer, d->dependency);
    else
      processTask(audioBuffer);
  }
  else
    emit output(audioBuffer);
}

void SAudioMatrixNode::processTask(const SAudioBuffer &audioBuffer)
{
  SAudioBuffer destBuffer(audioBuffer.format(), audioBuffer.numSamples());

  LXiStream_SAudioMatrixNode_mixMatrix(reinterpret_cast<const qint16 *>(audioBuffer.data()),
                                       audioBuffer.numSamples(),
                                       audioBuffer.format().numChannels(),
                                       d->inNumChannels,
                                       reinterpret_cast<qint16 *>(destBuffer.data()),
                                       d->appliedMatrix,
                                       d->outNumChannels);

  destBuffer.setTimeStamp(audioBuffer.timeStamp());

  emit output(destBuffer);
}


} // End of namespace
