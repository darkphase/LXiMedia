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

#include "v4l2input.h"
#include <fcntl.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

namespace LXiStream {
namespace V4lBackend {

QMap<QString, int> V4l2Input::deviceMap;

QList<SFactory::Scheme> V4l2Input::listDevices(void)
{
  QList<SFactory::Scheme> result;

  deviceMap.clear();
  for (unsigned i=0; i<=31; i++)
  {
    const QByteArray devName = QByteArray("/dev/video") + QByteArray::number(i);

    int fd = ::open(devName, O_RDONLY);
    if (fd < 0)
      break; // last device

    v4l2_capability capabilities;
    memset(&capabilities, 0, sizeof(capabilities));

    if(::ioctl(fd, VIDIOC_QUERYCAP, &capabilities) >= 0)
    {
      const QString name = QString((const char *)capabilities.card) + " " + QString((const char *)capabilities.bus_info);
      deviceMap[name] = i;
      result += SFactory::Scheme(-1, name);
    }

    ::close(fd);
  }

  return result;
}

V4l2Input::V4l2Input(const QString &device, QObject *parent)
  : SInterfaces::VideoInput(parent),
    devDesc(-1),
    mutex(QMutex::Recursive),
    outFormat(SVideoFormat::Format_Invalid, SSize(1280, 1024), SInterval::fromFrequency(25)),
    numOutBuffers(defaultOutBuffers),
    agc(true),
    agcCounter(0),
    agcMin(-1.0f), agcMax(-1.0f), agcMaxc(-1.0f),
    agcWeight(1.0),
    bufferRequest(),
    mappedBuffers(0),
    buffers(NULL),
    maps(NULL),
    bufferSize(0),
    streamOn(false)
{
  if (deviceMap.contains(device))
  {
    const QByteArray devName = QByteArray("/dev/video") + QByteArray::number(deviceMap[device]);

    devDesc = ::open(devName, O_RDWR);
    if (devDesc >= 0)
    {
      memset(&capabilities, 0, sizeof(capabilities));
      if (::ioctl(devDesc, VIDIOC_QUERYCAP, &capabilities) < 0)
        qWarning() << "V4l2Input: failed to ioctl device (VIDIOC_QUERYCAP) " << device;

      // Set default priority
      v4l2_priority prio = V4L2_PRIORITY_DEFAULT;
      ioctl(devDesc, VIDIOC_S_PRIORITY, &prio);

      // Enumerate formats
      for (int i=0; i>=0; i++)
      {
        v4l2_fmtdesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.index = i;
        desc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (::ioctl(devDesc, VIDIOC_ENUM_FMT, &desc) >= 0)
          pixelFormats += fromV4L(desc.pixelformat);
        else
          break;
      }

      if (pixelFormats.isEmpty())
      { // Default formats
        pixelFormats += SVideoFormat::Format_YUYV422;
        pixelFormats += SVideoFormat::Format_UYVY422;
        pixelFormats += SVideoFormat::Format_RGB32;
        pixelFormats += SVideoFormat::Format_BGR32;
      }

      // Enumerate inputs
      /*for (int i=0; i>=0; i++)
      {
        v4l2_input desc;
        memset(&desc, 0, sizeof(desc));
        desc.index = i;

        if (::ioctl(devDesc, VIDIOC_ENUMINPUT, &desc) >= 0)
          videoInputs[(const char *)desc.name] = desc;
        else
          break;
      }*/

      // Enumerate standards
      /*for (int i=0; i>=0; i++)
      {
        v4l2_standard desc;
        memset(&desc, 0, sizeof(desc));
        desc.index = i;

        if (::ioctl(devDesc, VIDIOC_ENUMSTD, &desc) >= 0)
          videoStandards[fromV4LVideoStandard(desc.id)] = desc;
        else
          break;
      }*/

      // Enumerate public controls
      /*for (int i=V4L2_CID_BASE; i<V4L2_CID_LASTP1; i++)
      {
        v4l2_queryctrl desc;
        memset(&desc, 0, sizeof(desc));
        desc.id = i;

        if (::ioctl(devDesc, VIDIOC_QUERYCTRL, &desc) >= 0)
        {
          if ((desc.flags & V4L2_CTRL_FLAG_DISABLED) == 0)
            videoControls[(const char *)desc.name] = desc;
        }
        else
          break;
      }*/

      // Enumerate private controls
      /*for (int i=V4L2_CID_PRIVATE_BASE; i>=0; i++)
      {
        v4l2_queryctrl desc;
        memset(&desc, 0, sizeof(desc));
        desc.id = i;

        if (::ioctl(devDesc, VIDIOC_QUERYCTRL, &desc) >= 0)
        {
          if ((desc.flags & V4L2_CTRL_FLAG_DISABLED) == 0)
            videoControls[(const char *)desc.name] = desc;
        }
        else
          break;
      }*/

      // Select input
      /*int inputID = 0;
      if (::ioctl(devDesc, VIDIOC_G_INPUT, &inputID) >= 0)
        selectInput(inputID);
      else
        qWarning() << "V4l2Input: failed to select input " << device;*/
    }
    else
      qWarning() << "V4l2Input: failed to open " << device;
  }
  else
    qFatal("V4l2Input: No such device: %s", device.toUtf8().data());
}

V4l2Input::~V4l2Input()
{
  if (devDesc >= 0)
    ::close(devDesc);
}

void V4l2Input::setFormat(const SVideoFormat &format)
{
  outFormat = format;
}

SVideoFormat V4l2Input::format(void)
{
  return outFormat;
}

void V4l2Input::setMaxBuffers(int b)
{
  numOutBuffers = b > 0 ? b : defaultOutBuffers;
}

int V4l2Input::maxBuffers(void) const
{
  return numOutBuffers;
}

bool V4l2Input::start(void)
{
  // Build a list of codecs to test
  QList<SVideoFormat> formats;
  if (!outFormat.isNull() && pixelFormats.contains(outFormat.format()))
    formats += outFormat;

  foreach (SVideoFormat::Format format, pixelFormats)
  {
    formats += SVideoFormat(format, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_Progressive);
    formats += SVideoFormat(format, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedTopFirst);
    formats += SVideoFormat(format, outFormat.size(), outFormat.frameRate(), SVideoFormat::FieldMode_InterlacedBottomFirst);
  }

  // Find a codec
  outFormat = SVideoFormat();
  foreach(SVideoFormat testFormat, formats)
  {
    v4l2_format imgformat;
    memset(&imgformat, 0, sizeof(imgformat));

    SSize testSize = testFormat.size();

    imgformat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imgformat.fmt.pix.width = testSize.width() > 0 ? testSize.width() : 720;
    imgformat.fmt.pix.height = testSize.height() > 0 ? testSize.height() : 576;
    imgformat.fmt.pix.pixelformat = toV4L(testFormat);
    imgformat.fmt.pix.field = toV4L(testFormat.fieldMode());
    imgformat.fmt.pix.bytesperline = testFormat.sampleSize() * testSize.width();

    if (::ioctl(devDesc, VIDIOC_S_FMT, &imgformat) >= 0)
    {
      if (::ioctl(devDesc, VIDIOC_G_FMT, &imgformat) >= 0)
      {
        testSize.setWidth(imgformat.fmt.pix.width);
        testSize.setHeight(imgformat.fmt.pix.height);
      }

      // Assuming always 4:3 aspect ratio.
      testSize.setAspectRatio(float(imgformat.fmt.pix.height) / float(imgformat.fmt.pix.width) * 1.333f);
      testFormat.setSize(testSize);
      outFormat = testFormat;
      bufferSize = outFormat.sampleSize() * testSize.width() * testSize.height();

      // Request new buffers
      bufferRequest.count = numOutBuffers;
      bufferRequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      bufferRequest.memory = V4L2_MEMORY_MMAP;

      if (::ioctl(devDesc, VIDIOC_REQBUFS, &bufferRequest) >= 0)
      {
        mappedBuffers = 0;
        buffers = new v4l2_buffer[bufferRequest.count];
        maps = new char *[bufferRequest.count];

        for (unsigned i=0; i<bufferRequest.count; i++)
        {
          v4l2_buffer * const buffer = buffers + i;
          memset(buffer, 0, sizeof(*buffer));

          buffer->index = i;
          buffer->type = bufferRequest.type;
          buffer->memory = bufferRequest.memory;
          if (ioctl(devDesc, VIDIOC_QUERYBUF, buffer) >= 0)
          {
            maps[i] = reinterpret_cast<char *>(
                mmap(NULL,
                     buffer->length,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     devDesc,
                     buffer->m.offset));

            mappedBuffers++;
          }
          else
            break;
        }
      }

      for (int i=0; i<mappedBuffers; i++)
      if (::ioctl(devDesc, VIDIOC_QBUF, buffers + i) < 0)
        qWarning() << "V4l2Input: failed to ioctl device (VIDIOC_QBUF) " << i;

      // Start the stream
      if (::ioctl(devDesc, VIDIOC_STREAMON, &bufferRequest.type) >= 0)
      {
        streamOn = true;

        ::v4l2_control control;
        control.id = V4L2_CID_AUDIO_MUTE;
        control.value = 0;
        ::ioctl(devDesc, VIDIOC_S_CTRL, &control);
        setControl(V4L2_CID_AUDIO_VOLUME, 0.8);

        return true;
      }
      else
        stop();
    }
  }

  qWarning() << "V4l2Input: failed to initialize";
  return false;
}

void V4l2Input::stop(void)
{
  if (streamOn)
  {
    ::ioctl(devDesc, VIDIOC_STREAMOFF, &bufferRequest.type);
    streamOn = false;
  }

  ::v4l2_control control;
  control.id = V4L2_CID_AUDIO_MUTE;
  control.value = 1;
  ::ioctl(devDesc, VIDIOC_S_CTRL, &control);
  setControl(V4L2_CID_AUDIO_VOLUME, 0.0);

  bufferSize = 0;
  agcMin = agcMax = agcMaxc = -1.0f;
  agcWeight = 1.0;

  if (mappedBuffers > 0)
  {
    for (int i=0; i<mappedBuffers; i++)
      munmap((void *)maps[i], buffers[i].length);

    delete [] buffers;
    buffers = NULL;
    delete [] maps;
    maps = NULL;
    mappedBuffers = 0;
  }
}

void V4l2Input::process(void)
{
  if (mappedBuffers > 0)
  { // mmap based capture
    for (int n=0; n<3; n++)
    {
      Memory * const mem = nextImage();
      if (mem)
      {
        SVideoBuffer buffer;
        if (outFormat.numPlanes() == 3)
        {
          int wr = 0, hr = 0;
          outFormat.planarYUVRatio(wr, hr);

          const int w = outFormat.size().width(), h = outFormat.size().height();
          const int s = w * h, s1 = (w / wr) * (h / hr);
          const int offset[4] = { 0, s, s + s1 , s + s1 + s1 };
          const int lineSize[4] = { h, h / hr, h / hr, 0 };

          buffer = SVideoBuffer(outFormat, SBuffer::MemoryPtr(mem), offset, lineSize);
        }
        else
        {
          const int offset[4] = { 0, 0, 0, 0 };
          const int lineSize[4] = { bufferSize / outFormat.size().height(), 0, 0, 0 };

          buffer = SVideoBuffer(outFormat, SBuffer::MemoryPtr(mem), offset, lineSize);
        }

        buffer.setTimeStamp(timer.smoothTimeStamp());

        if (agc)
        if ((agcCounter = (agcCounter + 1) % 25) == 0)
          updateAgc(buffer);

        emit produce(buffer);
      }
      else
      {
        qDebug() << "V4l2Input: Failed waiting for next image; attempting to recover stream.";

        // Stop the stream
        SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);
        ::ioctl(devDesc, VIDIOC_STREAMOFF, &bufferRequest.type);
        l.unlock();

        // Start it again
        l.relock(__FILE__, __LINE__);
        if (::ioctl(devDesc, VIDIOC_STREAMON, &bufferRequest.type) >= 0)
        {
          for (int i=0; i<mappedBuffers; i++)
            queueBuffer(i);
        }
      }
    }
  }
  else
  { // read() based capture
    SVideoBuffer buffer(outFormat);

    for (int n=0; n<3; n++)
    {
      int r = 0;
      if ((r = ::read(devDesc, buffer.data(), buffer.size())) > 0)
      {
        buffer.setTimeStamp(timer.smoothTimeStamp());

        if (agc)
        if ((agcCounter = (agcCounter + 1) % 25) == 0)
          updateAgc(buffer);

        emit produce(buffer);
      }
      else
      {
        qDebug() << "V4l2Input: Failed to read from device; attempting to recover stream.";

        // Stop the stream
        SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);
        ::ioctl(devDesc, VIDIOC_STREAMOFF, &bufferRequest.type);
        l.unlock();

        // Start it again
        l.relock(__FILE__, __LINE__);
        if (::ioctl(devDesc, VIDIOC_STREAMON, &bufferRequest.type) >= 0)
        {
          for (int i=0; i<mappedBuffers; i++)
            queueBuffer(i);
        }
      }
    }
  }
}

