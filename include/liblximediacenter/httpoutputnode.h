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

#ifndef LXMEDIACENTER_HTTPOUTPUTNODE_H
#define LXMEDIACENTER_HTTPOUTPUTNODE_H

#include <QtCore>
#include <QtNetwork>
#include <LXiStream>

namespace LXiMediaCenter {

/*! This is a generic output node, writing to a QIODevice.
 */
class HttpOutputNode : public QObject,
                       public SGraph::SinkNode,
                       private SInterfaces::BufferWriter::WriteCallback
{
Q_OBJECT
public:
  explicit                      HttpOutputNode(SGraph *);
  virtual                       ~HttpOutputNode();

  void                          setHeader(const QByteArray &);
  void                          enablePseudoStreaming(float speed, STime preload = STime::fromSec(10));
  bool                          addSocket(QAbstractSocket *);

  bool                          openFormat(const QString &, const SAudioCodec &, STime);
  bool                          openFormat(const QString &, const SAudioCodec &, const SVideoCodec &, STime);
  bool                          openFormat(const QString &, const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime);

  virtual bool                  start(STimer *);
  virtual void                  stop(void);

  bool                          isConnected(void) const;

signals:
  void                          disconnected(void);

public slots:
  void                          input(const SEncodedAudioBuffer &);
  void                          input(const SEncodedVideoBuffer &);
  void                          input(const SEncodedDataBuffer &);

private: // From SInterfaces::BufferReader::WriteCallback
  virtual void                  write(const uchar *, qint64);

private:
  void                          blockUntil(STime);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
