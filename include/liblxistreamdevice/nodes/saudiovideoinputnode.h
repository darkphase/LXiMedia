/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#ifndef LXISTREAMDEVICE_SAUDIOVIDEOINPUTNODE_H
#define LXISTREAMDEVICE_SAUDIOVIDEOINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "../export.h"

namespace LXiStreamDevice {

/*! This is a generic audio/video input node that can be used to obtain audio
    and video data from a capture device such as a video capture card.
 */
class LXISTREAMDEVICE_PUBLIC SAudioVideoInputNode : public ::LXiStream::SInterfaces::SourceNode
{
Q_OBJECT
public:
  explicit                      SAudioVideoInputNode(SGraph *, const QString &device = QString::null);
  virtual                       ~SAudioVideoInputNode();

  static QStringList            devices(void);

  void                          setFormat(const SVideoFormat &);
  void                          setMaxBuffers(int);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);

private:
  template <class _input> class Thread;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
