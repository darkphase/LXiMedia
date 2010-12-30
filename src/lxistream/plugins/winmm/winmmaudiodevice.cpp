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

#include "winmmaudiodevice.h"
#include "winmmaudioinput.h"
#include "winmmaudiooutput.h"


namespace LXiStream {
namespace WinMMBackend {


QMap<QString, int>            WinMMAudioDevice::foundDevices;
QMutex                        WinMMAudioDevice::mutex(QMutex::Recursive);
QMap<int, WinMMAudioOutput *> WinMMAudioDevice::outputs;


SSystem::DeviceEntryList WinMMAudioDevice::listDevices(void)
{
  SSystem::DeviceEntryList result;

  const QString url = "lx-winmmaudio://dev" + QString::number(int(WAVE_MAPPER));
  result += SSystem::DeviceEntry(-1, "Windows wave mapper", url);
  foundDevices[url] = int(WAVE_MAPPER);

  const unsigned numDevices = ::waveOutGetNumDevs();
  for (unsigned i=0; i<numDevices; i++)
  {
    WAVEOUTCAPS caps;
    if (waveOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
    if (caps.szPname != NULL)
    {
      const QString url = "lx-winmmaudio://dev" + QString::number(i);
      result += SSystem::DeviceEntry(0, QString::fromWCharArray(caps.szPname), url);
      foundDevices[url] = i;
    }
  }

  return result;
}

WinMMAudioDevice::WinMMAudioDevice(QObject *parent)
  : STerminals::AudioDevice(parent)
{
}

WinMMAudioDevice::~WinMMAudioDevice()
{
  foreach (WinMMAudioOutput *output, openedExclusive)
  if (output->nodeCount == -1)
    output->nodeCount = 0;
}

bool WinMMAudioDevice::open(const QUrl &url)
{
  const QString name = "lx-winmmaudio://" + url.host().toLower();

  if (foundDevices.contains(name))
  {
    dev = foundDevices[name];
    return true;
  }

  qWarning() << "AlsaDevice: Specified device was not found: " << name;
  return false;
}

QString WinMMAudioDevice::friendlyName(void) const
{
  if (dev == int(WAVE_MAPPER))
    return "Windows wave mapper";

  WAVEOUTCAPS caps;
  if (waveOutGetDevCaps(dev, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
  if (caps.szPname != NULL)
    return QString::fromWCharArray(caps.szPname);

  return QString::null;
}

QString WinMMAudioDevice::longName(void) const
{
  return QString::number(dev) + ":" + WinMMAudioDevice::friendlyName();
}

STerminal::Types WinMMAudioDevice::terminalType(void) const
{
  return Type_DigitalAudio | Type_AnalogAudio;
}

QList<STerminal::Stream> WinMMAudioDevice::inputStreams(void) const
{
  Stream stream;
  stream.name = "Capture";
  stream.audioPacketIDs += 0;
  stream.opaque = -1;

  return QList<Stream>() << stream;
}

QList<STerminal::Stream> WinMMAudioDevice::outputStreams(void) const
{
  Stream stream;
  stream.name = "Playback";
  stream.opaque = 1;

  Stream exclusive;
  exclusive.name = "Exclusive";
  exclusive.opaque = 2;

  return QList<Stream>() << stream << exclusive;
}

SNode * WinMMAudioDevice::openStream(const Stream &stream)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (stream.opaque.toInt() == -1)
  {
    return new WinMMAudioInput(dev, this);
  }
  else if (stream.opaque.toInt() == 1)
  {
    QMap<int, WinMMAudioOutput *>::Iterator i = outputs.find(dev);
    if (i != outputs.end())
    {
      if ((*i)->nodeCount >= 0)
      {
        SNode * const node = (*i)->createNode();
        node->setParent(this);
        return node;
      }
      else
        return NULL; // Is opened in exclusive mode.
    }

    WinMMAudioOutput * const output = new WinMMAudioOutput(dev, false, NULL); // No parent, should not be deleted automatically
    outputs[dev] = output;

    SNode * const node = output->createNode();
    node->setParent(this);
    return node;
  }
  else if (stream.opaque.toInt() == 2)
  {
    QMap<int, WinMMAudioOutput *>::Iterator i = outputs.find(dev);
    if (i != outputs.end())
    {
      if ((*i)->nodeCount == 0)
      {
        openedExclusive += *i;

        (*i)->nodeCount = -1; // Exclusive
        SMuxNode * const node = new SMuxNode(this);
        node->addNode(*i);
        return node;
      }
      else
        return NULL;  // Is already opened in exclusive mode or has nodes.
    }

    WinMMAudioOutput * const output = new WinMMAudioOutput(dev, false, NULL); // No parent, should not be deleted automatically
    outputs[dev] = output;
    openedExclusive += output;

    output->nodeCount = -1; // Exclusive
    SMuxNode * const node = new SMuxNode(this);
    node->addNode(output);

    return node;
  }

  return NULL;
}


} } // End of namespaces
