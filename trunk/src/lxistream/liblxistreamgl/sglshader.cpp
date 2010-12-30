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

#include <liblxistreamgl/sglshader.h>
#include <liblxistreamgl/stexturebuffer.h>

namespace LXiStreamGl {


SGlShader::SGlShader(const char *shaderCode)
          :shaderCode(shaderCode),
           shader(0),
           program(0)
{
  SGlSystem::checkThread();
  SGlSystem::activateContext();

  shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader, 1, (const GLchar **)&shaderCode, NULL);
  glCompileShader(shader);

  program = glCreateProgram();
  glAttachShader(program, shader);
  glLinkProgram(program);
  SGlSystem::printGlInfoLog(program);

  SGlSystem::printLastGlError();
}

SGlShader::~SGlShader()
{
  glDeleteProgram(program);
  program = 0;
  glDeleteShader(shader);
  shader = 0;

  SGlSystem::printLastGlError();
}

STextureBuffer SGlShader::processBuffer(const STextureBuffer &textureBuffer)
{
  if (!textureBuffer.isNull())
  {
    STextureBuffer destBuffer(textureBuffer.codec().size().size());

    const SGlSystem::Texture inTex = textureBuffer.texture();
    const SGlSystem::Texture outTex = destBuffer.texture();

    if (bind(outTex, inTex))
    {
      filterTexture(outTex, inTex);
      release(outTex);

      destBuffer.setTimeStamp(textureBuffer.timeStamp());

      return destBuffer;
    }
  }

  return SBuffer();
}

bool SGlShader::bind(const SGlSystem::Texture &outTex, const SGlSystem::Texture &inTex)
{
  if (outTex.frameBuffer->bind())
  {
    glPushAttrib(GL_VIEWPORT_BIT);
    glPushMatrix();
    glViewport(0, 0, outTex.width, outTex.height);
    glClear(GL_COLOR_BUFFER_BIT);
    SGlSystem::printLastGlError();

    glBindTexture(GL_TEXTURE_2D, inTex.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    SGlSystem::printLastGlError();

    glUseProgram(program);
    SGlSystem::printLastGlError();

    return true;
  }

  return false;
}

bool SGlShader::bind(const SGlSystem::Texture &outTex)
{
  if (outTex.frameBuffer->bind())
  {
    glPushAttrib(GL_VIEWPORT_BIT);
    glPushMatrix();
    glViewport(0, 0, outTex.width, outTex.height);
    glClear(GL_COLOR_BUFFER_BIT);
    SGlSystem::printLastGlError();

    glUseProgram(program);
    SGlSystem::printLastGlError();

    return true;
  }

  return false;
}

void SGlShader::release(const SGlSystem::Texture &outTex)
{
  glUseProgram(0);

  glPopMatrix();
  glPopAttrib();
  outTex.frameBuffer->release();

  SGlSystem::printLastGlError();
}

void SGlShader::filterTexture(const SGlSystem::Texture &outTex, const SGlSystem::Texture &inTex)
{
  glUniform1i(glGetUniformLocation(program, "tex"), 0);
  glUniform2f(glGetUniformLocation(program, "pixelSize"),
              1.0f / float(inTex.width),
              1.0f / float(inTex.height));
  SGlSystem::printLastGlError();

  glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(inTex.wf, inTex.hf); glVertex3f(-1.0f + (outTex.wf * 2.0f), -1.0f + (outTex.hf * 2.0f), 0.0f);
    glTexCoord2f(inTex.wf, 0.0f    ); glVertex3f(-1.0f + (outTex.wf * 2.0f), -1.0f,                      0.0f);
    glTexCoord2f(0.0f,     0.0f    ); glVertex3f(-1.0f,                      -1.0f,                      0.0f);
    glTexCoord2f(0.0f,     inTex.hf); glVertex3f(-1.0f,                      -1.0f + (outTex.hf * 2.0f), 0.0f);
  glEnd();
  SGlSystem::printLastGlError();
}


} // End of namespace