/*bool V4l2Input::selectInput(int inputIndex)
{
  if (ioctl(devDesc, VIDIOC_S_INPUT, &inputIndex) >= 0)
  {
    v4l2_input input;
    memset(&input, 0, sizeof(input));
    input.index = inputIndex;

    if (ioctl(devDesc, VIDIOC_G_INPUT, &input.index) >= 0)
    if (ioctl(devDesc, VIDIOC_ENUMINPUT, &input) >= 0)
    {
      Q_ASSERT(parent->tunerDev);
      parent->tunerDev->hasTuner = false;

      if (input.type == V4L2_INPUT_TYPE_TUNER)
      {
        v4l2_tuner tnr;
        memset(&tnr, 0, sizeof(tnr));
        tnr.index = input.tuner;
        if (ioctl(devDesc, VIDIOC_G_TUNER, &tnr) >= 0)
        {
          parent->tunerDev->hasTuner = true;
          parent->tunerDev->tunerID = input.tuner;
          parent->tunerDev->tunerLow = (tnr.capability & V4L2_TUNER_CAP_LOW) != 0;
        }
      }

      v4l2_std_id std = V4l2Device::toV4LVideoStandard(SAnalogTuner::VideoStandard_PAL);
      ioctl(devDesc, VIDIOC_S_STD, &std);

      return true;
    }
  }

  return false;
}*/

