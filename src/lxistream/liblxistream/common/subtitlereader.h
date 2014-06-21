/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef __SUBTITLEREADER_H
#define __SUBTITLEREADER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {

class SubtitleReader : public SInterfaces::BufferReader
{
Q_OBJECT
public:
                                SubtitleReader(const QString &, QObject *parent);
  virtual                       ~SubtitleReader();

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(QIODevice *, ProduceCallback *, bool streamed);
  virtual void                  stop(void);

public: // From SInterfaces::AbstractBufferReader
  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual int                   numTitles(void) const;
  virtual QList<AudioStreamInfo> audioStreams(int title) const;
  virtual QList<VideoStreamInfo> videoStreams(int title) const;
  virtual QList<DataStreamInfo> dataStreams(int title) const;
  virtual void                  selectStreams(int title, const QVector<StreamId> &);

  virtual bool                  process(void);

private:
  SEncodedDataBuffer            readNextSrtSubtitle(void);

private:
  QByteArray                    subtitleData;
  QBuffer                       subtitleBuffer;

  QIODevice                   * ioDevice;
  ProduceCallback             * produceCallback;

  bool                          open;
  bool                          utf8;
  SDataCodec                    dataCodec;
  const char                  * language;

  bool                          selected;
  STime                         pos;
};

} } // End of namespaces

#endif
