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

#ifndef LXISTREAMGUI_SVIDEOGENERATORNODE_H
#define LXISTREAMGUI_SVIDEOGENERATORNODE_H

#include <QtCore>
#include <QtGui>
#include <LXiStream>
#include "../export.h"

namespace LXiStreamGui {

class SImage;

class LXISTREAMGUI_PUBLIC SVideoGeneratorNode : public SInterfaces::Node
{
Q_OBJECT
public:
  explicit                      SVideoGeneratorNode(SGraph *);
  virtual                       ~SVideoGeneratorNode();

  void                          setImage(const SImage &);
  const SImage                & image(void) const;
  void                          setFrameRate(const SInterval &);
  const SInterval             & frameRate(void) const;

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);

signals:
  void                          output(const SVideoBuffer &);

public: // Helper methods
  static SImage                 drawCorneredImage(const SSize &);
  static SImage                 drawBusyWaitImage(const SSize &, int angle);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
