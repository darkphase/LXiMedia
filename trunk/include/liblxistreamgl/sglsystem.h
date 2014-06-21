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

#ifndef LXSTREAM_SGLSYSTEM_H
#define LXSTREAM_SGLSYSTEM_H

#include <QtCore>
#include <QtOpenGL>

namespace LXiStreamGl {


class SGlSystem
{
public:
  static const GLuint           invalidHandle;

  struct Texture
  {
    inline                      Texture(void)
        : frameBuffer(NULL), handle(invalidHandle), width(0), height(0),
          wf(1.0f), hf(1.0f) { }

    QGLFramebufferObject      * frameBuffer;
    GLuint                      handle;
    unsigned                    width, height;
    float                       wf, hf;
  };

public:
  static void                   initialize(QGLWidget *shareContext);
  static void                   shutdown(void);

  static bool                   canOffloadProcessing(void) __attribute__((pure));

  static inline void            checkThread(void)                               { checkThread(false); }
  static void                   activateContext(void);
  static bool                   printLastGlError(const char * = NULL, int = 0);
  static void                   printGlInfoLog(GLuint program, const char * = NULL, int = 0);

  static unsigned               toTextureSize(unsigned size);
  static const void           * emptyPixels(void) __attribute__((pure));
  static Texture                createTexture(unsigned width, unsigned height, bool fbo, const void * fromData = NULL);
  static void                   deleteTexture(const Texture &);

  static const Texture        & blackTexture(void) __attribute__((pure));
  static const Texture        & whiteTexture(void) __attribute__((pure));

  static inline quint64         allocatedTextureMem(void)                       { return textureMem(); }

private:
                                SGlSystem();

  static void                   checkThread(bool);
  static QQueue<Texture>      & textureCache(void) __attribute__((pure));
  static quint64              & textureMem(void) __attribute__((pure));

private:
  static unsigned               textureCacheSize;
  static GLint                  maxTextureSize;
  static QGLWidget            * context;
  static bool                   contextOwner;
};


} // End of namespace

#define printLastGlError() printLastGlError(__FILE__, __LINE__)
#define printGlInfoLog(p) printGlInfoLog(p, __FILE__, __LINE__)

#endif
