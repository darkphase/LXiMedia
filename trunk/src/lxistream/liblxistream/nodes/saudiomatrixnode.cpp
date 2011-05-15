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

// Implemented in saudiomatrixnode.mix.c
extern "C" void LXiStream_SAudioMatrixNode_mixMatrix
 (const qint16 * __restrict srcData, unsigned numSamples, unsigned srcNumChannels,
  qint16 * dstData, const float * appliedMatrix, unsigned dstNumChannels);

namespace LXiStream {

struct SAudioMatrixNode::Data
{
  float                         matrix[32][32];
  SAudioFormat::Channels        channels;

  SAudioFormat                  inFormat;
  float                       * appliedMatrix;

  QFuture<void>                 future;
};

SAudioMatrixNode::SAudioMatrixNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->channels = SAudioFormat::Channels_Stereo;
  d->appliedMatrix = NULL;

  setMatrix(Matrix_Identity);
}

SAudioMatrixNode::~SAudioMatrixNode()
{
  d->future.waitForFinished();
  delete [] d->appliedMatrix;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SAudioMatrixNode::setMatrix(Matrix m)
{
  for (int j=0; j<32; j++)
  for (int i=0; i<32; i++)
    d->matrix[i][j] = 0.0f;

  switch (m)
  {
  case Matrix_Identity:
    for (int i=0; i<32; i++)
      d->matrix[i][i] = 1.0f;

    break;

  case Matrix_SingleToAll:
    for (int j=0; j<32; j++)
    for (int i=0; i<32; i++)
      d->matrix[i][j] = 1.0f;

    break;

  case Matrix_AllToSingle:
    for (int j=0; j<32; j++)
    for (int i=0; i<32; i++)
      d->matrix[i][j] = 0.7f;

    break;

  case Matrix_MonoToStereo:
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_RightFront,           1.0f);
    break;

  case Matrix_StereoToMono:
    setCell(SAudioFormat::Channel_LeftFront,           SAudioFormat::Channel_CenterFront,           0.7f);
    setCell(SAudioFormat::Channel_RightFront,          SAudioFormat::Channel_CenterFront,           0.7f);
    break;

  case Matrix_FrontToBack:
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftBack,             1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterBack,           1.0f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightBack,            1.0f);
    break;

  case Matrix_SurroundToStereo:
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_LeftFront,            0.7f);
    setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftFront,            0.5f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_LeftFront,            0.3f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_RightFront,           0.7f);
    setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightFront,           0.5f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_RightFront,           0.3f);
    break;

  case Matrix_SurroundToSurround_3_0:
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftFront,            0.5f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_CenterFront,          0.3f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightFront,           0.5f);
    break;

  case Matrix_SurroundToSurround_4_0:
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_CenterBack,           0.7f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_CenterBack,           0.7f);
    setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_CenterBack,           0.7f);
    break;

  case Matrix_SurroundToSurround_5_1:
    setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftBack,             1.0f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_LeftBack,             0.7f);
    setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightBack,            1.0f);
    setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_RightBack,            0.7f);
    setCell(SAudioFormat::Channel_LowFrequencyEffects,  SAudioFormat::Channel_LowFrequencyEffects,  1.0f);
    break;
  }
}

void SAudioMatrixNode::setCell(SAudioFormat::Channel from, SAudioFormat::Channel to, float value)
{
  const int fromId = channelId(from), toId = channelId(to);
  if ((fromId >= 0) && (toId >= 0))
    d->matrix[fromId][toId] = value;
}

float SAudioMatrixNode::cell(SAudioFormat::Channel from, SAudioFormat::Channel to) const
{
  const int fromId = channelId(from), toId = channelId(to);
  if ((fromId >= 0) && (toId >= 0))
    return d->matrix[fromId][toId];

  return 0.0f;
}

void SAudioMatrixNode::setChannels(SAudioFormat::Channels channels)
{
  d->channels = channels;
}

SAudioFormat::Channels SAudioMatrixNode::channels(void) const
{
  return d->channels;
}

