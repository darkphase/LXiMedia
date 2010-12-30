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

#define GL_GLEXT_PROTOTYPES

#include "deinterlace.h"


namespace LXiStream {
namespace OpenGlBackend {


const char * const Deinterlace::diShaderCode =
 "uniform sampler2D tex;\n"
 "uniform vec2 pixelSize;\n"
 "uniform float fieldOffset;\n"
 "\n"
 "void main()\n"
 "{\n"
 "  vec2 posB = gl_TexCoord[0].st;\n"
 "  vec2 posA = posB;\n"
 "  posA.y -= pixelSize.y;\n"
 "  vec2 posC = posB;\n"
 "  posC.y += pixelSize.y;\n"
 "\n"
 "  vec4 a = texture2D(tex, posA);\n"
 "  vec4 b = texture2D(tex, posB);\n"
 "  vec4 c = texture2D(tex, posC);\n"
 "\n"
 "  vec2 w = vec2(pixelSize.x, 0.0);\n"
 "  vec4 am = (texture2D(tex, posA - w) + a + texture2D(tex, posA + w)) * 0.333;\n"
 "  vec4 bm = (texture2D(tex, posB - w) + b + texture2D(tex, posB + w)) * 0.333;\n"
 "  vec4 cm = (texture2D(tex, posC - w) + c + texture2D(tex, posC + w)) * 0.333;\n"
 "  vec4 abm = 1.0 - abs(am - bm); abm = 1.0 - (abm * abm);\n"
 "  vec4 acm = abs(am - cm); acm *= acm;\n"
 "\n"
 "  vec4 motion = 1.0 - (abm * (1.0 - acm)); motion *= motion;\n"
 "  motion = 1.0 - (motion * motion);\n"
 "  vec4 mix = (motion * ((a + c) * 0.5)) + ((1.0 - motion) * b);\n"
 "\n"
 "  float field = (posA.y - fieldOffset) / (pixelSize.y * 2.0);\n"
 "  field = ceil(((field - floor(field)) * 2.0) - 0.5);\n"
 "\n"
 "  gl_FragColor = (field * b) + ((1.0 - field) * mix);\n"
// "  gl_FragColor.r = motion;\n"
 "  gl_FragColor.a = 1.0;\n"
 "}\n";


DeinterlaceBlend::DeinterlaceBlend(const QString &, QObject *parent)
  : SInterfaces::VideoDeinterlacer(parent),
    SGlShader(diShaderCode)
{
}

SVideoBufferList DeinterlaceBlend::processBuffer(const SVideoBuffer &videoBuffer)
{
  const STextureBuffer textureBuffer = videoBuffer;
  if (!textureBuffer.isNull())
  {
    const SVideoCodec::FieldMode fieldMode = textureBuffer.codec().fieldMode();

    if ((fieldMode == SVideoCodec::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoCodec::FieldMode_InterlacedBottomFirst))
    {
      STextureBuffer destBuffer(textureBuffer.codec().size());
      const SGlSystem::Texture inTex = textureBuffer.texture();
      const SGlSystem::Texture outTex = destBuffer.texture();

      if (bind(outTex, inTex))
      {
        SGlSystem::printLastGlError();

        glUniform1f(glGetUniformLocation(shaderProgram(), "fieldOffset"), 1.0f / float(inTex.height));
        filterTexture(outTex, inTex);
        release(outTex);

        destBuffer.setTimeStamp(textureBuffer.timeStamp());
        destBuffer.setSubStreamId(textureBuffer.subStreamId());

        return SVideoBufferList() << destBuffer;
      }
    }

    return SVideoBufferList() << textureBuffer;
  }

  return SVideoBufferList() << videoBuffer;
}


DeinterlaceBob::DeinterlaceBob(const QString &, QObject *parent)
  : SInterfaces::VideoDeinterlacer(parent),
    SGlShader(diShaderCode),
    avgFrameTime(STime::fromMSec(40)),
    lastTimeStamp(STime::fromMSec(0))
{
}

SVideoBufferList DeinterlaceBob::processBuffer(const SVideoBuffer &videoBuffer)
{
  const STextureBuffer textureBuffer = videoBuffer;
  if (!textureBuffer.isNull())
  {
    const SVideoCodec::FieldMode fieldMode = textureBuffer.codec().fieldMode();

    if ((fieldMode == SVideoCodec::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoCodec::FieldMode_InterlacedBottomFirst))
    {
      STextureBuffer destBufferA(textureBuffer.codec().size());
      STextureBuffer destBufferB(textureBuffer.codec().size());
      const SGlSystem::Texture inTex = textureBuffer.texture();
      const SGlSystem::Texture outTexA = destBufferA.texture();
      const SGlSystem::Texture outTexB = destBufferB.texture();

      if (bind(outTexA, inTex))
      {
        SGlSystem::printLastGlError();

        glUniform1f(glGetUniformLocation(shaderProgram(), "fieldOffset"), 1.0f / float(inTex.height));
        filterTexture(outTexA, inTex);
        release(outTexA);

        if (bind(outTexB, inTex))
        {
          SGlSystem::printLastGlError();

          glUniform1f(glGetUniformLocation(shaderProgram(), "fieldOffset"), 0.0f);
          filterTexture(outTexB, inTex);
          release(outTexB);

          const STime timeStamp = textureBuffer.timeStamp();
          const STime delta = timeStamp - lastTimeStamp;
          lastTimeStamp = timeStamp;
          if (delta.isPositive() && (delta < STime::fromMSec(250)))
            avgFrameTime = STime::fromUSec(((avgFrameTime.toUSec() * 15) + delta.toUSec()) >> 4);

          if (fieldMode == SVideoCodec::FieldMode_InterlacedTopFirst)
          {
            destBufferA.setTimeStamp(timeStamp);
            destBufferA.setSubStreamId(textureBuffer.subStreamId());
            destBufferB.setTimeStamp(timeStamp + (avgFrameTime / 2));
            destBufferB.setSubStreamId(textureBuffer.subStreamId());

            return SVideoBufferList() << destBufferA << destBufferB;
          }
          else
          {
            destBufferA.setTimeStamp(timeStamp + (avgFrameTime / 2));
            destBufferA.setSubStreamId(textureBuffer.subStreamId());
            destBufferB.setTimeStamp(timeStamp);
            destBufferB.setSubStreamId(textureBuffer.subStreamId());

            return SVideoBufferList() << destBufferB << destBufferA;
          }
        }
      }
    }

    return SVideoBufferList() << textureBuffer;
  }

  return SVideoBufferList() << videoBuffer;
}


} } // End of namespaces