qreal V4l2Input::control(quint32 id) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  v4l2_queryctrl query;
  query.id = id;

  if ((ioctl(devDesc, VIDIOC_QUERYCTRL, &query) >= 0) && ((query.flags & V4L2_CTRL_FLAG_DISABLED) == 0))
  {
    v4l2_control control;
    memset(&control, 0, sizeof(control));
    control.id = id;

    if (ioctl(devDesc, VIDIOC_G_CTRL, &control) >= 0)
      return qreal(control.value - query.minimum) / qreal(query.maximum - query.minimum);
  }

  return 0.0;
}

bool V4l2Input::setControl(quint32 id, qreal value)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  v4l2_queryctrl query;
  memset(&query, 0, sizeof(query));
  query.id = id;

  if ((ioctl(devDesc, VIDIOC_QUERYCTRL, &query) >= 0) && ((query.flags & V4L2_CTRL_FLAG_DISABLED) == 0))
  {
    v4l2_control control;
    control.id = id;
    control.value = query.minimum + int(qreal(query.maximum - query.minimum) * value + 0.5);

    if (ioctl(devDesc, VIDIOC_S_CTRL, &control) >= 0)
      return true;
  }

  return false;
}

void V4l2Input::updateAgc(const SVideoBuffer &buffer)
{
  static const unsigned numSamples = 32;
  const SSize size = buffer.format().size();
  const unsigned sampleWidth = size.width() / numSamples;
  const SVideoFormat::Format format = buffer.format().format();

  float min = 1.0f, max = 0.0f, maxc = 0.0f;

  if ((format == SVideoFormat::Format_YUYV422) ||
      (format == SVideoFormat::Format_UYVY422))
  {
    for (unsigned i=0; i<numSamples; i++)
    {
      const quint8 * const line =
          reinterpret_cast<const quint8 *>(buffer.scanLine(size.height() / numSamples * i, 0));

      if (format == SVideoFormat::Format_YUYV422)
      {
        for (unsigned x=0, n=0; n<numSamples; x+=(sampleWidth*2), n++)
        {
          const float v = float(line[x]) / 255.0f;
          min = qMin(v, min);
          max = qMax(v, max);
          maxc = qMax(float(qAbs(line[x+1] - 128)) / 128.0f, maxc);
        }
      }
      else
      {
        for (unsigned x=1, n=0; n<numSamples; x+=(sampleWidth*2), n++)
        {
          const float v = float(line[x]) / 255.0f;
          min = qMin(v, min);
          max = qMax(v, max);
          maxc = qMax(float(qAbs(line[x-1] - 128)) / 128.0f, maxc);
        }
      }
    }
  }
  else if ((format == SVideoFormat::Format_YUV410P) ||
           (format == SVideoFormat::Format_YUV411P) ||
           (format == SVideoFormat::Format_YUV420P) ||
           (format == SVideoFormat::Format_YUV422P) ||
           (format == SVideoFormat::Format_YUV444P))
  {
    int cw = 2, ch = 2;
    SVideoFormat::planarYUVRatio(format, cw, ch);

    for (unsigned i=0; i<numSamples; i++)
    {
      const quint8 * const line =
          reinterpret_cast<const quint8 *>(buffer.scanLine((size.height() / ch) / numSamples * i, 0));
      const quint8 * const linec =
          reinterpret_cast<const quint8 *>(buffer.scanLine((size.height() / ch) / numSamples * i, 0));

      for (unsigned x=0, n=0; n<numSamples; x+=sampleWidth, n++)
      {
        const float v = float(line[x]) / 255.0f;
        min = qMin(v, min);
        max = qMax(v, max);
        maxc = qMax(float(qAbs(linec[x / cw] - 128)) / 128.0f, maxc);
      }
    }
  }
  else if ((format == SVideoFormat::Format_BGR32) ||
           (format == SVideoFormat::Format_RGB32))
  {
    for (unsigned i=0; i<numSamples; i++)
    {
      const quint32 * const line =
          reinterpret_cast<const quint32 *>(buffer.scanLine(size.height() / numSamples * i, 0));

      for (unsigned x=0, n=0; n<numSamples; x+=sampleWidth, n++)
      {
        const float v = float((line[x] >> 16) & 0xFF) / 255.0f;
        min = qMin(v, min);
        max = qMax(v, max);
        maxc = qMax(float(qAbs(int((line[x] >> 24) & 0xFF) - int(line[x] & 0xFF))) / 255.0f, maxc);
      }
    }
  }

  bool adjusting = false;

  if (max > 0.5f)
  {
    qreal brg = brightness(), obrg = brg;
    qreal con = contrast(), ocon = con;

    agcMin = (agcMin * (1.0f - agcWeight)) + (min * agcWeight);
    agcMax = (agcMax * (1.0f - agcWeight)) + (max * agcWeight);
    if (agcMin > 0.20f) brg -= 0.01;
    if (agcMax < 0.70f) con += 0.01;
    if (agcMin < 0.15f) brg += 0.01;
    if (agcMax > 0.75f) con -= 0.01;

    brg = qBound(0.3, brg, 0.7);
    con = qBound(0.3, con, 0.7);

    if ((qAbs(brg - obrg) >= 0.005) || (qAbs(con - ocon) >= 0.005))
    {
      setBrightness(brg);
      setContrast(con);
      adjusting = true;
    }

    //qDebug() << agcMin << agcMax << brg << con;
  }

  if (maxc >= 0.1f)
  {
    qreal sat = saturation(), osat = sat;

    agcMaxc = (agcMaxc * (1.0f - agcWeight)) + (maxc * agcWeight);
    if (agcMaxc < 0.7f) sat += 0.02;
    if (agcMaxc > 0.9f) sat -= 0.02;

    sat = qBound(0.5, sat, 0.9);

    if (qAbs(sat - osat) >= 0.01)
    {
      setSaturation(sat);
      adjusting = true;
    }
  }

  if (!adjusting)
    agcWeight = (agcWeight < 0.0041f) ? 0.004f : (agcWeight / 1.1f);
  else
    agcWeight = 0.2f;
}

