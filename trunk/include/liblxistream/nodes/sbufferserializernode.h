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
  _lxi_internal void            close(void);

private:
  template <class _buffer>
  _lxi_internal void            serialize(const _buffer &, quint32 bufferId);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
