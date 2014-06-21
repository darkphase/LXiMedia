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

#include "winmmaudiooutput.h"

// Missing defines in MinGW
#ifndef WAVE_FORMAT_DOLBY_AC3_SPDIF
  #define  WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092
#endif

#ifndef WAVE_FORMAT_44S16
  #define  WAVE_FORMAT_44S16 0x00000800
#endif

#ifndef WAVE_FORMAT_48S16
  #define  WAVE_FORMAT_48S16 0x00008000
#endif

namespace LXiStreamDevice {
namespace WinMMBackend {

WinMMAudioOutput::WinMMAudioOutput(const QString &, QObject *parent)
  : SInterfaces::AudioOutput(parent),
    waveOut(NULL),
    errorProduced(false),
    outLatency(STime::null),
    inFormat(),
    headers()
{
}

WinMMAudioOutput::~WinMMAudioOutput()
{
  WinMMAudioOutput::stop();
}

bool WinMMAudioOutput::start(void)
{
  errorProduced = false;
  outLatency = STime::null;

  return true;
}

void WinMMAudioOutput::stop(void)
{
  if (waveOut != NULL)
  {
    while (!headers.isEmpty())
      flushHeaders();

    ::waveOutClose(waveOut);
  }
}

STime WinMMAudioOutput::latency(void) const
{
  return outLatency;
}

void WinMMAudioOutput::consume(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  if ((inFormat != audioBuffer.format()) || (waveOut == NULL))
    openCodec(audioBuffer.format());

  if (waveOut && !audioBuffer.isNull())
  {
    const qint16 * const data = reinterpret_cast<const qint16 *>(audioBuffer.data());
    const size_t dataSize = audioBuffer.size();

    writeHeader(data, dataSize, audioBuffer);
  }
}

void WinMMAudioOutput::openCodec(const SAudioFormat &reqFormat)
{
  if (waveOut != NULL)
  {
    ::waveOutClose(waveOut);
    waveOut = NULL;
  }

  if (!reqFormat.isNull())
  {
    // Determine capabilities
    WAVEOUTCAPS caps;
    if (::waveOutGetDevCaps(WAVE_MAPPER, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
    {
//      if (reqFormat.format() == SAudioFormat::Format_AC3)
//      {
//        // Create a packet with silence to use when no data is arriving
//        silentBuffer = createAC3SilentPacket(codec().channelSetup());

//        WAVEFORMATEX format;
//        format.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
//        format.nChannels = 2;
//        format.nSamplesPerSec = 48000;
//        format.nAvgBytesPerSec = format.nChannels * format.nSamplesPerSec * sizeof(qint16);
//        format.nBlockAlign = 4;
//        format.wBitsPerSample = sizeof(qint16) * 8;
//        format.cbSize = sizeof(format);

//        MMRESULT result = ::waveOutOpen(&waveOut,
//                                        dev,
//                                        &format,
//                                        NULL, NULL,
//                                        WAVE_FORMAT_DIRECT);

//        if (result == MMSYSERR_NOERROR)
//          return true;
//        else if (!errorProduced)
//        {
//          WCHAR fault[256];
//          ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));

//          qWarning() << "AC3:" << QString::fromWCharArray(fault) <<
//                     "Formats:" << QString::number(caps.dwFormats, 16);

//          errorProduced = true;
//          return false;
//        }
//      }
      /*else*/ if (reqFormat.format() == SAudioFormat::Format_PCM_S16LE)
      {
        WAVEFORMATEX format;
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = reqFormat.numChannels();
        format.nSamplesPerSec = reqFormat.sampleRate();
        format.nAvgBytesPerSec = format.nChannels * format.nSamplesPerSec * reqFormat.sampleSize();
        format.nBlockAlign = format.nChannels * reqFormat.sampleSize();
        format.wBitsPerSample = reqFormat.sampleSize() * 8;
        format.cbSize = sizeof(format);

        MMRESULT result = ::waveOutOpen(
              &waveOut,
              WAVE_MAPPER,
              &format,
              0, 0,
              WAVE_FORMAT_DIRECT);

        if (result != MMSYSERR_NOERROR)
        {
          waveOut = NULL;

          if (!errorProduced)
          {
            WCHAR fault[256];
            ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));

            qWarning() << "WinMMAudioOutput:" << QString::fromWCharArray(fault) <<
                       "Formats:" << QString::number(caps.dwFormats, 16);

            errorProduced = true;
          }
        }
      }
    }
  }
}

void WinMMAudioOutput::writeHeader(const qint16 *data, size_t size, const SAudioBuffer &buffer)
{
  flushHeaders();

  WAVEHDR *waveHdr = new WAVEHDR();
  waveHdr->lpData = (CHAR *)data;
  waveHdr->dwBufferLength = size;
  waveHdr->dwBytesRecorded = 0;
  waveHdr->dwUser = 0;
  waveHdr->dwFlags = 0;
  waveHdr->dwLoops = 0;

  if (::waveOutPrepareHeader(waveOut, waveHdr, sizeof(*waveHdr)) == MMSYSERR_NOERROR)
  {
    headers.enqueue(QPair<WAVEHDR *, SAudioBuffer>(waveHdr, buffer));

    if (::waveOutWrite(waveOut, waveHdr, sizeof(*waveHdr)) == MMSYSERR_NOERROR)
    {
      outLatency += buffer.duration();
      if (outLatency.toMSec() > int(maxDelay))
        ::SleepEx(outLatency.toMSec() - maxDelay, TRUE);

      return;
    }

    ::waveOutUnprepareHeader(waveOut, waveHdr, sizeof(*waveHdr));
  }

  delete waveHdr;
}

void WinMMAudioOutput::flushHeaders(void)
{
  while (headers.count() > 0)
  {
    QPair<WAVEHDR *, SAudioBuffer> header = headers.head();

    if ((header.first->dwFlags & WHDR_DONE) != 0)
    if (::waveOutUnprepareHeader(waveOut, header.first, sizeof(*(header.first))) != WAVERR_STILLPLAYING)
    {
      delete header.first;

      if (!header.second.isNull())
        outLatency -= header.second.duration();

      headers.dequeue();

      continue;
    }

    break;
  }
}

} } // End of namespaces
