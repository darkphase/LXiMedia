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

#ifndef LXISTREAM_SNETWORKINPUTNODE_H
#define LXISTREAM_SNETWORKINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "sinputnode.h"
#include "../export.h"

namespace LXiStream {

class SEncodedVideoBuffer;

class LXISTREAM_PUBLIC SNetworkInputNode : public SInputNode
{
Q_OBJECT
public:
  explicit                      SNetworkInputNode(SGraph *, const QUrl & = QUrl());
  virtual                       ~SNetworkInputNode();

  void                          setUrl(const QUrl &);
  QUrl                          url(void) const;

  void                          setBufferDuration(const STime &);
  STime                         bufferDuration(void) const;

  /*! Fills the buffer by receiving data.
      \returns true if the buffer is properly filled, false if another call to\
               fillBuffer() is needed to receive more data.
   */
  bool                          fillBuffer(void);

  /*! Returns true if the buffer is properly filled.
   */
  bool                          bufferReady(void) const;

  /*! Returns a value indicating the buffering progress; 1.0 means finished and
      bufferReady becomes true.
   */
  float                         bufferProgress(void) const;

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          finished(void);

private:
  class BufferThread;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
