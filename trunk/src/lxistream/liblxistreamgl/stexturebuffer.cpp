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

#include <liblxistreamgl/stexturebuffer.h>

// Implemented in stexturebuffer.convert.c
extern "C" void LXiStream_STextureBuffer_convertRGBtoBGR(uchar *bgr, const uchar *rgb, unsigned numPixels);

namespace LXiStreamGl {

using namespace LXiStream;


const char * const STextureBuffer::yuvShaderCode =
 "uniform sampler2D tex;\n"
 "uniform vec2 uvScale, uPos, vPos;\n"
 "\n"
 "void main()\n"
 "{\n"
 "  float y = texture2D(tex, gl_TexCoord[0].st).r;\n"
 "  float u = texture2D(tex, (gl_TexCoord[0].st * uvScale) + uPos).r;\n"
 "  float v = texture2D(tex, (gl_TexCoord[0].st * uvScale) + vPos).r;\n"
 "\n"
 "  y = 1.1643 * (y - 0.0625);\n"
 "  u = u - 0.5;\n"
 "  v = v - 0.5;\n"
 "\n"
 "  gl_FragColor.r = y + 1.5958  * v;\n"
 "  gl_FragColor.g = y - 0.39173 * u - 0.81290 * v;\n"
 "  gl_FragColor.b = y + 2.017   * u;\n"
 "  gl_FragColor.a = 1.0;\n"
 "}\n";

GLuint                STextureBuffer::yuvShaderProgram = 0;
GLuint                STextureBuffer::yuvTexture = 0;
unsigned              STextureBuffer::yuvTextureW = 0,
                      STextureBuffer::yuvTextureH = 0;


STextureBuffer::STextureBuffer(void)
               :SBuffer()
{
}

STextureBuffer::STextureBuffer(const SSize &size)
               :SBuffer(new Descriptor(baseTypeId))
{
  descriptor()->texture = SGlSystem::createTexture(size.width(), size.height(), true);

  setCodec(SVideoCodec(SVideoCodec::Format_BGR32,
                       size,
                       0.0f,
                       SVideoCodec::FieldMode_Progressive));
}

STextureBuffer::STextureBuffer(const SBuffer &from)
               :SBuffer()
{
  createTexture(from);
}

STextureBuffer::STextureBuffer(const STextureBuffer &from)
               :SBuffer(from)
{
}

STextureBuffer::STextureBuffer(const QImage &from)
               :SBuffer()
{
  createTexture(from);
}

STextureBuffer & STextureBuffer::operator=(const SBuffer &from)
{
  if (from.typeId() == baseTypeId)
    SBuffer::operator=(from);
  else if (from.typeId() == SVideoBuffer::baseTypeId)
    createTexture(from);
  else
    SBuffer::operator=(from.convert(baseTypeId));

  return *this;
}

STextureBuffer & STextureBuffer::operator=(const STextureBuffer &from)
{
  SBuffer::operator=(from);

  return *this;
}

STextureBuffer & STextureBuffer::operator=(const SVideoBuffer &from)
{
  createTexture(from);

  return *this;
}

/*! Builds a texture for the specified QImage. The size of the actual texture
    will be clipped to a power of 2 (e.g. 128, 256, 512, 1024) and may be,
    based on performance settings, larger or smaller than the provided image.
    If the texture is larger; only a part of the texture is used, the image is
    not scaled up. If the texture is smaller, the image is scaled to fit the
    texture.
    \code
      const SGlSystem::Texture tex = buffer.texture();
      glBindTexture(GL_TEXTURE_2D, tex.handle);
      glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(tex.wf, 0.0f  ); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(tex.wf, tex.hf); glVertex3f( 1.0f,  1.0f, 0.0f);
        glTexCoord2f(0.0f,   tex.hf); glVertex3f(-1.0f,  1.0f, 0.0f);
        glTexCoord2f(0.0f,   0.0f  ); glVertex3f(-1.0f, -1.0f, 0.0f);
      glEnd();
    \endcode
 */
STextureBuffer & STextureBuffer::operator=(const QImage &from)
{
  createTexture(from);

  return *this;
}

SCodecList STextureBuffer::textureCodecs(void)
{
  SCodecList codecs;
  codecs << SVideoCodec::Format_BGR32
         << SVideoCodec::Format_RGB32;

#ifdef ENABLE_GLSL
  if (SGlSystem::canOffloadProcessing())
  {
    codecs << SVideoCodec::Format_YUV410P
           << SVideoCodec::Format_YUV411P
           << SVideoCodec::Format_YUV420P
           << SVideoCodec::Format_YUV422P
           << SVideoCodec::Format_YUV444P;
  }
#endif

  return codecs;
}

void STextureBuffer::createTexture(const SVideoBuffer &videoBuffer)
{
  SGlSystem::checkThread();
  SGlSystem::activateContext();

  if (!videoBuffer.isNull())
  {
    const SSize size = videoBuffer.codec().size();
    const SVideoCodec::Format format = videoBuffer.codec().format();
    const SVideoCodec::FieldMode fieldMode = videoBuffer.codec().fieldMode();

    // Get a descriptor
    Descriptor * desc = NULL;
    {
      const Descriptor * constDesc = const_cast<const STextureBuffer *>(this)->descriptor();
      const bool needFrameBuffer = !((format == SVideoCodec::Format_BGR32) || (format == SVideoCodec::Format_RGB32));

      if ((constDesc->m.typeId == 0) ||
          (bool(constDesc->texture.frameBuffer) != needFrameBuffer) ||
          (int(constDesc->texture.width) < size.width()) ||
          (int(constDesc->texture.height) < size.height()) ||
          (int(constDesc->texture.width) >= (size.width() * 2)) ||
          (int(constDesc->texture.height) >= (size.height() * 2)))
      {
        desc = new Descriptor(baseTypeId);
        desc->texture = SGlSystem::createTexture(size.width(), size.height(), needFrameBuffer);
        setDescriptor(desc);
      }
      else
        desc = descriptor();
    }

    if ((format == SVideoCodec::Format_BGR32) || (format == SVideoCodec::Format_RGB32))
    { // Simply upload data
      desc->texture.wf = float(size.width()) / float(desc->texture.width);
      desc->texture.hf = float(size.height()) / float(desc->texture.height);

      setCodec(SVideoCodec(SVideoCodec::Format_BGR32, size, videoBuffer.codec().frameRate(), fieldMode));

      if (format == SVideoCodec::Format_BGR32)
      {
        glBindTexture(GL_TEXTURE_2D, desc->texture.handle);

        if (videoBuffer.lineSize(0) == (size.width() * sizeof(quint32)))
        {
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, 0,
                          size.width(), size.height(),
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          videoBuffer.scanLine(0));
        }
        else for (int y=0; y<size.height(); y++)
        {
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, y,
                          size.width(), 1,
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          videoBuffer.scanLine(size.height() - y - 1));
        }
      }
      else if (format == SVideoCodec::Format_RGB32) // Need to swap bytes
      {
        quint32 _swapBuffer[4096 + (SMemoryBuffer::dataAlignVal / sizeof(quint32))];
        quint32 * const swapBuffer = SMemoryBuffer::align(_swapBuffer);

        glBindTexture(GL_TEXTURE_2D, desc->texture.handle);
        for (int y=0; y<size.height(); y++)
        {
          LXiStream_STextureBuffer_convertRGBtoBGR((uchar *)swapBuffer,
                                                  videoBuffer.scanLine(size.height() - y - 1),
                                                  size.width());

          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, y,
                          size.width(), 1,
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          swapBuffer);
        }
      }

