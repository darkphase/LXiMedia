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

#include "v4l1device.h"
#include "v4l1input.h"
#include "vbiinput.h"

namespace LXiStream {
namespace V4lBackend {


QMap<QString, int> V4l1Device::videoDevices;


SSystem::DeviceEntryList V4l1Device::listDevices(void)
{
  videoDevices.clear();
  SSystem::DeviceEntryList result;

  for (unsigned i=0; i<=99; i++)
  {
    int fd = ::open(("/dev/video" + QString::number(i)).toLatin1(), O_RDONLY);
    if (fd < 0)
      break; // last device

    video_capability capabilities;
    memset(&capabilities, 0, sizeof(capabilities));
    v4l2_capability capabilitiesV2;
    memset(&capabilitiesV2, 0, sizeof(capabilitiesV2));

    // Don't show devices that also support V4L2.
    if(ioctl(fd, VIDIOC_QUERYCAP, &capabilitiesV2) < 0)
    if(ioctl(fd, VIDIOCGCAP, &capabilities) >= 0)
    {
      const QString url = "lx-v4l1://" + SStringParser::toRawName((const char *)capabilities.name).toLower();

      videoDevices[url] = i;
      result += SSystem::DeviceEntry(2, (const char *)capabilities.name, url);
    }

    close(fd);
  }

  return result;
}

V4l1Device::V4l1Device(QObject *parent)
           :STerminals::AudioVideoDevice(parent),
            videoDev(NULL),
            vbiDev(NULL)
{
}

V4l1Device::~V4l1Device()
{
}

bool V4l1Device::open(const QUrl &url)
{
  const QString name = "lx-v4l1://" + url.host().toLower();

  if (videoDevices.contains(name))
  {
    const int i = videoDevices[name];

    videoDev = new V4l1Input("/dev/video" + QString::number(i), this);

    if (QFileInfo("/dev/vbi" + QString::number(i)).isReadable())
      vbiDev = new VBIInput("/dev/vbi" + QString::number(i), this);

    return true;
  }

  qWarning() << "V4l1Device: Specified device was not found: " << name;
  return false;
}

QString V4l1Device::friendlyName(void) const
{
  if (videoDev)
    return videoDev->friendlyName();
  else
    return QString::null;
}

QString V4l1Device::longName(void) const
{
  if (videoDev)
    return videoDev->longName();
  else
    return QString::null;
}

STerminal::Types V4l1Device::terminalType(void) const
{
  if (openAudioDevice)
    return Type_AnalogTelevision | Type_AnalogRadio;
  else
    return Type_AnalogTelevision;
}

QStringList V4l1Device::inputs(void) const
{
  return QStringList() << "0";
}

bool V4l1Device::selectInput(const QString &name)
{
  if (videoDev)
    return videoDev->selectInput(name.toInt());
  else
    return false;
}

STuner * V4l1Device::tuner(void) const
{
  return NULL;
}

QList<STerminal::Stream> V4l1Device::inputStreams(void) const
{
  Stream stream;
  stream.name = "Capture";
  stream.videoPacketIDs += 0;

  return QList<Stream>() << stream;
}

QList<STerminal::Stream> V4l1Device::outputStreams(void) const
{
  return QList<Stream>();
}

SNode * V4l1Device::openStream(const Stream &)
{
  if (videoDev)
  {
    SMuxNode * const muxNode = new SMuxNode(this);
    if (openVideoDevice)
      muxNode->addNode(videoDev);

    if (vbiDev)
      muxNode->addNode(vbiDev);

    // Now look for the audio device
    if (openAudioDevice && (audioNode == NULL))
      findAudioDevice(longName());

    if (audioNode)
      muxNode->addNode(audioNode);

    return muxNode;
  }

  return NULL;
}

quint16 V4l1Device::toV4L(SVideoCodec::Format format)
{
  switch (format)
  {
  case SVideoCodec::Format_GRAY8:          return VIDEO_PALETTE_GREY;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  case SVideoCodec::Format_RGB555:         return VIDEO_PALETTE_RGB555;
  case SVideoCodec::Format_RGB565:         return VIDEO_PALETTE_RGB565;
  case SVideoCodec::Format_RGB24:          return VIDEO_PALETTE_RGB24;
  case SVideoCodec::Format_RGB32:          return VIDEO_PALETTE_RGB32;
#else // RGB and BGR are deliberately swapped here.
  case SVideoCodec::Format_BGR24:          return VIDEO_PALETTE_RGB24;
  case SVideoCodec::Format_BGR32:          return VIDEO_PALETTE_RGB32;
#endif
  case SVideoCodec::Format_YUYV422:        return VIDEO_PALETTE_YUYV;
  case SVideoCodec::Format_UYVY422:        return VIDEO_PALETTE_UYVY;
  case SVideoCodec::Format_YUV411P:        return VIDEO_PALETTE_YUV411P;
  case SVideoCodec::Format_YUV420P:        return VIDEO_PALETTE_YUV420P;
  case SVideoCodec::Format_YUV422P:        return VIDEO_PALETTE_YUV422P;
  default:                                 return 0;
  }
}


} } // End of namespaces
