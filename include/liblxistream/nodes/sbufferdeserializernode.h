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

#ifndef LXISTREAM_SBUFFERDESERIALIZERNODE_H
#define LXISTREAM_SBUFFERDESERIALIZERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SBufferDeserializerNode : public SInterfaces::SourceNode
{
Q_OBJECT
public:
  explicit                      SBufferDeserializerNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SBufferDeserializerNode();

  void                          setIODevice(QIODevice *);
  bool                          hasIODevice(void) const;

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const QByteArray &);
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          output(const SSubtitleBuffer &);
  void                          output(const SSubpictureBuffer &);
  void                          finished(void);

private:
  _lxi_internal static bool     read(QIODevice *, char *, unsigned);
  template <class _buffer>
  _lxi_internal void            deserialize(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