V4l2Input::Memory * V4l2Input::nextImage(void)
{
  Q_ASSERT(mappedBuffers > 0);

  // Wait for next buffer
  timeval timeout;
  fd_set rdset;
  int n;

  FD_ZERO(&rdset);
  FD_SET(devDesc, &rdset);

  timeout.tv_sec = 0;
  timeout.tv_usec = 250000;

  n = select(devDesc + 1, &rdset, 0, 0, &timeout);
  if (n != -1)
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    v4l2_buffer cur_buf;
    memset(&cur_buf, 0, sizeof(cur_buf));

    cur_buf.type = buffers[0].type;
    cur_buf.memory = buffers[0].memory;

    if (ioctl(devDesc, VIDIOC_DQBUF, &cur_buf) >= 0)
    { // Got a buffer
      l.unlock();

      return new Memory(maps[cur_buf.index],
                        buffers[cur_buf.index].length,
                        cur_buf.index,
                        this);
    }
  }

  return NULL;
}

void V4l2Input::queueBuffer(int index)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (index < mappedBuffers)
  {
    if (::ioctl(devDesc, VIDIOC_QBUF, buffers + index) < 0)
      qWarning() << "V4l2Input: failed to ioctl device (VIDIOC_QBUF) " << index;
  }
}

#define FOURCC(a,b,c,d) (quint32(a) | (quint32(b) << 8) | (quint32(c) << 16) | (quint32(d) << 24))

