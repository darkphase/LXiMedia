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

#ifndef LXSTREAM_SVIDEODECODERNODE_H
#define LXSTREAM_SVIDEODECODERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sgraph.h"
#include "../sinterfaces.h"

namespace LXiStream {

class SVideoBuffer;

class S_DSO_PUBLIC SVideoDecoderNode : public QObject,
                                       public SGraph::Node
{
Q_OBJECT
Q_PROPERTY(LXiStream::SInterfaces::VideoDecoder::Flags flags READ flags WRITE setFlags)
public:
  typedef SInterfaces::VideoDecoder::Flags Flags;

public:
  explicit                      SVideoDecoderNode(SGraph *, Flags = SInterfaces::VideoDecoder::Flag_None);
  virtual                       ~SVideoDecoderNode();

  static QStringList            codecs(void);

  Flags                         flags(void) const;
  void                          setFlags(Flags);

public slots:
  void                          input(const SEncodedVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  void                          processTask(const SEncodedVideoBuffer &, SInterfaces::VideoDecoder *decoder);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
