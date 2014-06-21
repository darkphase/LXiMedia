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

#include "v4l2device.h"
#include "v4l2input.h"
#include "v4l2tuner.h"
#include "vbiinput.h"

namespace LXiStream {
namespace V4lBackend {


QMap<QString, int> V4l2Device::videoDevices;
QMap<QString, int> V4l2Device::vbiDevices;


SSystem::DeviceEntryList V4l2Device::listDevices(void)
{
  videoDevices.clear();
  vbiDevices.clear();
  SSystem::DeviceEntryList result;

  for (unsigned i=0; i<=31; i++)
  {
    int fd = ::open(("/dev/video" + QString::number(i)).toLatin1(), O_RDONLY);
    if (fd < 0)
      break; // last device

    v4l2_capability capabilities;
    memset(&capabilities, 0, sizeof(capabilities));

    if(ioctl(fd, VIDIOC_QUERYCAP, &capabilities) >= 0)
    {
      const QString url = "lx-v4l2://" + SStringParser::toRawName((const char *)capabilities.bus_info).toLower();

      videoDevices[url] = i;
      result += SSystem::DeviceEntry(1, (const char *)capabilities.card, url);
    }

    close(fd);
  }

  for (unsigned i=0; i<=31; i++)
  {
    int fd = ::open(("/dev/vbi" + QString::number(i)).toLatin1(), O_RDONLY);
    if (fd < 0)
      break; // last device

    v4l2_capability capabilities;
    memset(&capabilities, 0, sizeof(capabilities));

    if(ioctl(fd, VIDIOC_QUERYCAP, &capabilities) >= 0)
      vbiDevices["lx-v4l2://" + SStringParser::toRawName((const char *)capabilities.bus_info).toLower()] = i;

    close(fd);
  }

  return result;
}

V4l2Device::V4l2Device(QObject *parent)
  : STerminals::AudioVideoDevice(parent),
    videoDev(NULL),
    tunerDev(NULL),
    vbiDev(NULL)
{
}

V4l2Device::~V4l2Device()
{
}

bool V4l2Device::open(const QUrl &url)
{
  const QString name = "lx-v4l2://" + url.host().toLower();

  if (videoDevices.contains(name))
  {
    tunerDev = new V4l2Tuner(this);
    videoDev = new V4l2Input("/dev/video" + QString::number(videoDevices[name]), this);
    if (videoDev->hasVbiCapture() && vbiDevices.contains(name))
      vbiDev = new VBIInput("/dev/vbi" + QString::number(vbiDevices[name]), this);

    return true;
  }

  qWarning() << "V4l2Device: Specified device was not found: " << name;
  return false;
}

QString V4l2Device::friendlyName(void) const
{
  if (videoDev)
    return videoDev->friendlyName();
  else
    return QString::null;
}

QString V4l2Device::longName(void) const
{
  if (videoDev)
    return videoDev->longName();
  else
    return QString::null;
}

STerminal::Types V4l2Device::terminalType(void) const
{
  if (openAudioDevice)
    return Type_AnalogTelevision | Type_AnalogRadio;
  else
    return Type_AnalogTelevision;
}

QStringList V4l2Device::inputs(void) const
{
  if (videoDev)
    return videoDev->inputs();
  else
    return QStringList();
}

bool V4l2Device::selectInput(const QString &name)
{
  return videoDev->selectInput(name);
}

STuner * V4l2Device::tuner(void) const
{
  if (tunerDev)
  if (tunerDev->hasTuner)
    return tunerDev;

  return NULL;
}

QList<STerminal::Stream> V4l2Device::inputStreams(void) const
{
  Stream stream;
  stream.name = "Capture";
  stream.videoPacketIDs += 0;

  return QList<Stream>() << stream;
}

QList<STerminal::Stream> V4l2Device::outputStreams(void) const
{
  return QList<Stream>();
}

SNode * V4l2Device::openStream(const Stream &)
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

quint32 V4l2Device::toV4L(const SVideoCodec &codec)
{
#define FOURCC(a,b,c,d)       (quint32(a) | (quint32(b) << 8) | (quint32(c) << 16) | (quint32(d) << 24))

  if (codec.isCompressed())
  {
    if      (codec == "MJPEG")                  return FOURCC('M','J','P','G');
    else if (codec == "LJPEG")                  return FOURCC('L','J','P','G');
    else if (codec == "FFVHUFF")                return FOURCC('F','F','V','H');
    else if (codec == "HUFFYUV")                return FOURCC('H','F','Y','U');
    else if (codec == "FFV1")                   return FOURCC('F','F','V','1');
    else if (codec == "MPEG1")                  return FOURCC('M','P','E','G');
    else if (codec == "MPEG2")                  return FOURCC('M','P','G','2');
    else if (codec == "MPEG4")                  return FOURCC('X','V','I','D');
    else if (codec == "DVVIDEO")                return FOURCC('d','v','s','l');
    else                                        return 0;
  }
  else switch (codec.format())
  {
  case SVideoCodec::Format_GRAY8:               return FOURCC('G','R','E','Y');
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  case SVideoCodec::Format_RGB555:              return FOURCC('R','G','B','O');
  case SVideoCodec::Format_RGB565:              return FOURCC('R','G','B','P');
  case SVideoCodec::Format_RGB24:               return FOURCC('R','G','B','3');
  case SVideoCodec::Format_BGR24:               return FOURCC('B','G','R','3');
  case SVideoCodec::Format_RGB32:               return FOURCC('R','G','B','4');
  case SVideoCodec::Format_BGR32:               return FOURCC('B','G','R','4');
#else // RGB and BGR are deliberately swapped here.
  case SVideoCodec::Format_RGB555:              return FOURCC('B','G','R','O');
  case SVideoCodec::Format_RGB565:              return FOURCC('B','G','R','P');
  case SVideoCodec::Format_RGB24:               return FOURCC('B','G','R','3');
  case SVideoCodec::Format_BGR24:               return FOURCC('R','G','B','3');
  case SVideoCodec::Format_RGB32:               return FOURCC('B','G','R','4');
  case SVideoCodec::Format_BGR32:               return FOURCC('R','G','B','4');
#endif
  case SVideoCodec::Format_YUYV422:             return FOURCC('Y','U','Y','V');
  case SVideoCodec::Format_UYVY422:             return FOURCC('U','Y','V','Y');
  case SVideoCodec::Format_YUV411P:             return FOURCC('4','1','1','P');
  case SVideoCodec::Format_YUV420P:             return FOURCC('4','2','0','P');
  case SVideoCodec::Format_YUV422P:             return FOURCC('4','2','2','P');

  default:                                      return 0;
  }
#undef FOURCC
}

v4l2_field V4l2Device::toV4L(SVideoCodec::FieldMode fieldMode)
{
  switch(fieldMode)
  {
  case SVideoCodec::FieldMode_Invalid:               return V4L2_FIELD_ANY;
  case SVideoCodec::FieldMode_Progressive:           return V4L2_FIELD_NONE;
  case SVideoCodec::FieldMode_TopField:              return V4L2_FIELD_TOP;
  case SVideoCodec::FieldMode_BottomField:           return V4L2_FIELD_BOTTOM;
  case SVideoCodec::FieldMode_InterlacedTopFirst:    return V4L2_FIELD_INTERLACED;
  case SVideoCodec::FieldMode_InterlacedBottomFirst: return V4L2_FIELD_INTERLACED;
  case SVideoCodec::FieldMode_SequentialTopFirst:    return V4L2_FIELD_SEQ_TB;
  case SVideoCodec::FieldMode_SequentialBottomFirst: return V4L2_FIELD_SEQ_BT;
  case SVideoCodec::FieldMode_Alternating:           return V4L2_FIELD_ALTERNATE;
  }

  return V4L2_FIELD_ANY;
}

quint16 V4l2Device::toV4LAudioMode(SAnalogTuner::AudioStandard audioStandard)
{
  switch(audioStandard)
  {
  case SAnalogTuner::AudioStandard_None:    return V4L2_TUNER_MODE_STEREO; // Default stereo
  case SAnalogTuner::AudioStandard_Mono:    return V4L2_TUNER_MODE_MONO;
  case SAnalogTuner::AudioStandard_Stereo:  return V4L2_TUNER_MODE_STEREO;
  case SAnalogTuner::AudioStandard_Lang1:   return V4L2_TUNER_MODE_LANG1;
  case SAnalogTuner::AudioStandard_Lang2:   return V4L2_TUNER_MODE_LANG2;
  }

  return V4L2_TUNER_MODE_STEREO;
}

SAnalogTuner::AudioStandard V4l2Device::fromV4LAudioMode(quint16 mode)
{
  switch(mode)
  {
  default:                      return SAnalogTuner::AudioStandard_None;
  case V4L2_TUNER_MODE_MONO:    return SAnalogTuner::AudioStandard_Mono;
  case V4L2_TUNER_MODE_STEREO:  return SAnalogTuner::AudioStandard_Stereo;
  case V4L2_TUNER_MODE_LANG1:   return SAnalogTuner::AudioStandard_Lang1;
  case V4L2_TUNER_MODE_LANG2:   return SAnalogTuner::AudioStandard_Lang2;
  }
}

v4l2_std_id V4l2Device::toV4LVideoStandard(SAnalogTuner::VideoStandard videoStandard)
{
  switch(videoStandard)
  {
  case SAnalogTuner::VideoStandard_None:

  case SAnalogTuner::VideoStandard_PAL:             return V4L2_STD_PAL;
  case SAnalogTuner::VideoStandard_PAL_B:           return V4L2_STD_PAL_B;
  case SAnalogTuner::VideoStandard_PAL_B1:          return V4L2_STD_PAL_B1;
  case SAnalogTuner::VideoStandard_PAL_G:           return V4L2_STD_PAL_G;
  case SAnalogTuner::VideoStandard_PAL_H:           return V4L2_STD_PAL_H;
  case SAnalogTuner::VideoStandard_PAL_I:           return V4L2_STD_PAL_I;
  case SAnalogTuner::VideoStandard_PAL_D:           return V4L2_STD_PAL_D;
  case SAnalogTuner::VideoStandard_PAL_D1:          return V4L2_STD_PAL_D1;
  case SAnalogTuner::VideoStandard_PAL_K:           return V4L2_STD_PAL_K;
  case SAnalogTuner::VideoStandard_PAL_BG:          return V4L2_STD_PAL_BG;
  case SAnalogTuner::VideoStandard_PAL_DK:          return V4L2_STD_PAL_DK;
  case SAnalogTuner::VideoStandard_PAL_M:           return V4L2_STD_PAL_M;
  case SAnalogTuner::VideoStandard_PAL_N:           return V4L2_STD_PAL_N;
  case SAnalogTuner::VideoStandard_PAL_Nc:          return V4L2_STD_PAL_Nc;
  case SAnalogTuner::VideoStandard_PAL_60:          return V4L2_STD_PAL_60;

  case SAnalogTuner::VideoStandard_NTSC:            return V4L2_STD_NTSC;
  case SAnalogTuner::VideoStandard_NTSC_M:          return V4L2_STD_NTSC_M;
  case SAnalogTuner::VideoStandard_NTSC_M_JP:       return V4L2_STD_NTSC_M_JP;

  case SAnalogTuner::VideoStandard_SECAM:           return V4L2_STD_SECAM;
  case SAnalogTuner::VideoStandard_SECAM_B:         return V4L2_STD_SECAM_B;
  case SAnalogTuner::VideoStandard_SECAM_D:         return V4L2_STD_SECAM_D;
  case SAnalogTuner::VideoStandard_SECAM_G:         return V4L2_STD_SECAM_G;
  case SAnalogTuner::VideoStandard_SECAM_H:         return V4L2_STD_SECAM_H;
  case SAnalogTuner::VideoStandard_SECAM_K:         return V4L2_STD_SECAM_K;
  case SAnalogTuner::VideoStandard_SECAM_K1:        return V4L2_STD_SECAM_K1;
  case SAnalogTuner::VideoStandard_SECAM_L:         return V4L2_STD_SECAM_L;
  case SAnalogTuner::VideoStandard_SECAM_DK:        return V4L2_STD_SECAM_DK;
  }

  return V4L2_TUNER_MODE_STEREO;
}

SAnalogTuner::VideoStandard V4l2Device::fromV4LVideoStandard(v4l2_std_id id)
{
  switch (id)
  {
  case V4L2_STD_PAL_B:                    return SAnalogTuner::VideoStandard_PAL_B;
  case V4L2_STD_PAL_B1:                   return SAnalogTuner::VideoStandard_PAL_B1;
  case V4L2_STD_PAL_G:                    return SAnalogTuner::VideoStandard_PAL_G;
  case V4L2_STD_PAL_H:                    return SAnalogTuner::VideoStandard_PAL_H;
  case V4L2_STD_PAL_I:                    return SAnalogTuner::VideoStandard_PAL_I;
  case V4L2_STD_PAL_D:                    return SAnalogTuner::VideoStandard_PAL_D;
  case V4L2_STD_PAL_D1:                   return SAnalogTuner::VideoStandard_PAL_D1;
  case V4L2_STD_PAL_K:                    return SAnalogTuner::VideoStandard_PAL_K;
  case V4L2_STD_PAL_M:                    return SAnalogTuner::VideoStandard_PAL_M;
  case V4L2_STD_PAL_N:                    return SAnalogTuner::VideoStandard_PAL_N;
  case V4L2_STD_PAL_Nc:                   return SAnalogTuner::VideoStandard_PAL_Nc;
  case V4L2_STD_PAL_60:                   return SAnalogTuner::VideoStandard_PAL_60;
  case V4L2_STD_NTSC_M:                   return SAnalogTuner::VideoStandard_NTSC_M;
  case V4L2_STD_NTSC_M_JP:                return SAnalogTuner::VideoStandard_NTSC_M_JP;
  case V4L2_STD_SECAM_B:                  return SAnalogTuner::VideoStandard_SECAM_B;
  case V4L2_STD_SECAM_D:                  return SAnalogTuner::VideoStandard_SECAM_D;
  case V4L2_STD_SECAM_G:                  return SAnalogTuner::VideoStandard_SECAM_G;
  case V4L2_STD_SECAM_H:                  return SAnalogTuner::VideoStandard_SECAM_H;
  case V4L2_STD_SECAM_K:                  return SAnalogTuner::VideoStandard_SECAM_K;
  case V4L2_STD_SECAM_K1:                 return SAnalogTuner::VideoStandard_SECAM_K1;
  case V4L2_STD_SECAM_L:                  return SAnalogTuner::VideoStandard_SECAM_L;

  case V4L2_STD_PAL_BG:                   return SAnalogTuner::VideoStandard_PAL_BG;
  case V4L2_STD_PAL_DK:                   return SAnalogTuner::VideoStandard_PAL_DK;
  case V4L2_STD_PAL:                      return SAnalogTuner::VideoStandard_PAL;
  case V4L2_STD_NTSC:                     return SAnalogTuner::VideoStandard_NTSC;
  case V4L2_STD_SECAM_DK:                 return SAnalogTuner::VideoStandard_SECAM_DK;
  case V4L2_STD_SECAM:                    return SAnalogTuner::VideoStandard_SECAM;

  default:
    if      ((id & V4L2_STD_PAL) != 0)    return SAnalogTuner::VideoStandard_PAL;
    else if ((id & V4L2_STD_NTSC) != 0)   return SAnalogTuner::VideoStandard_NTSC;
    else if ((id & V4L2_STD_SECAM) != 0)  return SAnalogTuner::VideoStandard_SECAM;
    else                                  return SAnalogTuner::VideoStandard_None;
  }
}


} } // End of namespaces