      SGlSystem::printLastGlError();
    }
#ifdef ENABLE_GLSL
    else if ((format == SVideoCodec::Format_YUV410P) ||
             (format == SVideoCodec::Format_YUV411P) ||
             (format == SVideoCodec::Format_YUV420P) ||
             (format == SVideoCodec::Format_YUV422P) ||
             (format == SVideoCodec::Format_YUV444P))
    { // Upload and convert data
      unsigned yuvW = 0, yuvH = 0;
      SVideoCodec::planarYUVRatio(format, yuvW, yuvH);

      unsigned yuvWidth = size.width(), yuvHeight = size.height();
      if (yuvW < 2) // UV fields below each other
        yuvHeight += (size.height() / yuvH) * 2;
      else // UV fields next to each other
        yuvHeight += size.height() / yuvH;

      const unsigned texWidth = SGlSystem::toTextureSize(qMax(yuvWidth, videoBuffer.lineSize(0)));
      const unsigned texHeight = SGlSystem::toTextureSize(yuvHeight);

      if (desc->texture.frameBuffer->bind())
      {
        glPushAttrib(GL_VIEWPORT_BIT);
        glPushMatrix();
        glViewport(0, 0, desc->texture.width, desc->texture.height);
        glClear(GL_COLOR_BUFFER_BIT);
        SGlSystem::printLastGlError();

        if ((yuvTextureW < texWidth) || (yuvTextureH < texHeight) ||
            (yuvTextureW >= (texWidth * 2)) || (yuvTextureH >= (texHeight * 2)))
        {
          if ((yuvTextureW == 0) && (yuvTextureH == 0))
          {
            Q_ASSERT(yuvTexture == 0);
            glGenTextures(1, &yuvTexture);

            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, (const GLchar **)&yuvShaderCode, NULL);
            glCompileShader(fragmentShader);

            yuvShaderProgram = glCreateProgram();
            glAttachShader(yuvShaderProgram, fragmentShader);
            glLinkProgram(yuvShaderProgram);
            SGlSystem::printGlInfoLog(yuvShaderProgram);

            SGlSystem::printLastGlError();
          }

          glBindTexture(GL_TEXTURE_2D, yuvTexture);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexImage2D(GL_TEXTURE_2D, 0,
                       GL_LUMINANCE8,
                       texWidth, texHeight,
                       0,
                       GL_LUMINANCE,
                       GL_UNSIGNED_BYTE,
                       SGlSystem::emptyPixels());
          SGlSystem::printLastGlError();

          yuvTextureW = texWidth;
          yuvTextureH = texHeight;
        }
        else
        {
          glBindTexture(GL_TEXTURE_2D, yuvTexture);
        }

        // Upload Y
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,
                        videoBuffer.lineSize(0), size.height(),
                        GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        videoBuffer.scanLine(0, 0));
        SGlSystem::printLastGlError();

        // Upload U
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, size.height(),
                        videoBuffer.lineSize(1), size.height() / yuvH,
                        GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        videoBuffer.scanLine(0, 1));
        SGlSystem::printLastGlError();

        // Upload V
        if (yuvW < 2) // UV fields below each other
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, size.height() + (size.height() / yuvH),
                          videoBuffer.lineSize(2), size.height() / yuvH,
                          GL_LUMINANCE,
                          GL_UNSIGNED_BYTE,
                          videoBuffer.scanLine(0, 2));
        else // UV fields next to each other
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          size.width() / yuvW, size.height(),
                          videoBuffer.lineSize(2), size.height() / yuvH,
                          GL_LUMINANCE,
                          GL_UNSIGNED_BYTE,
                          videoBuffer.scanLine(0, 2));

        SGlSystem::printLastGlError();

        const float wf = float(size.width()) / float(texWidth);
        const float hf = float(size.height()) / float(texHeight);
        const float twf = desc->texture.wf = float(size.width()) / float(desc->texture.width);
        const float thf = desc->texture.hf = float(size.height()) / float(desc->texture.height);

        setCodec(SVideoCodec(SVideoCodec::Format_BGR32, size, videoBuffer.codec().frameRate(), fieldMode));

        glUseProgram(yuvShaderProgram);
        glUniform1i(glGetUniformLocation(yuvShaderProgram, "tex"), 0);
        glUniform2f(glGetUniformLocation(yuvShaderProgram, "uvScale"), 1.0f / float(yuvW), 1.0f / float(yuvH));
        glUniform2f(glGetUniformLocation(yuvShaderProgram, "uPos"), 0.0f, hf);
        if (yuvW < 2) // UV fields below each other
          glUniform2f(glGetUniformLocation(yuvShaderProgram, "vPos"), 0.0f, hf + (hf / float(yuvH)));
        else // UV fields next to each other
          glUniform2f(glGetUniformLocation(yuvShaderProgram, "vPos"), wf / float(yuvW), hf);
        SGlSystem::printLastGlError();

          glBegin(GL_QUADS);
            glNormal3f(0.0f, 0.0f, -1.0f);
            glTexCoord2f(wf,   0.0f); glVertex3f(-1.0f + (twf * 2.0f), -1.0f + (thf * 2.0f), 0.0f);
            glTexCoord2f(wf,   hf  ); glVertex3f(-1.0f + (twf * 2.0f), -1.0f,                0.0f);
            glTexCoord2f(0.0f, hf  ); glVertex3f(-1.0f,                -1.0f,                0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,                -1.0f + (thf * 2.0f), 0.0f);
          glEnd();

        glUseProgram(0);

        glPopMatrix();
        glPopAttrib();
        desc->texture.frameBuffer->release();
      }
      else
        operator=(SBuffer());

      SGlSystem::printLastGlError();
    }
