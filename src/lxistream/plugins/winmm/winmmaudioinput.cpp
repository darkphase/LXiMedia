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

#include "winmmaudioinput.h"

namespace LXiStreamDevice {
namespace WinMMBackend {

WinMMAudioInput::WinMMAudioInput(const QString &, QObject *parent)
  : SInterfaces::AudioInput(parent),
    waveIn(NULL),
    inFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 48000)
{
}

WinMMAudioInput::~WinMMAudioInput()
{
  WinMMAudioInput::stop();
}

void WinMMAudioInput::setFormat(const SAudioFormat &f)
{
  inFormat = f;
}

SAudioFormat WinMMAudioInput::format(void)
{
  return inFormat;
}

bool WinMMAudioInput::start(void)
{
  WAVEFORMATEX format;
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = inFormat.numChannels();
  format.nSamplesPerSec = inFormat.sampleRate();
  format.nAvgBytesPerSec = format.nChannels * format.nSamplesPerSec * inFormat.sampleSize();
  format.nBlockAlign = format.nChannels * inFormat.sampleSize();
  format.wBitsPerSample = inFormat.sampleSize() * 8;
  format.cbSize = sizeof(format);

  MMRESULT result = ::waveInOpen(
        &waveIn,
        WAVE_MAPPER,
        &format,
        NULL, NULL,
        CALLBACK_NULL | WAVE_FORMAT_DIRECT);

  if (result == MMSYSERR_NOERROR)
  {
    queueHeaders();

    if (waveInStart(waveIn) == MMSYSERR_NOERROR)
      return true;

    ::waveInClose(waveIn);
  }

  waveIn = NULL;

  WCHAR fault[256];
  ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));
  qWarning() << "WinMMAudioInput:" << QString::fromWCharArray(fault);

  return false;
}

void WinMMAudioInput::stop(void)
{
  if (waveIn != NULL)
  {
    ::waveInStop(waveIn);

    flushHeaders();

    ::waveInClose(waveIn);
    waveIn = NULL;
  }
}

bool WinMMAudioInput::process(void)
{
  if (waveIn != NULL)
  {
    while (!headers.isEmpty() && ((headers.front().first->dwFlags & WHDR_DONE) != 0))
    {
      if (::waveInUnprepareHeader(waveIn, headers.front().first, sizeof(*(headers.front().first))) == MMSYSERR_NOERROR)
      {
        SAudioBuffer buffer = headers.front().second;
        delete headers.front().first;
        headers.dequeue();

        buffer.setTimeStamp(timer.smoothTimeStamp(buffer.duration()));
        emit produce(buffer);

        continue;
      }

      break;
    }

    queueHeaders();
  }

  return false;
}

void WinMMAudioInput::queueHeaders(void)
{
  STime delay;
  for (QQueue< QPair<WAVEHDR *, SAudioBuffer> >::Iterator i=headers.begin(); i!=headers.end(); i++)
    delay += i->second.duration();

  while (delay.toMSec() < maxDelay)
  {
    SAudioBuffer buffer(inFormat);
    buffer.setNumSamples(inFormat.sampleRate() / 25);

    WAVEHDR *waveHdr = new WAVEHDR();
    waveHdr->lpData = (CHAR *)buffer.data();
    waveHdr->dwBufferLength = buffer.size();
    waveHdr->dwBytesRecorded = 0;
    waveHdr->dwUser = 0;
    waveHdr->dwFlags = 0;
    waveHdr->dwLoops = 0;

    if (::waveInPrepareHeader(waveIn, waveHdr, sizeof(*waveHdr)) == MMSYSERR_NOERROR)
    {
      if (::waveInAddBuffer(waveIn, waveHdr, sizeof(*waveHdr)) == MMSYSERR_NOERROR)
      {
        headers.enqueue(QPair<WAVEHDR *, SAudioBuffer>(waveHdr, buffer));
        delay += buffer.duration();

        continue;
      }

      ::waveInUnprepareHeader(waveIn, waveHdr, sizeof(*waveHdr));
    }

    delete waveHdr;

    break;
  }
}

void WinMMAudioInput::flushHeaders(void)
{
  while (headers.count() > 0)
  {
    QPair<WAVEHDR *, SAudioBuffer> header = headers.head();

    if ((header.first->dwFlags & WHDR_DONE) != 0)
    if (::waveInUnprepareHeader(waveIn, header.first, sizeof(*(header.first))) == MMSYSERR_NOERROR)
    {
      delete header.first;
      headers.dequeue();
      continue;
    }

    break;
  }
}

} } // End of namespaces
