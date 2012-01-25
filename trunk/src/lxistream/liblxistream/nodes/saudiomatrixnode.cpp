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

#include "nodes/saudiomatrixnode.h"

// Implemented in saudiomatrixnode.mix.c
extern "C" void LXiStream_SAudioMatrixNode_mixMatrix
 (const qint16 * srcData, unsigned numSamples, unsigned srcNumChannels,
  qint16 * dstData, const int * appliedMatrix, unsigned dstNumChannels);

namespace LXiStream {

struct SAudioMatrixNode::Data
{
  QMap<SAudioFormat::Channels, Matrix> matrix;

  SAudioFormat                  inFormat;
  SAudioFormat                  outFormat;
  int                         * appliedMatrix;
};

SAudioMatrixNode::SAudioMatrixNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->appliedMatrix = NULL;
}

SAudioMatrixNode::~SAudioMatrixNode()
{
  delete [] d->appliedMatrix;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SAudioMatrixNode::setMatrix(SAudioFormat::Channels from, Matrix matrix)
{
  d->matrix[from] = matrix;
}

void SAudioMatrixNode::setMatrix(SAudioFormat::Channels from, MatrixMode m, SAudioFormat::Channels to)
{
  QMap<SAudioFormat::Channels, Matrix>::Iterator matrix = d->matrix.find(from);
  if (matrix == d->matrix.end())
    matrix = d->matrix.insert(from, Matrix());

  for (int j=0; j<32; j++)
  for (int i=0; i<32; i++)
    matrix->d.matrix[i][j] = 0.0f;

  matrix->setChannels(to);

  switch (m)
  {
  case Matrix_Identity:
    for (int i=0; i<32; i++)
      matrix->d.matrix[i][i] = 1.0f;

    break;

  case Matrix_SingleToAll:
    for (int j=0; j<32; j++)
    for (int i=0; i<32; i++)
      matrix->d.matrix[i][j] = 1.0f;

    break;

  case Matrix_AllToSingle:
    for (int j=0; j<32; j++)
    for (int i=0; i<32; i++)
      matrix->d.matrix[i][j] = 0.7f;

    break;

  case Matrix_MonoToStereo:
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_RightFront,           1.0f);
    break;

  case Matrix_StereoToMono:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_CenterFront,          0.7f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_CenterFront,          0.7f);
    break;

  case Matrix_FrontToBack:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftBack,             1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterBack,           1.0f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightBack,            1.0f);
    break;

  case Matrix_SurroundToStereo:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_LeftFront,            0.7f);
    matrix->setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftFront,            0.5f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_LeftFront,            0.3f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_RightFront,           0.7f);
    matrix->setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightFront,           0.5f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_RightFront,           0.3f);
    break;

  case Matrix_SurroundToSurround_3_0:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftFront,            0.5f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_CenterFront,          0.3f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    matrix->setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightFront,           0.5f);
    break;

  case Matrix_SurroundToSurround_4_0:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    matrix->setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_CenterBack,           0.7f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_CenterBack,           0.7f);
    matrix->setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_CenterBack,           0.7f);
    break;

  case Matrix_SurroundToSurround_5_1:
    matrix->setCell(SAudioFormat::Channel_LeftFront,            SAudioFormat::Channel_LeftFront,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterFront,          SAudioFormat::Channel_CenterFront,          1.0f);
    matrix->setCell(SAudioFormat::Channel_RightFront,           SAudioFormat::Channel_RightFront,           1.0f);
    matrix->setCell(SAudioFormat::Channel_LeftBack,             SAudioFormat::Channel_LeftBack,             1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_LeftBack,             0.7f);
    matrix->setCell(SAudioFormat::Channel_RightBack,            SAudioFormat::Channel_RightBack,            1.0f);
    matrix->setCell(SAudioFormat::Channel_CenterBack,           SAudioFormat::Channel_RightBack,            0.7f);
    matrix->setCell(SAudioFormat::Channel_LowFrequencyEffects,  SAudioFormat::Channel_LowFrequencyEffects,  1.0f);
    break;
  }
}