#endif

    if (!isNull())
      setTimeStamp(videoBuffer.timeStamp());
  }
}

void STextureBuffer::createTexture(const QImage &image)
{
  SGlSystem::checkThread();

  const Descriptor * constDesc = const_cast<const STextureBuffer *>(this)->descriptor();
  Descriptor * desc = NULL;

  QImage glImage = QGLWidget::convertToGLFormat(image);
  int iw = SGlSystem::toTextureSize(glImage.width());
  int ih = SGlSystem::toTextureSize(glImage.height());

  if ((glImage.width() > iw) || (glImage.height() > ih))
  {
    glImage = glImage.scaled(iw,ih, Qt::KeepAspectRatio, Qt::FastTransformation);
    iw = SGlSystem::toTextureSize(glImage.width());
    ih = SGlSystem::toTextureSize(glImage.height());
  }

  if ((constDesc->m.typeId == 0) ||
      (constDesc->texture.handle == SGlSystem::invalidHandle) ||
      (int(constDesc->texture.width) < iw) ||
      (int(constDesc->texture.height) < iw) ||
      ((int(constDesc->texture.width) - 128) > iw) ||
      ((int(constDesc->texture.height) - 128) > ih))
  {
    desc = new Descriptor(baseTypeId);
    desc->texture = SGlSystem::createTexture(glImage.width(), glImage.height(), false);
    setDescriptor(desc);
  }
  else
    desc = descriptor();

  desc->texture.wf = float(glImage.width()) / float(desc->texture.width);
  desc->texture.hf = float(glImage.height()) / float(desc->texture.height);

  setCodec(SVideoCodec(SVideoCodec::Format_BGR32,
                       SSize(glImage.width(), glImage.height()),
                       0.0f,
                       SVideoCodec::FieldMode_Progressive));

  // Upload data
  glBindTexture(GL_TEXTURE_2D, desc->texture.handle);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                  glImage.width(),
                  glImage.height(),
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  glImage.bits());

  SGlSystem::printLastGlError();
}


