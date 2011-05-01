/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __VIDEORESIZER_H
#define __VIDEORESIZER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

class VideoResizer : public SInterfaces::VideoResizer
{
Q_OBJECT
public:
                                VideoResizer(const QString &, QObject *);
  virtual                       ~VideoResizer();

  static int                    algoFlags(const QString &);

public: // From SInterfaces::VideoResizer
  virtual void                  setSize(const SSize &);
  virtual SSize                 size(void) const;
  virtual void                  setAspectRatioMode(Qt::AspectRatioMode);
  virtual Qt::AspectRatioMode   aspectRatioMode(void) const;

  virtual bool                  needsResize(const SVideoFormat &);
  virtual SVideoBuffer          processBuffer(const SVideoBuffer &);

private:
  const int                     filterFlags;
  SSize                         scaleSize;
  Qt::AspectRatioMode           scaleAspectRatioMode;
  SVideoFormat                  lastFormat;
  SVideoFormat                  destFormat;

  SwsContext                  * swsContext;
};


} } // End of namespaces

#endif
