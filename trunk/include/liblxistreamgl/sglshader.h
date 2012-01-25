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

#ifndef LXSTREAM_SGLSHADER_H
#define LXSTREAM_SGLSHADER_H

#include <QtCore/QObject>
#include <LXiStream>
#include <liblxistreamgl/sglsystem.h>
#include <liblxistreamgl/stexturebuffer.h>

namespace LXiStreamGl {


class SGlShader
{
public:
  explicit                      SGlShader(const char *);
  virtual                       ~SGlShader();

  STextureBuffer                processBuffer(const STextureBuffer &);

protected:
  inline unsigned               shaderProgram(void) const                       { return program; }
  bool                          bind(const SGlSystem::Texture &, const SGlSystem::Texture &);
  bool                          bind(const SGlSystem::Texture &);
  void                          release(const SGlSystem::Texture &);
  void                          filterTexture(const SGlSystem::Texture &, const SGlSystem::Texture &);

private:
  const char            * const shaderCode;
  unsigned                      shader;
  unsigned                      program;
};


} // End of namespace

#endif
