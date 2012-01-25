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

#ifndef LXSTREAM_STEXTUREBUFFER_H
#define LXSTREAM_STEXTUREBUFFER_H

#include <QtGui/QImage>
#include <LXiStream>
#include <liblxistreamgl/sglsystem.h>

namespace LXiStreamGl {

/*! This class represents a buffer containing video data. These do not have to
    be raw pixels, it also may be encoded with any video codec.
 */
class STextureBuffer : public LXiStream::SBuffer
{
public:
  static const TypeId           baseTypeId = (0x02 << SBuffer::mediumTypeIdOfs) | (LXiStream::SVideoBuffer::baseTypeId & SBuffer::dataTypeIdMask);

protected:
  class Descriptor : public LXiStream::SBuffer::Descriptor
  {
  public:
    explicit                    Descriptor(TypeId typeId);
    virtual                     ~Descriptor();

    virtual LXiStream::SBuffer::Descriptor * detach(size_t newSizeHint = 0) const;
    virtual LXiStream::SBuffer  convert(TypeId) const;
    virtual bool                mainThread(void) const;

  public:
    SGlSystem::Texture          texture;
  };

public:
                                STextureBuffer(void);
                                STextureBuffer(const SSize &);
                                STextureBuffer(const SBuffer &);
                                STextureBuffer(const STextureBuffer &);
                                STextureBuffer(const QImage &);

  STextureBuffer              & operator=(const SBuffer &);
  STextureBuffer              & operator=(const STextureBuffer &);
  STextureBuffer              & operator=(const LXiStream::SVideoBuffer &);
  STextureBuffer              & operator=(const QImage &);

  inline LXiStream::SVideoCodec codec(void) const                               { return descriptor()->codec; }

  static LXiStream::SCodecList  textureCodecs(void) __attribute__((pure));
  inline const SGlSystem::Texture & texture(void) const                         { return descriptor()->texture; }

protected:
  inline const Descriptor     * descriptor(void) const                          { return static_cast<const Descriptor *>(SBuffer::descriptor()); }
  inline Descriptor           * descriptor(bool metaOnly = false)               { return static_cast<Descriptor *>(SBuffer::descriptor(metaOnly)); }

  void                          createTexture(const LXiStream::SVideoBuffer &);
  void                          createTexture(const QImage &);

private:
  static const char     * const yuvShaderCode;
  static GLuint                 yuvShaderProgram;
  static GLuint                 yuvTexture;
  static unsigned               yuvTextureW, yuvTextureH;
};


} // End of namespace

#endif
