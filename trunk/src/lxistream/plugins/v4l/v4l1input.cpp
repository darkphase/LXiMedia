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

#include "v4l1input.h"
#include <errno.h>

namespace LXiStream {
namespace V4lBackend {

QMap<QString, int> V4l1Input::deviceMap;

QList<SFactory::Scheme> V4l1Input::listDevices(void)
{
  QList<SFactory::Scheme> result;

  deviceMap.clear();
  for (int i=0; i<=99; i++)
  {
    const QByteArray devName = QByteArray("/dev/video") + QByteArray::number(i);

    int fd = ::open(devName, O_RDONLY);
    if (fd < 0)
      break; // last device

    video_capability capabilities;
    memset(&capabilities, 0, sizeof(capabilities));
    v4l2_capability capabilitiesV2;
    memset(&capabilitiesV2, 0, sizeof(capabilitiesV2));

    // Don't show devices that also support V4L2.
    if(::ioctl(fd, VIDIOC_QUERYCAP, &capabilitiesV2) < 0)
    if(::ioctl(fd, VIDIOCGCAP, &capabilities) >= 0)
    {
      const QString name = (const char *)capabilities.name;
      deviceMap[name] = i;
      result += SFactory::Scheme(-1, name);
    }

    ::close(fd);
  }

  return result;
}

V4l1Input::V4l1Input(const QString &device, QObject *parent)
  : SInterfaces::VideoInput(parent),
    devDesc(-1),
    mutex(QMutex::Recursive),
    outFormat(),
    buffers(),
    mmaps(NULL),
    map(NULL),
    bufferLock(NULL),
    bufferSize(0),
    currentBufferIndex(0)
{
  if (deviceMap.contains(device))
  {
    const QByteArray devName = QByteArray("/dev/video") + QByteArray::number(deviceMap[device]);

    devDesc = ::open(devName, O_RDWR);
    if (devDesc >= 0)
    {
      memset(&capabilities, 0, sizeof(capabilities));
      if (::ioctl(devDesc, VIDIOCGCAP, &capabilities) < 0)
        qWarning() << "V4l1Input: failed to ioctl device (VIDIOCGCAP)";
    }
    else
      qWarning() << "V4l1Input: failed to open: " << device;
  }
  else
    qFatal("V4l1Input: No such device: %s", device.toUtf8().data());
}

V4l1Input::~V4l1Input()
{
  if (devDesc >= 0)
    ::close(devDesc);
}

void V4l1Input::setFormat(const SVideoFormat &format)
{
  outFormat = format;
}

SVideoFormat V4l1Input::format(void)
{
  return outFormat;
}

/*bool V4l1Input::selectInput(int inputIndex)
{
    struct video_channel channel;
    memset(&channel, 0, sizeof(channel));

    channel.channel = inputIndex;
    if (ioctl(devDesc, VIDIOCGCHAN, &channel) < 0)
      return false;

    channel.channel = inputIndex;
    if (ioctl(devDesc, VIDIOCSCHAN, &channel) < 0)
      return false;

    return true;
}*/

bool V4l1Input::start(void)
{
  // Find a format
  QList<SVideoFormat> formats;
  formats += outFormat;
  formats += SVideoFormat(SVideoFormat::Format_RGB32, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedTopFirst);
  formats += SVideoFormat(SVideoFormat::Format_RGB32, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedBottomFirst);
  formats += SVideoFormat(SVideoFormat::Format_RGB32, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_Progressive);
  formats += SVideoFormat(SVideoFormat::Format_YUYV422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedTopFirst);
  formats += SVideoFormat(SVideoFormat::Format_YUYV422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedBottomFirst);
  formats += SVideoFormat(SVideoFormat::Format_YUYV422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_Progressive);
  formats += SVideoFormat(SVideoFormat::Format_UYVY422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedTopFirst);
  formats += SVideoFormat(SVideoFormat::Format_UYVY422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedBottomFirst);
  formats += SVideoFormat(SVideoFormat::Format_UYVY422, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_Progressive);

  outFormat = SVideoFormat();
  foreach(SVideoFormat testFormat, formats)
  {
    SSize testSize = testFormat.size();

    if (testSize.width() == 0)
      testSize.setWidth(capabilities.maxwidth);
    else if (testSize.width() < int(capabilities.minwidth))
      testSize.setWidth(capabilities.minwidth);
    else if (testSize.width() > int(capabilities.maxwidth))
      testSize.setWidth(capabilities.maxwidth);

    if (testSize.height() == 0)
      testSize.setHeight(capabilities.maxheight);
    else if (testSize.height() < int(capabilities.minheight))
      testSize.setHeight(capabilities.minheight);
    else if (testSize.height() > int(capabilities.maxheight))
      testSize.setHeight(capabilities.maxheight);

    testFormat.setSize(testSize);

    if (testFormat.fieldMode() == SVideoFormat::FieldMode_Invalid)
      testFormat.setFieldMode(SVideoFormat::FieldMode_Progressive);

    video_picture grabPict;
    memset(&grabPict, 0, sizeof(grabPict));

    if (::ioctl(devDesc, VIDIOCGPICT, &grabPict) >= 0)
    {
      grabPict.depth = testFormat.sampleSize() * 8;
      grabPict.palette = toV4L(testFormat);

      if (ioctl(devDesc, VIDIOCSPICT, &grabPict) >= 0 )
      {
        video_window grabWin;
        memset(&grabWin, 0, sizeof(grabWin));

        grabWin.x = 0;
        grabWin.y = 0;
        grabWin.width = testSize.width();
        grabWin.height = testSize.height();

        bufferSize = testFormat.sampleSize() * testSize.width() * testSize.height();

        if (::ioctl(devDesc, VIDIOCSWIN, &grabWin) >= 0)
        {
          outFormat = testFormat;
          // Assuming always 4:3 aspect ratio.
          testSize.setAspectRatio(float(grabWin.height) / float(grabWin.width) * 1.333f);
          outFormat.setSize(testSize);
          currentBufferIndex = 0;

          memset(&buffers, 0, sizeof(buffers));

          if (::ioctl(devDesc, VIDIOCGMBUF, &buffers) >= 0)
          {
            mmaps = new video_mmap[buffers.frames];

            for (int i=0; i<buffers.frames; i++)
            {
              mmaps[i].format = grabPict.palette;
              mmaps[i].frame = i;
              mmaps[i].width = grabWin.width;
              mmaps[i].height = grabWin.height;
            }

            map = reinterpret_cast<char *>(mmap(0, buffers.size,
                                                PROT_READ | PROT_WRITE,
                                                MAP_SHARED,
                                                devDesc, 0));

            if (map != MAP_FAILED)
            {
              bufferLock = new QSemaphore[buffers.frames];

              for (int i=0; i<buffers.frames; i++)
                queueBuffer(i);

              break;
            }

            // Failed, remove buffers again and fallback to read() capture
            delete [] mmaps;
            mmaps = NULL;
            map = NULL;
          }

          // Fallback to read() capture
          break;
        }
      }
    }
  }

  return !outFormat.isNull();
}

void V4l1Input::stop(void)
{
  bufferSize = 0;

  if (map != NULL)
  {
    for (int i=0; i<buffers.frames; i++)
      bufferLock[i].acquire(1);

    delete [] bufferLock;
    bufferLock = NULL;
    delete [] mmaps;
    mmaps = NULL;
    ::munmap(map, buffers.size);
    map = NULL;
  }
}

void V4l1Input::process(void)
{
  if (map)
  { // mmap based capture
    bufferLock[currentBufferIndex].acquire(1);

    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);
    if (::ioctl(devDesc, VIDIOCSYNC, mmaps + currentBufferIndex) >= 0)
    {
      l.unlock();

      Memory * const mem = 
          new Memory(map + buffers.offsets[currentBufferIndex],
                     bufferSize,
                     currentBufferIndex,
                     this);

      const int offset[4] = { 0, 0, 0, 0 };
      const int lineSize[4] = { bufferSize / outFormat.size().height(), 0, 0, 0 };

      SVideoBuffer buffer(outFormat, SBuffer::MemoryPtr(mem), offset, lineSize);
      buffer.setTimeStamp(timer.smoothTimeStamp());

      currentBufferIndex = (currentBufferIndex + 1) % buffers.frames;

      emit produce(buffer);
    }
    else
    {
      qWarning() << "V4l1Input: failed to wait for frame " << currentBufferIndex << " (VIDIOCSYNC)";
      bufferLock[currentBufferIndex].release(1);
    }
  }
  else
  { // read() based capture
    SVideoBuffer buffer(outFormat);

    int r = 0;
    if ((r = ::read(devDesc, buffer.data(), buffer.size())) > 0)
    {
      buffer.setTimeStamp(timer.smoothTimeStamp());

      emit produce(buffer);
    }
    else
      qWarning() << "V4l1Input: failed to read from device" << errno;
  }
}

quint16 V4l1Input::toV4L(SVideoFormat::Format format)
{
  switch (format)
  {
  case SVideoFormat::Format_GRAY8:         return VIDEO_PALETTE_GREY;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  case SVideoFormat::Format_RGB555:        return VIDEO_PALETTE_RGB555;
  case SVideoFormat::Format_RGB565:        return VIDEO_PALETTE_RGB565;
  case SVideoFormat::Format_RGB24:         return VIDEO_PALETTE_RGB24;
  case SVideoFormat::Format_RGB32:         return VIDEO_PALETTE_RGB32;
#else // RGB and BGR are deliberately swapped here.
  case SVideoFormat::Format_BGR24:         return VIDEO_PALETTE_RGB24;
  case SVideoFormat::Format_BGR32:         return VIDEO_PALETTE_RGB32;
#endif
  case SVideoFormat::Format_YUYV422:       return VIDEO_PALETTE_YUYV;
  case SVideoFormat::Format_UYVY422:       return VIDEO_PALETTE_UYVY;
  case SVideoFormat::Format_YUV411P:       return VIDEO_PALETTE_YUV411P;
  case SVideoFormat::Format_YUV420P:       return VIDEO_PALETTE_YUV420P;
  case SVideoFormat::Format_YUV422P:       return VIDEO_PALETTE_YUV422P;
  default:                                 return 0;
  }
}

void V4l1Input::queueBuffer(int index)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (index < buffers.frames)
  {
    if (ioctl(devDesc, VIDIOCMCAPTURE, mmaps + index) >= 0)
      bufferLock[index].release(1);
    else
      qWarning() << "V4l1Input: failed to free frame " << index << " (VIDIOCMCAPTURE)";
  }
}


V4l1Input::Memory::Memory(char *data, int size, int bufferIndex, V4l1Input *parent)
  : SBuffer::Memory(size, data, size),
    bufferIndex(bufferIndex),
    parent(parent)
{
}

V4l1Input::Memory::~Memory()
{
  parent->queueBuffer(bufferIndex);
}


} } // End of namespaces
