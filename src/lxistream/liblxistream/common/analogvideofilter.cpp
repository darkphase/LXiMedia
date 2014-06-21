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

#include "analogvideofilter.h"

// Implemented in deinterlace.mix.c
extern "C" void LXiStream_AnalogVideoFilter_mixLine(uchar *, const uchar *, quint32, unsigned);
extern "C" void LXiStream_AnalogVideoFilter_conbrg(uchar *, const uchar *, quint8, quint8, unsigned);
extern "C" void LXiStream_AnalogVideoFilter_conbrg_YUYV(uchar *, const uchar *, quint8, quint8, unsigned);
extern "C" void LXiStream_AnalogVideoFilter_conbrg_UYVY(uchar *, const uchar *, quint8, quint8, unsigned);

namespace LXiStream {
namespace Common {


AnalogVideoFilter::AnalogVideoFilter(QObject *parent)
    : SNodes::Video::AnalogVideoFilter(Behavior_CpuProcessing, SVideoBuffer::baseTypeId,
           SCodecList() << SVideoCodec::Format_YUYV422
                        << SVideoCodec::Format_UYVY422
                        << SVideoCodec::Format_YUV410P
                        << SVideoCodec::Format_YUV411P
                        << SVideoCodec::Format_YUV420P
                        << SVideoCodec::Format_YUV422P
                        << SVideoCodec::Format_YUV444P
                        << SVideoCodec::Format_RGB32
                        << SVideoCodec::Format_BGR32,
            parent),
      tempBuffer(),
      minVal(0), maxVal(255),
      count(0)
{
}

bool AnalogVideoFilter::prepare(const SCodecList &)
{
  return true;
}

bool AnalogVideoFilter::unprepare(void)
{
  tempBuffer = SBuffer();

  return true;
}

#define MINMAX(pos) \
  if (__builtin_expect(++count >= 25, false)) \
  { \
    quint8 mi = 255, ma = 0; \
    for (int y=0; y<size.height(); y+=8) \
    { \
      const uchar * const tmp = tempBuffer.scanLine(y, 0); \
      for (int x=0; x<size.width(); x+=8) \
      { \
        const quint8 val = pos; \
        mi = qMin(val, mi); \
        ma = qMax(val, ma); \
      } \
    } \
    const int mid = int(mi) - int(minVal); \
    if (qAbs(mid) >= 16) minVal = mi; \
    else if ((mid <= -2) && (minVal > 0)) minVal--; \
    else if ((mid >= 2) && (minVal < 96)) minVal++; \
    const int mad = int(ma) - int(maxVal); \
    if (qAbs(mid) >= 16) maxVal = ma; \
    else if ((mad <= -2) && (maxVal < 255)) maxVal++; \
    else if ((mad >= 2) && (maxVal > 160)) maxVal--; \
    count = 0; \
  }

SNode::Result AnalogVideoFilter::processBuffer(const SBuffer &input, SBufferList &output)
{
  SVideoBuffer videoBuffer = input;
  if (!videoBuffer.isNull())
  {
    const SVideoCodec codec = videoBuffer.codec();
    if (codec.isCompressed())
    {
      // Can not process an encoded buffer, just passtrough.
      output << videoBuffer;
      return Result_Active;
    }

    const SSize size = codec.size();
    const SVideoCodec::Format format = codec.format();

    if (!tempBuffer.isNull())
    {
      const SVideoCodec tempCodec = tempBuffer.codec();
      const SSize tempSize = tempCodec.size();

      if ((size.width() != tempSize.width()) ||
          (size.height() != tempSize.height()) ||
          (format != tempCodec.format()) ||
          !qFuzzyCompare(size.aspectRatio(), size.aspectRatio()))
      {
        tempBuffer = videoBuffer;
      }
    }
    else
      tempBuffer = videoBuffer;

    const quint8 cmi = (minVal >= 16) ? (minVal - 16) : 0;
    const quint8 cma = (maxVal < 240) ? (maxVal + 16) : 255;

    if (format == SVideoCodec::Format_YUYV422)
    {
      for (int y=0; y<size.height(); y++)
      {
        uchar * const src = videoBuffer.scanLine(y, 0);
        uchar * const tmp = tempBuffer.scanLine(y, 0);

        LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x04020402, videoBuffer.lineSize(0));
        LXiStream_AnalogVideoFilter_conbrg_YUYV(src, tmp, cmi, cma, videoBuffer.lineSize(0));
      }

      MINMAX(tmp[x*2]);

      output << videoBuffer;
    }
    else if (format == SVideoCodec::Format_UYVY422)
    {
      for (int y=0; y<size.height(); y++)
      {
        uchar * const src = videoBuffer.scanLine(y, 0);
        uchar * const tmp = tempBuffer.scanLine(y, 0);

        LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x02040204, videoBuffer.lineSize(0));
        LXiStream_AnalogVideoFilter_conbrg_UYVY(src, tmp, cmi, cma, videoBuffer.lineSize(0));
      }

      MINMAX(tmp[(x*2)+1]);

      output << videoBuffer;
    }
    else if ((format == SVideoCodec::Format_YUV410P) ||
             (format == SVideoCodec::Format_YUV411P) ||
             (format == SVideoCodec::Format_YUV420P) ||
             (format == SVideoCodec::Format_YUV422P) ||
             (format == SVideoCodec::Format_YUV444P))
    {
      // Y
      for (int y=0; y<size.height(); y++)
      {
        uchar * const src = videoBuffer.scanLine(y, 0);
        uchar * const tmp = tempBuffer.scanLine(y, 0);

        LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x02020202, videoBuffer.lineSize(0));
        LXiStream_AnalogVideoFilter_conbrg(src, tmp, cmi, cma, videoBuffer.lineSize(0));
      }