quint32 V4l2Input::toV4L(SVideoFormat::Format format)
{
  switch (format)
  {
  case SVideoFormat::Format_GRAY8:              return FOURCC('G','R','E','Y');

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  case SVideoFormat::Format_RGB555:             return FOURCC('R','G','B','O');
  case SVideoFormat::Format_RGB565:             return FOURCC('R','G','B','P');
  case SVideoFormat::Format_RGB24:              return FOURCC('R','G','B','3');
  case SVideoFormat::Format_BGR24:              return FOURCC('B','G','R','3');
  case SVideoFormat::Format_RGB32:              return FOURCC('R','G','B','4');
  case SVideoFormat::Format_BGR32:              return FOURCC('B','G','R','4');
#else // RGB and BGR are deliberately swapped here.
  case SVideoFormat::Format_RGB555:             return FOURCC('B','G','R','O');
  case SVideoFormat::Format_RGB565:             return FOURCC('B','G','R','P');
  case SVideoFormat::Format_RGB24:              return FOURCC('B','G','R','3');
  case SVideoFormat::Format_BGR24:              return FOURCC('R','G','B','3');
  case SVideoFormat::Format_RGB32:              return FOURCC('B','G','R','4');
  case SVideoFormat::Format_BGR32:              return FOURCC('R','G','B','4');
#endif

  case SVideoFormat::Format_YUYV422:            return FOURCC('Y','U','Y','V');
  case SVideoFormat::Format_UYVY422:            return FOURCC('U','Y','V','Y');
  case SVideoFormat::Format_YUV410P:            return FOURCC('4','1','0','P');
  case SVideoFormat::Format_YUV411P:            return FOURCC('4','1','1','P');
  case SVideoFormat::Format_YUV420P:            return FOURCC('4','2','0','P');
  case SVideoFormat::Format_YUV422P:            return FOURCC('4','2','2','P');

  case SVideoFormat::Format_BGGR8:              return FOURCC('B','A','8','1');
  case SVideoFormat::Format_GBRG8:              return FOURCC('G','B','R','G');
  case SVideoFormat::Format_GRBG8:              return FOURCC('G','R','B','G');
  case SVideoFormat::Format_RGGB8:              return FOURCC('R','G','G','B');
  case SVideoFormat::Format_BGGR10:             return FOURCC('B','G','1','0');
  case SVideoFormat::Format_GBRG10:             return FOURCC('G','B','1','0');
  case SVideoFormat::Format_GRBG10:             return FOURCC('B','A','1','0');
  case SVideoFormat::Format_RGGB10:             return FOURCC('R','G','1','0');
  case SVideoFormat::Format_BGGR16:             return FOURCC('B','Y','R','2');

  default:                                      return 0;
  }
}

