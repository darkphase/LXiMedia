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

#ifndef LXSTREAM_SAUDIODECODERNODE_H
#define LXSTREAM_SAUDIODECODERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../sgraph.h"
#include "../export.h"

namespace LXiStream {

class SAudioBuffer;

class LXISTREAM_PUBLIC SAudioDecoderNode : public QObject,
                                           public SGraph::Node
{
Q_OBJECT
public:
  typedef SInterfaces::AudioDecoder::Flags Flags;

public:
  explicit                      SAudioDecoderNode(SGraph *, Flags = SInterfaces::AudioDecoder::Flag_None);
  virtual                       ~SAudioDecoderNode();

  static QStringList            codecs(void);

  Flags                         flags(void) const;
  void                          setFlags(Flags);

public: // From SGraph::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SEncodedAudioBuffer &);

signals:
  void                          output(const SAudioBuffer &);

private:
  _lxi_internal void            processTask(const SEncodedAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
