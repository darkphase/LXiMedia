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

#include <liblxistreamgl/sglvideoview.h>

namespace LXiStreamGl {

using namespace LXiStream;


SGlVideoView::SGlVideoView(QWidget *parent)
             :QGLWidget(QGLFormat(QGL::DirectRendering | QGL::DoubleBuffer), parent),
              lastBuffer(SBuffer()),
              overScan(1.01f)
{
  SGlSystem::initialize(this);

  setAttribute(Qt::WA_OpaquePaintEvent);
}

SGlVideoView::~SGlVideoView()
{
}

void SGlVideoView::input(const SVideoBuffer &buffer)
{
  SGlSystem::checkThread();

  const STextureBuffer textureBuffer = buffer;

  if (!textureBuffer.isNull())
  {
    lastBuffer = textureBuffer;
    update();
  }
}

void SGlVideoView::initializeGL()
{
  SGlSystem::checkThread();

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_TEXTURE_2D);
}

void SGlVideoView::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
}

void SGlVideoView::paintGL(void)
{
  SGlSystem::checkThread();

  glPushAttrib(GL_ENABLE_BIT | GL_HINT_BIT | GL_CURRENT_BIT);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Render a black background
    glBindTexture(GL_TEXTURE_2D, SGlSystem::blackTexture().handle);
    glBegin(GL_QUADS);
      glNormal3f(0.0f, 0.0f, -1.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 1.0f);
      glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 1.0f);
      glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glEnd();

    if (!lastBuffer.isNull())
    {
      const SSize bufferSize = lastBuffer.codec().size();
      const float ux = float(bufferSize.width()) / float(lastBuffer.texture().width);
      const float uy = float(bufferSize.height()) / float(lastBuffer.texture().height);

      QSize zs = bufferSize.absoluteSize();
      zs.scale(width(), height(), Qt::KeepAspectRatio);
      const float zx = float(zs.width()) / float(width());
      const float zy = float(zs.height()) / float(height());

      glBindTexture(GL_TEXTURE_2D, lastBuffer.texture().handle);
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(ux,   0.0f); glVertex3f( zx, -zy, 0.0f);
        glTexCoord2f(ux,   uy  ); glVertex3f( zx,  zy, 0.0f);
        glTexCoord2f(0.0f, uy  ); glVertex3f(-zx,  zy, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-zx, -zy, 0.0f);
      glEnd();
    }

  glPopAttrib();
}


} // End of namespace