SVideoFormat::Format V4l2Input::fromV4L(quint32 format)
{
  switch (format)
  {
  case FOURCC('G','R','E','Y'): return SVideoFormat::Format_GRAY8;
  
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  case FOURCC('R','G','B','O'): return SVideoFormat::Format_RGB555;
  case FOURCC('R','G','B','P'): return SVideoFormat::Format_RGB565;
  case FOURCC('R','G','B','3'): return SVideoFormat::Format_RGB24;
  case FOURCC('B','G','R','3'): return SVideoFormat::Format_BGR24;
  case FOURCC('R','G','B','4'): return SVideoFormat::Format_RGB32;
  case FOURCC('B','G','R','4'): return SVideoFormat::Format_BGR32;
#else // RGB and BGR are deliberately swapped here.
  case FOURCC('B','G','R','O'): return SVideoFormat::Format_RGB555;
  case FOURCC('B','G','R','P'): return SVideoFormat::Format_RGB565;
  case FOURCC('B','G','R','3'): return SVideoFormat::Format_RGB24;
  case FOURCC('R','G','B','3'): return SVideoFormat::Format_BGR24;
  case FOURCC('B','G','R','4'): return SVideoFormat::Format_RGB32;
  case FOURCC('R','G','B','4'): return SVideoFormat::Format_BGR32;
#endif

  case FOURCC('Y','U','Y','V'): return SVideoFormat::Format_YUYV422;
  case FOURCC('U','Y','V','Y'): return SVideoFormat::Format_UYVY422;
  case FOURCC('4','1','0','P'): return SVideoFormat::Format_YUV410P;
  case FOURCC('4','1','1','P'): return SVideoFormat::Format_YUV411P;
  case FOURCC('4','2','0','P'): return SVideoFormat::Format_YUV420P;
  case FOURCC('4','2','2','P'): return SVideoFormat::Format_YUV422P;

  case FOURCC('B','A','8','1'): return SVideoFormat::Format_BGGR8;
  case FOURCC('G','B','R','G'): return SVideoFormat::Format_GBRG8;
  case FOURCC('G','R','B','G'): return SVideoFormat::Format_GRBG8;
  case FOURCC('R','G','G','B'): return SVideoFormat::Format_RGGB8;
  case FOURCC('B','G','1','0'): return SVideoFormat::Format_BGGR10;
  case FOURCC('G','B','1','0'): return SVideoFormat::Format_GBRG10;
  case FOURCC('B','A','1','0'): return SVideoFormat::Format_GRBG10;
  case FOURCC('R','G','1','0'): return SVideoFormat::Format_RGGB10;
  case FOURCC('B','Y','R','2'): return SVideoFormat::Format_BGGR16;

  default:                      return SVideoFormat::Format_Invalid;
  }
}

