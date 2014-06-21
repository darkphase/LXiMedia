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

#ifndef LXISTREAMDEVICE_SAUDIOINPUTNODE_H
#define LXISTREAMDEVICE_SAUDIOINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "../export.h"

namespace LXiStreamDevice {

/*! This is a generic audio input node that can be used to obtain audio data
    from an audio device such as a audio capture card.
 */
class LXISTREAMDEVICE_PUBLIC SAudioInputNode : public ::LXiStream::SInterfaces::SourceNode
{
Q_OBJECT
public:
  explicit                      SAudioInputNode(SGraph *, const QString &device = QString::null);
  virtual                       ~SAudioInputNode();

  static QStringList            devices(void);

  void                          setFormat(const SAudioFormat &);
  SAudioFormat                  format() const;

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const SAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
