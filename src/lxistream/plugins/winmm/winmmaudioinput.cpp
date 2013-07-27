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

// Not defined by MinGW
#ifndef WAVE_FORMAT_96M08
#define WAVE_FORMAT_96M08 0x00010000
#endif
#ifndef WAVE_FORMAT_96S08
#define WAVE_FORMAT_96S08 0x00020000
#endif
#ifndef WAVE_FORMAT_96M16
#define WAVE_FORMAT_96M16 0x00040000
#endif
#ifndef WAVE_FORMAT_96S16
#define WAVE_FORMAT_96S16 0x00080000
#endif

namespace LXiStreamDevice {
namespace WinMMBackend {

QMap<QString, WinMMAudioInput::Device> WinMMAudioInput::deviceMap;

QList<SFactory::Scheme> WinMMAudioInput::listDevices(void)
{
  QList<SFactory::Scheme> result;

  deviceMap.clear();

  const UINT count = ::waveInGetNumDevs();
  for (UINT i=0; i<count; i++)
  {
    WAVEINCAPS caps;
    if (::waveInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
    {
      HMIXER mixer = NULL;
      if (::mixerOpen(&mixer, i, 0, 0, MIXER_OBJECTF_WAVEIN) == MMSYSERR_NOERROR)
      {
        MIXERLINE mixerLine;
        memset(&mixerLine, 0, sizeof(mixerLine));

        mixerLine.cbStruct = sizeof(mixerLine);
        mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
        if (::mixerGetLineInfo(HMIXEROBJ(mixer), &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
        for (unsigned j=0, dc=0, n=mixerLine.cConnections; j<n; j++)
        {
          mixerLine.dwSource = j;
          if (::mixerGetLineInfo(HMIXEROBJ(mixer), &mixerLine, MIXER_GETLINEINFOF_SOURCE) == MMSYSERR_NOERROR)
          {
            QString name = QString::fromWCharArray(mixerLine.szName);
            bool isDesktop = false;

            if ((name.simplified().compare("Stereo Mix", Qt::CaseInsensitive) == 0) ||
                (name.simplified().compare("Waveout Mix", Qt::CaseInsensitive) == 0) ||
                (name.simplified().compare("Mixed Output", Qt::CaseInsensitive) == 0) ||
                (name.simplified().compare("What You Hear", Qt::CaseInsensitive) == 0))
            {
              name = "Desktop";
              isDesktop = true;
              dc++;
            }

            if (deviceMap.contains(name))
            {
              int idx = 1;
              while (deviceMap.contains(name + ' ' + QString::number(idx)))
                idx++;

              name += ' ' + QString::number(idx);
            }

            result += SFactory::Scheme((isDesktop && (dc == 1)) ? 0 : -1, name);
            deviceMap.insert(name, Device(i, j, mixerLine.dwLineID));
          }
        }

        ::mixerClose(mixer);
      }
    }
  }

  return result;
}

WinMMAudioInput::WinMMAudioInput(const QString &dev, QObject *parent)
  : SInterfaces::AudioInput(parent),
    device(deviceMap.contains(dev) ? deviceMap[dev] : Device(WAVE_MAPPER)),
    waveIn(NULL),
    inFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 44100)
{
}

WinMMAudioInput::~WinMMAudioInput()
{
  WinMMAudioInput::stop();
}

void WinMMAudioInput::setFormat(const SAudioFormat &f)
{
  inFormat = f;

  correctFormat();
}

SAudioFormat WinMMAudioInput::format(void)
{
  return inFormat;
}

bool WinMMAudioInput::start(void)
{
  correctFormat();

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
        device.id,
        &format,
        0, 0,
        CALLBACK_NULL | WAVE_FORMAT_DIRECT);

  if (result == MMSYSERR_NOERROR)
  {
    HMIXER mixer = NULL;
    if (::mixerOpen(&mixer, device.id, 0, 0, MIXER_OBJECTF_WAVEIN) == MMSYSERR_NOERROR)
    {
      selectSource(mixer, device.source);
      setVolume(mixer, device.lineId, 65535);

      ::mixerClose(mixer);
    }

    queueHeaders();

    if (waveInStart(waveIn) == MMSYSERR_NOERROR)
      return true;

    ::waveInClose(waveIn);
  }

  waveIn = NULL;

  WCHAR fault[256];
  ::waveOutGetErrorText(result, fault, sizeof(fault) / sizeof(WCHAR));
  qWarning() << "WinMMAudioInput: " << QString::fromWCharArray(fault);

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

bool WinMMAudioInput::setVolume(HMIXER mixer, DWORD lineId, uint16_t volume)
{
  MIXERLINECONTROLS lineControls;
  memset(&lineControls, 0, sizeof(lineControls));
  lineControls.cbStruct = sizeof(lineControls);
  lineControls.dwLineID = lineId;
  lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

  MIXERCONTROL control;
  memset(&control, 0, sizeof(control));
  control.cbStruct = sizeof(control);
  lineControls.cbmxctrl = sizeof(control);
  lineControls.pamxctrl = &control;

  if (::mixerGetLineControls(HMIXEROBJ(mixer), &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
  {
    MIXERCONTROLDETAILS details;
    memset(&details, 0, sizeof(details));
    details.cbStruct = sizeof(details);
    details.dwControlID = control.dwControlID;
    details.cChannels = 1;
    details.cMultipleItems = 0;

    MIXERCONTROLDETAILS_UNSIGNED value;
    memset(&value, 0, sizeof(value));
    value.dwValue = volume;
    details.cbDetails = sizeof(value);
    details.paDetails = &value;

    return ::mixerSetControlDetails(HMIXEROBJ(mixer), &details, MIXER_GETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR;
  }

  return false;
}

bool WinMMAudioInput::selectSource(HMIXER mixer, DWORD source)
{
  MIXERLINE mixerLine;
  memset(&mixerLine, 0, sizeof(mixerLine));
  mixerLine.cbStruct = sizeof(mixerLine);
  mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

  if (::mixerGetLineInfo(HMIXEROBJ(mixer), &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
  {
    MIXERLINECONTROLS lineControls;
    memset(&lineControls, 0, sizeof(lineControls));
    lineControls.cbStruct = sizeof(lineControls);
    lineControls.dwLineID = mixerLine.dwLineID;
    lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
    lineControls.cControls = 1;

    MIXERCONTROL control;
    memset(&control, 0, sizeof(control));
    lineControls.cbmxctrl = sizeof(control);
    lineControls.pamxctrl = &control;

    if (::mixerGetLineControls(HMIXEROBJ(mixer), &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
    {
      lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
      if (::mixerGetLineControls(HMIXEROBJ(mixer), &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
        return false;
    }

    MIXERCONTROLDETAILS details;
    memset(&details, 0, sizeof(details));
    details.cbStruct = sizeof(details);
    details.dwControlID = control.dwControlID;
    details.cChannels = 1;
    details.cMultipleItems = control.cMultipleItems;

    MIXERCONTROLDETAILS_LISTTEXT value[details.cMultipleItems];
    memset(&value, 0, sizeof(value));
    details.cbDetails = sizeof(*value);
    details.paDetails = &value;

    if (::mixerGetControlDetails(HMIXEROBJ(mixer), &details, MIXER_GETCONTROLDETAILSF_LISTTEXT) == MMSYSERR_NOERROR)
    {
      for (unsigned i=0; i<details.cMultipleItems; i++)
      {
        MIXERLINE line;
        memset(&line, 0, sizeof(line));
        line.cbStruct = sizeof(line);
        line.dwLineID = value[i].dwParam1;

        if (::mixerGetLineInfo(HMIXEROBJ(mixer), &line, MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR)
        if (line.dwSource == source)
        {
          MIXERCONTROLDETAILS_UNSIGNED newval[details.cMultipleItems];
          memset(&newval, 0, sizeof(newval));
          newval[i].dwValue = 1;
          details.cbDetails = sizeof(*newval);
          details.paDetails = &newval;

          if (::mixerSetControlDetails(HMIXEROBJ(mixer), &details, MIXER_SETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR)
            return true;
        }
      }
    }
  }

  return false;
}

void WinMMAudioInput::correctFormat(void)
{
  WAVEINCAPS caps;
  if (::waveInGetDevCaps(device.id, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
  {
    if ((inFormat.sampleSize() == 2) && (inFormat.numChannels() == 2))
    {
      if      ((inFormat.sampleRate() >= 96000) && ((caps.dwFormats & WAVE_FORMAT_96S16) != 0))
        inFormat.setSampleRate(96000);
      else if ((inFormat.sampleRate() >= 44100) && ((caps.dwFormats & WAVE_FORMAT_4S16) != 0))
        inFormat.setSampleRate(44100);
      else if ((inFormat.sampleRate() >= 22050) && ((caps.dwFormats & WAVE_FORMAT_2S16) != 0))
        inFormat.setSampleRate(22050);
      else if ((inFormat.sampleRate() >= 11025) && ((caps.dwFormats & WAVE_FORMAT_1S16) != 0))
        inFormat.setSampleRate(11025);
    }
    else if ((inFormat.sampleSize() == 2) && (inFormat.numChannels() == 1))
    {
      if      ((inFormat.sampleRate() >= 96000) && ((caps.dwFormats & WAVE_FORMAT_96M16) != 0))
        inFormat.setSampleRate(96000);
      else if ((inFormat.sampleRate() >= 44100) && ((caps.dwFormats & WAVE_FORMAT_4M16) != 0))
        inFormat.setSampleRate(44100);
      else if ((inFormat.sampleRate() >= 22050) && ((caps.dwFormats & WAVE_FORMAT_2M16) != 0))
        inFormat.setSampleRate(22050);
      else if ((inFormat.sampleRate() >= 11025) && ((caps.dwFormats & WAVE_FORMAT_1M16) != 0))
        inFormat.setSampleRate(11025);
    }
    else if ((inFormat.sampleSize() == 1) && (inFormat.numChannels() == 2))
    {
      if      ((inFormat.sampleRate() >= 96000) && ((caps.dwFormats & WAVE_FORMAT_96S08) != 0))
        inFormat.setSampleRate(96000);
      else if ((inFormat.sampleRate() >= 44100) && ((caps.dwFormats & WAVE_FORMAT_4S08) != 0))
        inFormat.setSampleRate(44100);
      else if ((inFormat.sampleRate() >= 22050) && ((caps.dwFormats & WAVE_FORMAT_2S08) != 0))
        inFormat.setSampleRate(22050);
      else if ((inFormat.sampleRate() >= 11025) && ((caps.dwFormats & WAVE_FORMAT_1S08) != 0))
        inFormat.setSampleRate(11025);
    }
    else if ((inFormat.sampleSize() == 1) && (inFormat.numChannels() == 1))
    {
      if      ((inFormat.sampleRate() >= 96000) && ((caps.dwFormats & WAVE_FORMAT_96M08) != 0))
        inFormat.setSampleRate(96000);
      else if ((inFormat.sampleRate() >= 44100) && ((caps.dwFormats & WAVE_FORMAT_4M08) != 0))
        inFormat.setSampleRate(44100);
      else if ((inFormat.sampleRate() >= 22050) && ((caps.dwFormats & WAVE_FORMAT_2M08) != 0))
        inFormat.setSampleRate(22050);
      else if ((inFormat.sampleRate() >= 11025) && ((caps.dwFormats & WAVE_FORMAT_1M08) != 0))
        inFormat.setSampleRate(11025);
    }
  }
}

void WinMMAudioInput::queueHeaders(void)
{
  STime delay;
  for (QQueue< QPair<WAVEHDR *, SAudioBuffer> >::Iterator i=headers.begin(); i!=headers.end(); i++)
    delay += i->second.duration();

  while (delay.toMSec() < maxDelay)
  {
    SAudioBuffer buffer(inFormat, ((inFormat.sampleRate() / 25) / 1024) * 1024);

    WAVEHDR *waveHdr = new WAVEHDR();
    waveHdr->lpData = (CHAR *)buffer.constData();
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
