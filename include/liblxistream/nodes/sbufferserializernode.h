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

#ifndef LXISTREAM_SBUFFERSERIALIZERNODE_H
#define LXISTREAM_SBUFFERSERIALIZERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SBufferSerializerNode : public SInterfaces::SinkNode
{
Q_OBJECT
public:
  explicit                      SBufferSerializerNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SBufferSerializerNode();

  void                          setIODevice(QIODevice *, bool autoClose = false);
  bool                          hasIODevice(void) const;

public: // From SInterfaces::SinkNode
  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const QByteArray &);
  void                          input(const SAudioBuffer &);
  void                          input(const SVideoBuffer &);
  void                          input(const SSubtitleBuffer &);
  void                          input(const SSubpictureBuffer &);

signals:
  void                          closed(void);

private slots:
  void                          close(void);

private:
  template <class _buffer>
  void                          serialize(const _buffer &, quint32 bufferId);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
