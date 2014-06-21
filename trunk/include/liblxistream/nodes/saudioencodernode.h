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

#ifndef LXSTREAM_SAUDIOENCODERNODE_H
#define LXSTREAM_SAUDIOENCODERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class SIOOutputNode;

class LXISTREAM_PUBLIC SAudioEncoderNode : public SInterfaces::Node
{
Q_OBJECT
friend class SIOOutputNode;
public:
  typedef SInterfaces::AudioEncoder::Flags Flags;

public:
  explicit                      SAudioEncoderNode(SGraph *);
  virtual                       ~SAudioEncoderNode();

  static QStringList            codecs(void);
  static QStringList            losslessCodecs(void);

  bool                          openCodec(const SAudioCodec &, SIOOutputNode *, STime duration, Flags = SInterfaces::AudioEncoder::Flag_None);
  SAudioCodec                   codec(void) const;

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);

signals:
  void                          output(const SEncodedAudioBuffer &);

private:
  const SInterfaces::AudioEncoder * encoder(void) const;

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
