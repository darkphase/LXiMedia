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

#include "alsaoutput.h"
#include "module.h"

namespace LXiStreamDevice {
namespace AlsaBackend {

AlsaOutput::AlsaOutput(const QString &dev, QObject *parent)
  : SInterfaces::AudioOutput(parent),
    dev(deviceName(dev)),
    pcm(NULL),
    resampler(NULL),
    outLatency(STime::null),
    lastFormat(),
    outFormat()
{
}

AlsaOutput::~AlsaOutput()
{
  if (pcm)
    snd_pcm_close(pcm);

  delete resampler;
}

bool AlsaOutput::start(void)
{
  return true;
}

void AlsaOutput::stop(void)
{
  if (pcm != NULL)
  {
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
    pcm = NULL;
  }

  delete resampler;
  resampler = NULL;

  lastFormat = SAudioFormat();
}

STime AlsaOutput::latency(void) const
{
  return outLatency;
}

void AlsaOutput::consume(const SAudioBuffer &audioBuffer)
{
  if (pcm)
  {
    // Determine latency
    snd_pcm_sframes_t delay = 0;
    if (snd_pcm_delay(pcm, &delay) == 0)
      outLatency = STime::fromClock(delay, outFormat.sampleRate());
  }

  if (!audioBuffer.isNull())
  {
    if ((lastFormat != audioBuffer.format()) || (pcm == NULL))
      openFormat(audioBuffer.format());

    if (pcm)
    {
      SAudioBuffer buffer = audioBuffer;
      if (resampler)
        buffer = resampler->processBuffer(buffer);

      const unsigned packetSize = (buffer.format().sampleSize() * buffer.format().numChannels());
      const qint16 * const data = reinterpret_cast<const qint16 *>(buffer.data());
      const size_t dataSize = packetSize ? (buffer.size() / packetSize) : 0;

      if (dataSize > 0)
      {
        for (int i=0, err=-1; (i<3)&&(err<0); i++)
        if ((err = snd_pcm_writei(pcm, data, dataSize)) < 0)
        {
          if (err == -EBADFD)
            qWarning() << "AlsaOutput: PCM is not in the right state.";
          else if (err == -EPIPE)
            qDebug() << "AlsaOutput: Buffer underrun occurred.";
          else if (err == -ESTRPIPE)
            qDebug() << "AlsaOutput: A suspend event occurred.";
          else
            qWarning() << "AlsaOutput: An unknown error " << err << " occurred.";

          snd_pcm_prepare(pcm);
        }
      }
    }
  }
}

void AlsaOutput::openFormat(const SAudioFormat &reqFormat)
{
  if (pcm)
  {
    snd_pcm_close(pcm);
    pcm = NULL;
  }

  delete resampler;
  resampler = NULL;

  lastFormat = reqFormat;

  snd_pcm_format_t format = toALSA(reqFormat);
  snd_pcm_uframes_t bufferSize = 0;
  unsigned sampleRate = reqFormat.sampleRate();
  unsigned numChannels = reqFormat.numChannels();

  // Open correct audio device
  /*if ((reqCodec == "AC3") || (reqCodec == "DTS"))
  { // Open iec958 passthrough device
    if (!dev.startsWith("iec958:"))
    {
      qWarning() << "AlsaOutput: Can only passthrough AC3 or DTS over IEC958 (S/PDIF).";
      return;
    }

    const quint8 aes0 = ((reqCodec == "AC3") ? IEC958_AES0_NONAUDIO : 0) | IEC958_AES0_CON_NOT_COPYRIGHT;
    const quint8 aes1 = IEC958_AES1_CON_DIGDIGCONV_OTHER;//0x82;
    const quint8 aes2 = IEC958_AES2_CON_SOURCE_UNSPEC | IEC958_AES2_CON_CHANNEL_UNSPEC;
    const quint8 aes3 = ((sampleRate == 32000)
                                  ? IEC958_AES3_CON_FS_32000
                                  : ((sampleRate == 44100)
                                      ? IEC958_AES3_CON_FS_44100
                                      : IEC958_AES3_CON_FS_48000));
    const QString iecdev = dev +
                           ",AES0=" + QString::number(aes0) +
                           ",AES1=" + QString::number(aes1) +
                           ",AES2=" + QString::number(aes2) +
                           ",AES3=" + QString::number(aes3);

    if (snd_pcm_open(&pcm, iecdev.toLatin1().data(), SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
      qWarning() << "AlsaOutput: Failed to open device " << iecdev;
      return; // Unable to play
    }

    format = SND_PCM_FORMAT_S16_LE;
    sampleRate = 48000; // AC3 bitrate is always equal to 48000 Hz samplerate
    numChannels = 2;
    bufferSize = (maxDelay * 48000) / 1000;
  }
  else*/ if ((format != SND_PCM_FORMAT_UNKNOWN) && (sampleRate > 0))
  {
    if (snd_pcm_open(&pcm, dev.toLatin1().data(), SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
      qWarning() << "AlsaOutput: Failed to open device " << dev;
      return; // Unable to play
    }

    bufferSize = (maxDelay * sampleRate) / 1000;
  }
  else
  {
    qWarning() << "AlsaOutput: Format not supported for playback or passthrough " << reqFormat.formatName();
    return; // Unable to play
  }

  // Set appropriate HW parameters
  snd_pcm_hw_params_t *hw_params = NULL;

  if (snd_pcm_hw_params_malloc(&hw_params) == 0)
  if (snd_pcm_hw_params_any(pcm, hw_params) == 0)
  if (snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) == 0)
  if (snd_pcm_hw_params_set_format(pcm, hw_params, format) == 0)
  if (snd_pcm_hw_params_set_rate_resample(pcm, hw_params, 1) == 0)
  if (snd_pcm_hw_params_set_rate_near(pcm, hw_params, &sampleRate, 0) == 0)
  if (snd_pcm_hw_params_set_channels(pcm, hw_params, numChannels) == 0)
  if (snd_pcm_hw_params_set_buffer_size_near(pcm, hw_params, &bufferSize) == 0)
  if (snd_pcm_hw_params(pcm, hw_params) == 0)
  if (snd_pcm_prepare(pcm) == 0)
  {
    if ((numChannels != reqFormat.numChannels()) || (sampleRate != reqFormat.sampleRate()))
    {
      resampler = ::LXiStream::SInterfaces::AudioResampler::create(this, QString::null);
      if (resampler)
        resampler->setSampleRate(sampleRate);
    }
  }

  if (hw_params)
      snd_pcm_hw_params_free(hw_params);
}

snd_pcm_format_t AlsaOutput::toALSA(SAudioFormat::Format format)
{
  switch (format)
  {
  case SAudioFormat::Format_PCM_S16LE:  return SND_PCM_FORMAT_S16_LE;
  case SAudioFormat::Format_PCM_S16BE:  return SND_PCM_FORMAT_S16_BE;
  case SAudioFormat::Format_PCM_U16LE:  return SND_PCM_FORMAT_U16_LE;
  case SAudioFormat::Format_PCM_U16BE:  return SND_PCM_FORMAT_U16_BE;
  case SAudioFormat::Format_PCM_S8:     return SND_PCM_FORMAT_S8;
  case SAudioFormat::Format_PCM_U8:     return SND_PCM_FORMAT_U8;
  case SAudioFormat::Format_PCM_MULAW:  return SND_PCM_FORMAT_MU_LAW;
  case SAudioFormat::Format_PCM_ALAW:   return SND_PCM_FORMAT_A_LAW;
  case SAudioFormat::Format_PCM_S32LE:  return SND_PCM_FORMAT_S32_LE;
  case SAudioFormat::Format_PCM_S32BE:  return SND_PCM_FORMAT_S32_BE;
  case SAudioFormat::Format_PCM_U32LE:  return SND_PCM_FORMAT_U32_LE;
  case SAudioFormat::Format_PCM_U32BE:  return SND_PCM_FORMAT_U32_BE;
  case SAudioFormat::Format_PCM_S24LE:  return SND_PCM_FORMAT_S24_LE;
  case SAudioFormat::Format_PCM_S24BE:  return SND_PCM_FORMAT_S24_BE;
  case SAudioFormat::Format_PCM_U24LE:  return SND_PCM_FORMAT_U24_LE;
  case SAudioFormat::Format_PCM_U24BE:  return SND_PCM_FORMAT_U24_BE;
  default:                              return SND_PCM_FORMAT_UNKNOWN;
  }
}

} } // End of namespaces