quint32 V4l2Input::toV4L(const SVideoCodec &codec)
{
  if      (codec == "MJPEG")                    return FOURCC('M','J','P','G');
  else if (codec == "LJPEG")                    return FOURCC('L','J','P','G');
  else if (codec == "FFVHUFF")                  return FOURCC('F','F','V','H');
  else if (codec == "HUFFYUV")                  return FOURCC('H','F','Y','U');
  else if (codec == "FFV1")                     return FOURCC('F','F','V','1');
  else if (codec == "MPEG1")                    return FOURCC('M','P','E','G');
  else if (codec == "MPEG2")                    return FOURCC('M','P','G','2');
  else if (codec == "MPEG4")                    return FOURCC('X','V','I','D');
  else if (codec == "DVVIDEO")                  return FOURCC('d','v','s','l');
  else                                          return 0;
}

#undef FOURCC

v4l2_field V4l2Input::toV4L(SVideoFormat::FieldMode fieldMode)
{
  switch(fieldMode)
  {
  case SVideoFormat::FieldMode_Invalid:               return V4L2_FIELD_ANY;
  case SVideoFormat::FieldMode_Progressive:           return V4L2_FIELD_NONE;
  case SVideoFormat::FieldMode_TopField:              return V4L2_FIELD_TOP;
  case SVideoFormat::FieldMode_BottomField:           return V4L2_FIELD_BOTTOM;
  case SVideoFormat::FieldMode_InterlacedTopFirst:    return V4L2_FIELD_INTERLACED;
  case SVideoFormat::FieldMode_InterlacedBottomFirst: return V4L2_FIELD_INTERLACED;
  case SVideoFormat::FieldMode_SequentialTopFirst:    return V4L2_FIELD_SEQ_TB;
  case SVideoFormat::FieldMode_SequentialBottomFirst: return V4L2_FIELD_SEQ_BT;
  case SVideoFormat::FieldMode_Alternating:           return V4L2_FIELD_ALTERNATE;
  }

  return V4L2_FIELD_ANY;
}


V4l2Input::Memory::Memory(char *data, int size, int bufferIndex, V4l2Input *parent)
  : SBuffer::Memory(size, data, size),
    bufferIndex(bufferIndex),
    parent(parent)
{
}

V4l2Input::Memory::~Memory()
{
  parent->queueBuffer(bufferIndex);
}


} } // End of namespaces
