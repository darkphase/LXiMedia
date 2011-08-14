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
#include <libmms/mms.h>

namespace LXiStream {
namespace MMSBackend {

class NetworkBufferReader : public SInterfaces::NetworkBufferReader,
                            public SInterfaces::BufferReader::ReadCallback
{
Q_OBJECT
public:
  explicit                      NetworkBufferReader(const QString &, QObject *);
  virtual                       ~NetworkBufferReader();

public: // From SInterfaces::BufferReader
  virtual bool                  openProtocol(const QString &);

  virtual bool                  start(const QUrl &url, ProduceCallback *, quint16 programId);
  virtual void                  stop(void);

  virtual bool                  buffer(void);
  virtual STime                 bufferDuration(void) const;

  virtual bool                  process(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QList<StreamId> &);

public: // From SInterfaces::BufferReader::ReadCallback
  virtual qint64                read(uchar *buffer, qint64 size);
  virtual qint64                seek(qint64 offset, int whence);

private:
  static QList<QUrl>            resolve(const QUrl &);

private:
  ::mms_t                     * mmsHandle;

  SInterfaces::BufferReader   * bufferReader;
};


} } // End of namespaces

#endif