      unsigned wr = 1, hr = 1;
      if (codec.planarYUVRatio(wr, hr))
      if (hr == 1)
      {
        // U
        for (int y=0; y<size.height(); y++)
        {
          uchar * const src = videoBuffer.scanLine(y, 1);
          uchar * const tmp = tempBuffer.scanLine(y, 1);

          LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x04040404, videoBuffer.lineSize(1));
          memcpy(src, tmp, videoBuffer.lineSize(1));
        }

        // V
        for (int y=0; y<size.height(); y++)
        {
          uchar * const src = videoBuffer.scanLine(y, 2);
          uchar * const tmp = tempBuffer.scanLine(y, 2);

          LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x04040404, videoBuffer.lineSize(2));
          memcpy(src, tmp, videoBuffer.lineSize(2));
        }
      }

      MINMAX(tmp[x]);

      output << videoBuffer;
    }
    else if (format == SVideoCodec::Format_RGB32)
    {
      for (int y=0; y<size.height(); y++)
      {
        uchar * const src = videoBuffer.scanLine(y, 0);
        uchar * const tmp = tempBuffer.scanLine(y, 0);

        LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x00040408, videoBuffer.lineSize(0));
        LXiStream_AnalogVideoFilter_conbrg(src, tmp, cmi, cma, videoBuffer.lineSize(0));
      }

      MINMAX(qMax(quint8((tmp[(x*4)+1]>>1)+(tmp[(x*4)+2]>>1)), qMax(tmp[(x*4)+1], tmp[(x*4)+2])));

      output << videoBuffer;
    }
    else if (format == SVideoCodec::Format_BGR32)
    {
      for (int y=0; y<size.height(); y++)
      {
        uchar * const src = videoBuffer.scanLine(y, 0);
        uchar * const tmp = tempBuffer.scanLine(y, 0);

        LXiStream_AnalogVideoFilter_mixLine(tmp, src, 0x00080404, videoBuffer.lineSize(0));
        LXiStream_AnalogVideoFilter_conbrg(src, tmp, cmi, cma, videoBuffer.lineSize(0));
      }

      MINMAX(qMax(quint8((tmp[(x*4)+2]>>1)+(tmp[(x*4)+3]>>1)), qMax(tmp[(x*4)+2], tmp[(x*4)+3])));

      output << videoBuffer;
    }
    else
      output << videoBuffer;

    return Result_Active;
  }

  return Result_Idle;
}


} } // End of namespaces