SAudioMatrixNode::Matrix SAudioMatrixNode::guessMatrix(SAudioFormat::Channels from, SAudioFormat::Channels to)
{
  if (SAudioFormat::numChannels(to) == 1)
  {
    return Matrix_AllToSingle;
  }
  else if (to == SAudioFormat::Channels_Stereo)
  {
    if (SAudioFormat::numChannels(from) < 2)
      return Matrix_SingleToAll;
    else if (SAudioFormat::numChannels(from) > 2)
      return Matrix_SurroundToStereo;
  }
  else if (to == SAudioFormat::Channels_Quadraphonic)
  {
    if (from == SAudioFormat::Channels_Mono)
      return Matrix_SingleToAll;
    else if (from == SAudioFormat::Channels_Stereo)
      return Matrix_FrontToBack;
  }
  else if (to == SAudioFormat::Channels_Surround_3_0)
  {
    if (SAudioFormat::numChannels(from) > 2)
      return Matrix_SurroundToSurround_3_0;
  }
  else if (to == SAudioFormat::Channels_Surround_4_0)
  {
    if (SAudioFormat::numChannels(from) > 2)
      return Matrix_SurroundToSurround_4_0;
  }
  else if ((to == SAudioFormat::Channels_Surround_5_0) || (to == SAudioFormat::Channels_Surround_5_1))
  {
    if (SAudioFormat::numChannels(from) > 2)
      return Matrix_SurroundToSurround_5_1;
  }

  return Matrix_Identity;
}

bool SAudioMatrixNode::start(void)
{
  d->inFormat = SAudioFormat();

  return true;
}

void SAudioMatrixNode::stop(void)
{
  d->future.waitForFinished();
}

void SAudioMatrixNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (!audioBuffer.isNull() && (audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    if (d->inFormat != audioBuffer.format())
    {
      d->inFormat = audioBuffer.format();
      buildMatrix();
    }

    if (d->appliedMatrix)
      d->future = QtConcurrent::run(this, &SAudioMatrixNode::processTask, audioBuffer);
    else
      emit output(audioBuffer);
  }
  else
    emit output(audioBuffer);
}

int SAudioMatrixNode::channelId(SAudioFormat::Channel channel)
{
  for (int i=0; i<32; i++)
  if (quint32(channel) == (quint32(1) << i))
    return i;

  return -1;
}

void SAudioMatrixNode::processTask(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  SAudioFormat outFormat = d->inFormat;
  outFormat.setChannelSetup(d->channels);
  SAudioBuffer destBuffer(outFormat, audioBuffer.numSamples());

  LXiStream_SAudioMatrixNode_mixMatrix(reinterpret_cast<const qint16 *>(audioBuffer.data()),
                                       audioBuffer.numSamples(),
                                       audioBuffer.format().numChannels(),
                                       reinterpret_cast<qint16 *>(destBuffer.data()),
                                       d->appliedMatrix,
                                       outFormat.numChannels());

  destBuffer.setTimeStamp(audioBuffer.timeStamp());

  emit output(destBuffer);
}

void SAudioMatrixNode::buildMatrix(void)
{
  delete [] d->appliedMatrix;
  d->appliedMatrix = NULL;

  const unsigned inChannels = d->inFormat.numChannels();
  const unsigned outChannels = SAudioFormat::numChannels(d->channels);

  if ((inChannels > 0) && (outChannels > 0))
  {
    int inPos[32]; memset(inPos, 0, sizeof(inPos));
    for (int i=0, n=0; i<32; i++)
    if ((d->inFormat.channelSetup() & (quint32(1) << i)) != 0)
      inPos[n++] = i;

    int outPos[32]; memset(outPos, 0, sizeof(outPos));
    for (int i=0, n=0; i<32; i++)
    if ((d->channels & (quint32(1) << i)) != 0)
      outPos[n++] = i;

    d->appliedMatrix = new float[inChannels * outChannels];

    bool identityMatrix = outChannels == inChannels;
    for (unsigned j=0; j<outChannels; j++)
    {
      float * const line = d->appliedMatrix + (j * inChannels);
      for (unsigned i=0; i<inChannels; i++)
      {
        const int ii = inPos[i], jj = outPos[j];

        line[i] = d->matrix[ii][jj];
        identityMatrix &= qFuzzyCompare(line[i], ii == jj ? 1.0f : 0.0f);
      }
    }

    if (identityMatrix)
    {
      delete [] d->appliedMatrix;
      d->appliedMatrix = NULL;
    }
  }
}

} // End of namespace
