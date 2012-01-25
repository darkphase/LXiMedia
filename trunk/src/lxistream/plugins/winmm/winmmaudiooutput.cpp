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

#include "winmmaudiooutput.h"
#include "winmmaudiodevice.h"

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

namespace LXiStream {
namespace WinMMBackend {


WinMMAudioOutput::WinMMAudioOutput(int dev, bool iec958, QObject *parent)
  : AudioOutput(iec958, parent),
    nodeCount(0),
    outLatency(STime::fromMSec(0)),
    silentBuffer(SBuffer()),
    lastBuffer(SBuffer()),
    lastBufferRepeats(0),
    outCodec(),
    dev(dev),
    waveOut(NULL),
    errorProduced(false)
{
}

WinMMAudioOutput::~WinMMAudioOutput()
{
  if (waveOut != NULL)
  {
    while (!headers.isEmpty())
      flushHeaders();

    ::waveOutClose(waveOut);
  }
}

Common::AudioMixerNode * WinMMAudioOutput::createNode(void)
{
  return new WinMMAudioOutputNode(this);
}

bool WinMMAudioOutput::prepare(const SCodecList &codecs)
{
  outCodec = SAudioCodec(SAudioCodec::Format_PCM_S16LE, SAudioCodec::Channel_Stereo, 48000);

  foreach (const SAudioCodec &codec, codecs)
  if (((codec.format() == SAudioCodec::Format_PCM_S16LE) &&
       (codec.numChannels() > 0) &&
       (codec.sampleRate() > 0)) ||
      ((codec == "AC3") || (codec == "DTS")))
  {
    outCodec = codec;
    break;
  }

  return AudioOutput::prepare(codecs);
}

bool WinMMAudioOutput::unprepare(void)
{
  if (waveOut != NULL)
  {
    while (!headers.isEmpty())
      flushHeaders();

    ::waveOutClose(waveOut);
    waveOut = NULL;
  }

  return AudioOutput::unprepare();
}

bool WinMMAudioOutput::openCodec(const SAudioCodec &reqCodec)
{
  if (waveOut != NULL)
  {
    ::waveOutClose(waveOut);
    waveOut = NULL;
  }

  if (!reqCodec.isNull())
  {
    // Determine capabilities
    WAVEOUTCAPS caps;
    if (::waveOutGetDevCaps(dev, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
    {
      if (reqCodec == "AC3")
      {
        // Create a packet with silence to use when no data is arriving
        silentBuffer = createAC3SilentPacket(codec().channelSetup());

        WAVEFORMATEX format;
        format.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
        format.nChannels = 2;
        format.nSamplesPerSec = 48000;
        format.nAvgBytesPerSec = format.nChannels * format.nSamplesPerSec * sizeof(qint16);
        format.nBlockAlign = 4;
        format.wBitsPerSample = sizeof(qint16) * 8;
        format.cbSize = sizeof(format);

        MMRESULT result = ::waveOutOpen(&waveOut,
                                        dev,
                                        &format,
                                        NULL, NULL,
                                        WAVE_FORMAT_DIRECT);

        if (result == MMSYSERR_NOERROR)
          return true;
        else if (!errorProduced)
        {
          WCHAR fault[256];
          ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));

          qWarning() << "AC3:" << QString::fromWCharArray(fault) <<
                     "Formats:" << QString::number(caps.dwFormats, 16);

          errorProduced = true;
          return false;
        }
      }
      else if (reqCodec.format() == SAudioCodec::Format_PCM_S16LE)
      {
        silentBuffer = createSilentPacket(codec().channelSetup());

        WAVEFORMATEX format;
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = codec().numChannels();
        format.nSamplesPerSec = 48000;
        format.nAvgBytesPerSec = format.nChannels * format.nSamplesPerSec * codec().sampleSize();
        format.nBlockAlign = format.nChannels * codec().sampleSize();
        format.wBitsPerSample = codec().sampleSize() * 8;
        format.cbSize = sizeof(format);

        MMRESULT result = ::waveOutOpen(&waveOut,
                                        dev,
                                        &format,
                                        NULL, NULL,
                                        WAVE_FORMAT_DIRECT);

        if (result == MMSYSERR_NOERROR)
          return true;
        else if (!errorProduced)
        {
          WCHAR fault[256];
          ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));

          qWarning() << "AC3:" << QString::fromWCharArray(fault) <<
                     "Formats:" << QString::number(caps.dwFormats, 16);

          errorProduced = true;
          return false;
        }
      }
    }
  }

  return false;
}

STime WinMMAudioOutput::latency(void) const
{
  return outLatency;
}

void WinMMAudioOutput::writeAudio(const SAudioBuffer &buffer)
{
  if (!buffer.isNull())
  {
    lastBuffer = buffer;
    lastBufferRepeats = 0;

    if ((outCodec != buffer.codec()) || (waveOut == NULL))
      openCodec(buffer.codec());
  }
  else if (!lastBuffer.isNull())
  {
    if (outLatency > STime::fromMSec(80))
      return; // Enough frames are buffered, we do not have to repeat the last buffer

    else if (++lastBufferRepeats == 3)
      lastBuffer = silentBuffer;

    else if (lastBufferRepeats >= 100)
      lastBuffer = SBuffer(); // Stop playing
  }

  if (waveOut && !lastBuffer.isNull())
  {
    const qint16 * const data = reinterpret_cast<const qint16 *>(lastBuffer.bits());
    const size_t dataSize = lastBuffer.numBytes();

    writeHeader(data, dataSize, lastBuffer);
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
  ::waveOutPrepareHeader(waveOut, waveHdr, sizeof(*waveHdr));
  headers.enqueue(QPair<WAVEHDR *, SAudioBuffer>(waveHdr, buffer));

  ::waveOutWrite(waveOut, waveHdr, sizeof(*waveHdr));

  outLatency += buffer.duration();
  if (outLatency.toMSec() > int(maxDelay))
    ::SleepEx(outLatency.toMSec() - maxDelay, TRUE);
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


WinMMAudioOutputNode::WinMMAudioOutputNode(WinMMAudioOutput *parent)
  : Common::AudioOutputNode(parent),
    parent(parent)
{
  parent->nodeCount++;
}

WinMMAudioOutputNode::~WinMMAudioOutputNode()
{
  parent->nodeCount--;
}


} } // End of namespaces
