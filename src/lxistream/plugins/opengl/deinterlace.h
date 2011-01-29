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

#ifndef OPENGLBACKEND_DEINTERLACE_H
#define OPENGLBACKEND_DEINTERLACE_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGl>

namespace LXiStream {
namespace OpenGlBackend {


class Deinterlace
{
protected:
  static const char     * const diShaderCode;
};


class DeinterlaceBlend : public SInterfaces::VideoDeinterlacer,
                         public SGlShader,
                         private Deinterlace
{
Q_OBJECT
public:
  explicit                      DeinterlaceBlend(const QString &, QObject *);

  virtual SVideoBufferList      processBuffer(const SVideoBuffer &);

};


class DeinterlaceBob : public SInterfaces::VideoDeinterlacer,
                       public SGlShader,
                       private Deinterlace
{
Q_OBJECT
public:
  explicit                      DeinterlaceBob(const QString &, QObject *);

  virtual SVideoBufferList      processBuffer(const SVideoBuffer &);

private:
  STime                         avgFrameTime;
  STime                         lastTimeStamp;
};


} } // End of namespaces

#endif