STextureBuffer::Descriptor::Descriptor(TypeId typeId)
    : SBuffer::Descriptor(typeId),
      texture()
{
}

STextureBuffer::Descriptor::~Descriptor()
{
  SGlSystem::deleteTexture(texture);
}

SBuffer::Descriptor * STextureBuffer::Descriptor::detach(size_t) const
{
  quint32 * const tmp = new quint32[texture.width * texture.height];
  glBindTexture(GL_TEXTURE_2D, texture.handle);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

  Descriptor * const d = new Descriptor(m.typeId);
  d->texture = SGlSystem::createTexture(texture.width,
                                        texture.height,
                                        tmp);

  d->texture.wf = texture.wf;
  d->texture.hf = texture.hf;

  delete [] tmp;

  return d;
}

LXiStream::SBuffer STextureBuffer::Descriptor::convert(TypeId typeId) const
{
  SGlSystem::checkThread();

  if (typeId == SVideoBuffer::baseTypeId)
  {
    const SVideoCodec videoCodec = codec;
    const SSize size = videoCodec.size();
    SVideoBuffer videoBuffer(texture.width * texture.height * sizeof(quint32));

    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, videoBuffer.bits());

    videoBuffer.setNumBytes(texture.width * texture.height * sizeof(quint32));
    videoBuffer.setCodec(SVideoCodec(SVideoCodec::Format_BGR32,
                                     size,
                                     videoCodec.frameRate(),
                                     videoCodec.fieldMode()));

    const unsigned lineSize = texture.width * sizeof(quint32);
    videoBuffer.setLineSize(0, lineSize);
    videoBuffer.setOffset(0, 0);

    // Flip buffer
    quint32 swapBuffer[4096];
    for (int y=0; y<size.height()/2; y++)
    {
      void * const line1 = videoBuffer.scanLine(y);
      void * const line2 = videoBuffer.scanLine(size.height() - y - 1);

      memcpy(swapBuffer, line1, lineSize);
      memcpy(line1, line2, lineSize);
      memcpy(line2, swapBuffer, lineSize);
    }

    SGlSystem::printLastGlError();

    return videoBuffer;
  }
  else
    return SBuffer::Descriptor::convert(typeId);
}

bool STextureBuffer::Descriptor::mainThread(void) const
{
  return true;
}


} // End of namespace
