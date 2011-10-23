/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
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

#include "audioformatconverter.h"
#include "../../algorithms/audioconvert.h"
#include "../../algorithms/data.h"

namespace LXiStream {
namespace Common {

template <SAudioFormat::Format _srcFormat, SAudioFormat::Format _dstFormat>
AudioFormatConverterBase<_srcFormat, _dstFormat>::AudioFormatConverterBase(const QString &, QObject *parent)
  : SInterfaces::AudioFormatConverter(parent)
{
}

template <SAudioFormat::Format _srcFormat, SAudioFormat::Format _dstFormat>
AudioFormatConverterBase<_srcFormat, _dstFormat>::~AudioFormatConverterBase()
{
}

template <SAudioFormat::Format _srcFormat, SAudioFormat::Format _dstFormat>
bool AudioFormatConverterBase<_srcFormat, _dstFormat>::openFormat(const SAudioFormat &srcFormat, const SAudioFormat &dstFormat)
{
  if ((srcFormat == _srcFormat) && (dstFormat == _dstFormat))
    return true;

  return false;
}


SAudioBuffer AudioFormatConverter_Format_PCM_S16LE_Format_PCM_S16BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16BE_Format_PCM_S16LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U16LE_Format_PCM_U16BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U16LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U16BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<quint16 *>(destBuffer.data()),
        reinterpret_cast<const quint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U16BE_Format_PCM_U16LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U16BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U16LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<quint16 *>(destBuffer.data()),
        reinterpret_cast<const quint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32LE_Format_PCM_S32BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32BE_Format_PCM_S32LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U32LE_Format_PCM_U32BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U32LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U32BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<quint32 *>(destBuffer.data()),
        reinterpret_cast<const quint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U32BE_Format_PCM_U32LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U32BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U32LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<quint32 *>(destBuffer.data()),
        reinterpret_cast<const quint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F32LE_Format_PCM_F32BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F32LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F32BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<float *>(destBuffer.data()),
        reinterpret_cast<const float *>(audioBuffer.data()),
        destBuffer.size() / sizeof(float));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F32BE_Format_PCM_F32LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F32BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F32LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<float *>(destBuffer.data()),
        reinterpret_cast<const float *>(audioBuffer.data()),
        destBuffer.size() / sizeof(float));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F64LE_Format_PCM_F64BE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F64LE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F64BE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<double *>(destBuffer.data()),
        reinterpret_cast<const double *>(audioBuffer.data()),
        destBuffer.size() / sizeof(double));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F64BE_Format_PCM_F64LE::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F64BE))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F64LE,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::Data::swapBytes(
        reinterpret_cast<double *>(destBuffer.data()),
        reinterpret_cast<const double *>(audioBuffer.data()),
        destBuffer.size() / sizeof(double));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16LE, SAudioFormat::Format_PCM_S16BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16BE, SAudioFormat::Format_PCM_S16LE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U16LE, SAudioFormat::Format_PCM_U16BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U16BE, SAudioFormat::Format_PCM_U16LE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32LE, SAudioFormat::Format_PCM_S32BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32BE, SAudioFormat::Format_PCM_S32LE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U32LE, SAudioFormat::Format_PCM_U32BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U32BE, SAudioFormat::Format_PCM_U32LE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F32LE, SAudioFormat::Format_PCM_F32BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F32BE, SAudioFormat::Format_PCM_F32LE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F64LE, SAudioFormat::Format_PCM_F64BE>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F64BE, SAudioFormat::Format_PCM_F64LE>;


SAudioBuffer AudioFormatConverter_Format_PCM_S8_Format_PCM_S16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S8))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const qint8 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U8_Format_PCM_U16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U8))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint16 *>(destBuffer.data()),
        reinterpret_cast<const quint8 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16_Format_PCM_S8::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S8,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint8 *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint8));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U16_Format_PCM_U8::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U8,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint8 *>(destBuffer.data()),
        reinterpret_cast<const quint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint8));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16_Format_PCM_S32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U16_Format_PCM_U32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint32 *>(destBuffer.data()),
        reinterpret_cast<const quint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32_Format_PCM_S16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U32_Format_PCM_U16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint16 *>(destBuffer.data()),
        reinterpret_cast<const quint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16_Format_PCM_F32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<float *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(float));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F32_Format_PCM_S16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const float *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32_Format_PCM_F32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<float *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(float));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F32_Format_PCM_S32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const float *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16_Format_PCM_F64::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F64,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<double *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(double));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F64_Format_PCM_S16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F64))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const double *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32_Format_PCM_F64::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_F64,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<double *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(double));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_F64_Format_PCM_S32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_F64))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const double *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S8, SAudioFormat::Format_PCM_S16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U8, SAudioFormat::Format_PCM_U16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S8>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U8>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_S32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_U32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_S16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_U16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F32, SAudioFormat::Format_PCM_S32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_F64>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_F64>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_F64, SAudioFormat::Format_PCM_S32>;


SAudioBuffer AudioFormatConverter_Format_PCM_U16_Format_PCM_S16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint16 *>(destBuffer.data()),
        reinterpret_cast<const quint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S16_Format_PCM_U16::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U16,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint16 *>(destBuffer.data()),
        reinterpret_cast<const qint16 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint16));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_U32_Format_PCM_S32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_U32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<qint32 *>(destBuffer.data()),
        reinterpret_cast<const quint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(qint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

SAudioBuffer AudioFormatConverter_Format_PCM_S32_Format_PCM_U32::convertBuffer(const SAudioBuffer &audioBuffer)
{
  if ((audioBuffer.format() == SAudioFormat::Format_PCM_S32))
  {
    SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_U32,
                                         audioBuffer.format().channelSetup(),
                                         audioBuffer.format().sampleRate()),
                            audioBuffer.numSamples());

    Algorithms::AudioConvert::convert(
        reinterpret_cast<quint32 *>(destBuffer.data()),
        reinterpret_cast<const qint32 *>(audioBuffer.data()),
        destBuffer.size() / sizeof(quint32));

    destBuffer.setTimeStamp(audioBuffer.timeStamp());

    return destBuffer;
  }

  return SAudioBuffer();
}

template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U16, SAudioFormat::Format_PCM_S16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S16, SAudioFormat::Format_PCM_U16>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_U32, SAudioFormat::Format_PCM_S32>;
template class AudioFormatConverterBase<SAudioFormat::Format_PCM_S32, SAudioFormat::Format_PCM_U32>;

} } // End of namespaces
