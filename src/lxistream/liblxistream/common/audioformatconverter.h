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

#ifndef LXISTREAM_AUDIOFORMATCONVERTER_H
#define LXISTREAM_AUDIOFORMATCONVERTER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {

template <SAudioFormat::Format _srcFormat, SAudioFormat::Format _dstFormat>
class AudioFormatConverterBase : public SInterfaces::AudioFormatConverter
{
public:
  template <class _instance>
  static void registerClass(int priority = 0)
  {
    SInterfaces::AudioFormatConverter::registerClass<_instance>(_srcFormat, _dstFormat, priority);
  }

public:
  explicit                      AudioFormatConverterBase(const QString &, QObject *);
  virtual                       ~AudioFormatConverterBase();

public: // From SInterfaces::AudioFormatConverter
  virtual bool                  openFormat(const SAudioFormat &srcFormat, const SAudioFormat &dstFormat);
};

////////////////////////////////////////////////////////////////////////////////
// Swap
class AudioFormatConverter_Format_PCM_S16LE_Format_PCM_S16BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16LE, SAudioFormat::Format_PCM_S16BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16LE_Format_PCM_S16BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16LE, SAudioFormat::Format_PCM_S16BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16BE_Format_PCM_S16LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16BE, SAudioFormat::Format_PCM_S16LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16BE_Format_PCM_S16LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16BE, SAudioFormat::Format_PCM_S16LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U16LE_Format_PCM_U16BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U16LE, SAudioFormat::Format_PCM_U16BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U16LE_Format_PCM_U16BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U16LE, SAudioFormat::Format_PCM_U16BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U16BE_Format_PCM_U16LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U16BE, SAudioFormat::Format_PCM_U16LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U16BE_Format_PCM_U16LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U16BE, SAudioFormat::Format_PCM_U16LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32LE_Format_PCM_S32BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32LE, SAudioFormat::Format_PCM_S32BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32LE_Format_PCM_S32BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32LE, SAudioFormat::Format_PCM_S32BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32BE_Format_PCM_S32LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32BE, SAudioFormat::Format_PCM_S32LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32BE_Format_PCM_S32LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32BE, SAudioFormat::Format_PCM_S32LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U32LE_Format_PCM_U32BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U32LE, SAudioFormat::Format_PCM_U32BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U32LE_Format_PCM_U32BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U32LE, SAudioFormat::Format_PCM_U32BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U32BE_Format_PCM_U32LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U32BE, SAudioFormat::Format_PCM_U32LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U32BE_Format_PCM_U32LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U32BE, SAudioFormat::Format_PCM_U32LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F32LE_Format_PCM_F32BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F32LE, SAudioFormat::Format_PCM_F32BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F32LE_Format_PCM_F32BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F32LE, SAudioFormat::Format_PCM_F32BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F32BE_Format_PCM_F32LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F32BE, SAudioFormat::Format_PCM_F32LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F32BE_Format_PCM_F32LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F32BE, SAudioFormat::Format_PCM_F32LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F64LE_Format_PCM_F64BE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F64LE, SAudioFormat::Format_PCM_F64BE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F64LE_Format_PCM_F64BE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F64LE, SAudioFormat::Format_PCM_F64BE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F64BE_Format_PCM_F64LE : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F64BE, SAudioFormat::Format_PCM_F64LE>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F64BE_Format_PCM_F64LE(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F64BE, SAudioFormat::Format_PCM_F64LE>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};


////////////////////////////////////////////////////////////////////////////////
// Bit-depth conversion
class AudioFormatConverter_Format_PCM_S8_Format_PCM_S16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S8, SAudioFormat::Format_PCM_S16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S8_Format_PCM_S16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S8, SAudioFormat::Format_PCM_S16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U8_Format_PCM_U16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U8, SAudioFormat::Format_PCM_U16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U8_Format_PCM_U16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U8, SAudioFormat::Format_PCM_U16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16_Format_PCM_S8 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S8>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16_Format_PCM_S8(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S8>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U16_Format_PCM_U8 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U8>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U16_Format_PCM_U8(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U8>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16_Format_PCM_S32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16_Format_PCM_S32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U16_Format_PCM_U32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U16_Format_PCM_U32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32_Format_PCM_S16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_S16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32_Format_PCM_S16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_S16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U32_Format_PCM_U16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_U16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U32_Format_PCM_U16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_U16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16_Format_PCM_F32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16_Format_PCM_F32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F32_Format_PCM_S16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F32_Format_PCM_S16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32_Format_PCM_F32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32_Format_PCM_F32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F32_Format_PCM_S32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F32_Format_PCM_S32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16_Format_PCM_F64 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F64>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16_Format_PCM_F64(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F64>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F64_Format_PCM_S16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F64_Format_PCM_S16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32_Format_PCM_F64 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F64>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32_Format_PCM_F64(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F64>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_F64_Format_PCM_S32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_F64_Format_PCM_S32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};


////////////////////////////////////////////////////////////////////////////////
// Signed-unsigned conversion
class AudioFormatConverter_Format_PCM_U16_Format_PCM_S16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_S16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U16_Format_PCM_S16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_S16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S16_Format_PCM_U16 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_U16>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S16_Format_PCM_U16(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_U16>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_U32_Format_PCM_S32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_S32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_U32_Format_PCM_S32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_S32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

class AudioFormatConverter_Format_PCM_S32_Format_PCM_U32 : public AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_U32>
{
Q_OBJECT
public:
  inline AudioFormatConverter_Format_PCM_S32_Format_PCM_U32(const QString &name, QObject *parent)
    : AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_U32>(name, parent)
  {
  }

  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &);
};

} } // End of namespaces

#endif
