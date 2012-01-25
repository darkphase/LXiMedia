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

#ifndef LXISTREAM_SAUDIOMATRIXNODE_H
#define LXISTREAM_SAUDIOMATRIXNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {


/*! This audio filter can be used to apply a surround matrix to an audio stream.
    This surround matrix can be used to map a stereo audio stream to a surrond
    setup by, dor example duplicating the left and right channels to the front
    and rear channels. It can also mix the left and right channels into one
    channel for the center speaker and the subwoofer.
 */
class LXISTREAM_PUBLIC SAudioMatrixNode : public SInterfaces::Node
{
Q_OBJECT
public:
  class LXISTREAM_PUBLIC Matrix
  {
  friend class SAudioMatrixNode;
  public:
                                Matrix(void);
                                ~Matrix(void);

    void                        setChannels(SAudioFormat::Channels);
    SAudioFormat::Channels      channels(void) const;

    void                        setCell(SAudioFormat::Channel from, SAudioFormat::Channel to, float);
    float                       cell(SAudioFormat::Channel from, SAudioFormat::Channel to) const;

  private:
    struct
    {
      float                     matrix[32][32];
      SAudioFormat::Channels    channels;
    }                           d;
  };

  enum MatrixMode
  {
    Matrix_Identity = 0,
    Matrix_SingleToAll,
    Matrix_AllToSingle,
    Matrix_MonoToStereo,
    Matrix_StereoToMono,
    Matrix_FrontToBack,
    Matrix_SurroundToStereo,
    Matrix_SurroundToSurround_3_0,
    Matrix_SurroundToSurround_4_0,
    Matrix_SurroundToSurround_5_1
  };

public:
                                SAudioMatrixNode(SGraph *);
  virtual                       ~SAudioMatrixNode();

  void                          setMatrix(SAudioFormat::Channels from, Matrix);
  void                          setMatrix(SAudioFormat::Channels from, MatrixMode, SAudioFormat::Channels to);
  void                          guessMatrices(SAudioFormat::Channels to);
  const Matrix                & matrix(SAudioFormat::Channels from) const;

  static MatrixMode             guessMatrix(SAudioFormat::Channels from, SAudioFormat::Channels to);

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);

signals:
  void                          output(const SAudioBuffer &);

private:
  static int                    channelId(SAudioFormat::Channel);
  void                          buildMatrix(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