void SAudioMatrixNode::guessMatrices(SAudioFormat::Channels to)
{
  setMatrix(SAudioFormat::Channels_Mono,              guessMatrix(SAudioFormat::Channels_Mono,              to), to);
  setMatrix(SAudioFormat::Channels_Stereo,            guessMatrix(SAudioFormat::Channels_Stereo,            to), to);
  setMatrix(SAudioFormat::Channels_Quadraphonic,      guessMatrix(SAudioFormat::Channels_Quadraphonic,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_3_0,      guessMatrix(SAudioFormat::Channels_Surround_3_0,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_4_0,      guessMatrix(SAudioFormat::Channels_Surround_4_0,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_5_0,      guessMatrix(SAudioFormat::Channels_Surround_5_0,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_5_1,      guessMatrix(SAudioFormat::Channels_Surround_5_1,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_6_0,      guessMatrix(SAudioFormat::Channels_Surround_6_0,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_6_1,      guessMatrix(SAudioFormat::Channels_Surround_6_1,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_7_1,      guessMatrix(SAudioFormat::Channels_Surround_7_1,      to), to);
  setMatrix(SAudioFormat::Channels_Surround_7_1_Wide, guessMatrix(SAudioFormat::Channels_Surround_7_1_Wide, to), to);
}

const SAudioMatrixNode::Matrix & SAudioMatrixNode::matrix(SAudioFormat::Channels from) const
{
  return d->matrix[from];
}

SAudioMatrixNode::MatrixMode SAudioMatrixNode::guessMatrix(SAudioFormat::Channels from, SAudioFormat::Channels to)
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
  d->outFormat = SAudioFormat();

  return true;
}

void SAudioMatrixNode::stop(void)
{
}

void SAudioMatrixNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && (audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    if (d->inFormat != audioBuffer.format())
    {
      d->inFormat = audioBuffer.format();
      buildMatrix();
    }

    if (d->appliedMatrix)
    {
      SAudioBuffer destBuffer(d->outFormat, audioBuffer.numSamples());

      LXiStream_SAudioMatrixNode_mixMatrix(reinterpret_cast<const qint16 *>(audioBuffer.data()),
                                           audioBuffer.numSamples(),
                                           audioBuffer.format().numChannels(),
                                           reinterpret_cast<qint16 *>(destBuffer.data()),
                                           d->appliedMatrix,
                                           d->outFormat.numChannels());

      destBuffer.setTimeStamp(audioBuffer.timeStamp());

      emit output(destBuffer);
    }
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

void SAudioMatrixNode::buildMatrix(void)
{
  delete [] d->appliedMatrix;
  d->appliedMatrix = NULL;

  QMap<SAudioFormat::Channels, Matrix>::ConstIterator matrix = d->matrix.find(d->inFormat.channelSetup());
  if (matrix != d->matrix.end())
  {
    d->outFormat = d->inFormat;
    d->outFormat.setChannelSetup(matrix->d.channels);

    const unsigned inChannels = d->inFormat.numChannels();
    const unsigned outChannels = d->outFormat.numChannels();

    if ((inChannels > 0) && (outChannels > 0))
    {
      int inPos[32]; memset(inPos, 0, sizeof(inPos));
      for (int i=0, n=0; i<32; i++)
      if ((d->inFormat.channelSetup() & (quint32(1) << i)) != 0)
        inPos[n++] = i;

      int outPos[32]; memset(outPos, 0, sizeof(outPos));
      for (int i=0, n=0; i<32; i++)
      if ((matrix->d.channels & (quint32(1) << i)) != 0)
        outPos[n++] = i;

      d->appliedMatrix = new int[inChannels * outChannels];

      bool identityMatrix = outChannels == inChannels;
      for (unsigned j=0; j<outChannels; j++)
      {
        int * const line = d->appliedMatrix + (j * inChannels);
        for (unsigned i=0; i<inChannels; i++)
        {
          const int ii = inPos[i], jj = outPos[j];

          line[i] = int((matrix->d.matrix[ii][jj] * 256.0f) + 0.5f);
          identityMatrix &= line[i] == (ii == jj ? 256 : 0);
        }
      }

      if (identityMatrix)
      {
        delete [] d->appliedMatrix;
        d->appliedMatrix = NULL;
      }
    }
  }
}


SAudioMatrixNode::Matrix::Matrix(void)
{
  d.channels = SAudioFormat::Channels_Stereo;

  for (int j=0; j<32; j++)
  for (int i=0; i<32; i++)
    d.matrix[j][i] = 0.0f;
}

SAudioMatrixNode::Matrix::~Matrix(void)
{
}

void SAudioMatrixNode::Matrix::setChannels(SAudioFormat::Channels channels)
{
  d.channels = channels;
}

SAudioFormat::Channels SAudioMatrixNode::Matrix::channels(void) const
{
  return d.channels;
}

void SAudioMatrixNode::Matrix::setCell(SAudioFormat::Channel from, SAudioFormat::Channel to, float value)
{
  const int fromId = channelId(from), toId = channelId(to);
  if ((fromId >= 0) && (toId >= 0))
    d.matrix[fromId][toId] = value;
}

float SAudioMatrixNode::Matrix::cell(SAudioFormat::Channel from, SAudioFormat::Channel to) const
{
  const int fromId = channelId(from), toId = channelId(to);
  if ((fromId >= 0) && (toId >= 0))
    return d.matrix[fromId][toId];

  return 0.0f;
}

} // End of namespace
