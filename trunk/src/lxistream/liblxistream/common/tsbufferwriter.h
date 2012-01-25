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

#ifndef LXISTREAMCOMMON_TSBUFFERWRITER_H
#define LXISTREAMCOMMON_TSBUFFERWRITER_H

#include <QtCore>
#include <LXiStream>
#include "mpeg.h"

namespace LXiStream {
namespace Common {

class TsBufferWriter : public SInterfaces::BufferWriter
{
Q_OBJECT
private:
  class Filter : public QIODevice
  {
  public:
    explicit                    Filter(TsBufferWriter *parent);

  protected:
    virtual qint64              readData(char *data, qint64 maxSize);
    virtual qint64              writeData(const char *data, qint64 maxSize);

  private:
    TsBufferWriter      * const parent;
  };

public:
  explicit                      TsBufferWriter(const QString &, QObject *);
  virtual                       ~TsBufferWriter();

public: // From SInterfaces::BufferWriter
  virtual bool                  openFormat(const QString &);
  virtual bool                  addStream(const SInterfaces::AudioEncoder *, STime);
  virtual bool                  addStream(const SInterfaces::VideoEncoder *, STime);

  virtual bool                  start(QIODevice *);
  virtual void                  stop(void);
  virtual void                  process(const SEncodedAudioBuffer &);
  virtual void                  process(const SEncodedVideoBuffer &);
  virtual void                  process(const SEncodedDataBuffer &);

private:
  SInterfaces::BufferWriter   * bufferWriter;
  Filter                        filter;
  QIODevice                   * ioDevice;
};


} } // End of namespaces

#endif
