/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __NETWORKBUFFERREADER_H
#define __NETWORKBUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"
#include "bufferreaderbase.h"

namespace LXiStream {
namespace FFMpegBackend {

class NetworkBufferReader : public SInterfaces::NetworkBufferReader,
                            public BufferReaderBase
{
Q_OBJECT
public:
  explicit                      NetworkBufferReader(const QString &, QObject *);
  virtual                       ~NetworkBufferReader();

  inline bool                   process(bool fast)                              { return BufferReaderBase::process(fast); }

public: // From SInterfaces::NetworkBufferReader
  virtual bool                  openProtocol(const QString &);

  virtual bool                  start(const QUrl &url, ProduceCallback *, quint16 programId);
  virtual void                  stop(void);
  inline virtual bool           process(void)                                   { return BufferReaderBase::process(); }

private:
};

} } // End of namespaces

#endif
