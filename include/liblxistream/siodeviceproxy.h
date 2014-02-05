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

#ifndef LXISTREAM_SIODEVICEPROXY_H
#define LXISTREAM_SIODEVICEPROXY_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SIODeviceProxy : public QIODevice
{
public:
  explicit                      SIODeviceProxy(QObject *parent = NULL);
  virtual                       ~SIODeviceProxy();

  QIODevice                   * createReader(QObject * parent = NULL);
  bool                          isActive() const;
  bool                          isReusable() const;

public: // From QIODevice
  virtual bool                  isSequential() const;
  virtual bool                  open(OpenMode mode);
  virtual void                  close(void);

  virtual bool                  seek(qint64);
  virtual qint64                pos() const;
  virtual qint64                size(void) const;
  virtual qint64                bytesAvailable() const;

  virtual qint64                readData(char *data, qint64 maxSize);
  virtual qint64                writeData(const char *data, qint64 maxSize);

private:
  class Reader;
  struct Data;
  QExplicitlySharedDataPointer<Data> d;
};

} // End of namespace

#endif
