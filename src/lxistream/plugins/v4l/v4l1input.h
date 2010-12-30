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

#ifndef V4LBACKEND_V4L1INPUT_H
#define V4LBACKEND_V4L1INPUT_H

#include <QtCore>
#include <LXiStream>
#include "videodev.h"

namespace LXiStream {
namespace V4lBackend {

class V4l1Input : public SInterfaces::VideoInput
{
Q_OBJECT
public:
  class Memory : public SBuffer::Memory
  {
  public:
                                Memory(char *data, int size, int, V4l1Input *);
    virtual                     ~Memory();

  private:
    int                         bufferIndex;
    V4l1Input           * const parent;
  };

public:
  static QList<SFactory::Scheme> listDevices(void);

public:
                                V4l1Input(const QString &, QObject *);
  virtual                       ~V4l1Input();

public: // From SInterfaces::VideoInput
  virtual void                  setFormat(const SVideoFormat &);
  virtual SVideoFormat          format(void);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

private:
  static quint16                toV4L(SVideoFormat::Format);
  void                          queueBuffer(int);

private:
  static QMap<QString, int>     deviceMap;

  int                           devDesc;
  video_capability              capabilities;

  mutable QMutex                mutex;
  STimer                        timer;
  SVideoFormat                  outFormat;

  video_mbuf                    buffers;
  video_mmap                  * mmaps;
  char                        * map;
  QSemaphore                  * bufferLock;
  size_t                        bufferSize;
  int                           currentBufferIndex;
};


} } // End of namespaces

#endif
