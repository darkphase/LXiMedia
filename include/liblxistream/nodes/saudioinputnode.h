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

#ifndef LXISTREAM_SAUDIOINPUTNODE_H
#define LXISTREAM_SAUDIOINPUTNODE_H

#include <QtCore>
#include "../sinterfaces.h"

namespace LXiStream {

/*! This is a generic audio input node that can be used to obtain audio data
    from an audio device such as a audio capture card.
 */
class SAudioInputNode : public QObject,
                        public SInterfaces::SourceNode
{
Q_OBJECT
public:
  explicit                      SAudioInputNode(SGraph *, const QString &device = QString::null);
  virtual                       ~SAudioInputNode();

  static QStringList            devices(void);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

signals:
  void                          output(const SAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
