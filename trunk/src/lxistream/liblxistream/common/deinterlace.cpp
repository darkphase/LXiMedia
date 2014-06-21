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

#include "deinterlace.h"
#include "../../algorithms/deinterlace.h"

namespace LXiStream {
namespace Common {


SVideoBufferList DeinterlaceBlend::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    SVideoFormat format = videoBuffer.format();
    const SVideoFormat::FieldMode fieldMode = format.fieldMode();
    const SSize size = format.size();

    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst))
    {
      format.setFieldMode(SVideoFormat::FieldMode_Progressive);
      SVideoBuffer destBuffer(format);

      if ((format == SVideoFormat::Format_YUYV422) ||
          (format == SVideoFormat::Format_UYVY422) ||
          (format == SVideoFormat::Format_BGR32) ||
          (format == SVideoFormat::Format_RGB32))
      {
        copyLines(destBuffer, videoBuffer, 0);
      }
      else if ((format == SVideoFormat::Format_YUV410P) ||
               (format == SVideoFormat::Format_YUV411P) ||
               (format == SVideoFormat::Format_YUV420P) ||
               (format == SVideoFormat::Format_YUV422P) ||
               (format == SVideoFormat::Format_YUV444P))
      {
        copyLines(destBuffer, videoBuffer, 0); // Y

        int wr = 1, hr = 1;
        if (format.planarYUVRatio(wr, hr))
        {
          if (hr == 1)
          {
            copyLines(destBuffer, videoBuffer, 1); // U
            copyLines(destBuffer, videoBuffer, 2); // V
          }
          else for (int y=0, n=size.height()/hr; y<n; y++)
          {
            memcpy(destBuffer.scanLine(y, 1), videoBuffer.scanLine(y, 1), destBuffer.lineSize(1)); // U
            memcpy(destBuffer.scanLine(y, 2), videoBuffer.scanLine(y, 2), destBuffer.lineSize(2)); // V
          }
        }
      }

      destBuffer.setTimeStamp(videoBuffer.timeStamp());

      return SVideoBufferList() << destBuffer;
    }
    else
      return SVideoBufferList() << videoBuffer;
  }

  return SVideoBufferList();
}

void DeinterlaceBlend::copyLines(SVideoBuffer &destBuffer, const SVideoBuffer &videoBuffer, int plane)
{
  const SSize size = destBuffer.format().size();
  if (size.height() > 1)
  {
    memcpy(
        destBuffer.scanLine(0, plane),
        videoBuffer.scanLine(0, plane),
        destBuffer.lineSize(0));

    for (int y=1, n=size.height()-1; y<n; y+=2)
    {
      Algorithms::Deinterlace::smartBlendFields(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y - 1, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y + 1, plane)),
          destBuffer.lineSize(plane));

      memcpy(
          destBuffer.scanLine(y + 1, plane),
          videoBuffer.scanLine(y + 1, plane),
          destBuffer.lineSize(plane));
    }

    if ((size.height() & 1) == 0)
    {
      memcpy(
          destBuffer.scanLine(size.height() - 1, plane),
          videoBuffer.scanLine(size.height() - 1, plane),
          destBuffer.lineSize(plane));
    }
  }
}

SVideoBufferList DeinterlaceBob::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    const STime timeStamp = videoBuffer.timeStamp();
    const STime frameTime =
        (videoBuffer.format().frameRate().isValid())
        ? STime(1, videoBuffer.format().frameRate())
        : STime(1, SInterval::fromFrequency(25));

    SVideoFormat format = videoBuffer.format();
    const SVideoFormat::FieldMode fieldMode = format.fieldMode();
    const SSize size = format.size();

    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst))
    {
      format.setFieldMode(SVideoFormat::FieldMode_Progressive);
      SVideoBuffer destBufferA(format);
      SVideoBuffer destBufferB(format);

      if ((format == SVideoFormat::Format_YUYV422) ||
          (format == SVideoFormat::Format_UYVY422) ||
          (format == SVideoFormat::Format_BGR32) ||
          (format == SVideoFormat::Format_RGB32))
      {
        copyLines(destBufferA, videoBuffer, 1, 0);
        copyLines(destBufferB, videoBuffer, 2, 0);
      }
      else if ((format == SVideoFormat::Format_YUV410P) ||
               (format == SVideoFormat::Format_YUV411P) ||
               (format == SVideoFormat::Format_YUV420P) ||
               (format == SVideoFormat::Format_YUV422P) ||
               (format == SVideoFormat::Format_YUV444P))
      {
        copyLines(destBufferA, videoBuffer, 1, 0); // Y
        copyLines(destBufferB, videoBuffer, 2, 0); // Y

        int wr = 1, hr = 1;
        if (format.planarYUVRatio(wr, hr))
        {
          if (hr == 1)
          {
            copyLines(destBufferA, videoBuffer, 1, 1); // U
            copyLines(destBufferB, videoBuffer, 2, 1); // V

            copyLines(destBufferA, videoBuffer, 1, 2); // U
            copyLines(destBufferB, videoBuffer, 2, 2); // V
          }
          else for (int y=0, n=size.height()/hr; y<n; y++)
          {
            memcpy(destBufferA.scanLine(y, 1), videoBuffer.scanLine(y, 1), destBufferA.lineSize(1)); // U
            memcpy(destBufferA.scanLine(y, 2), videoBuffer.scanLine(y, 2), destBufferA.lineSize(2)); // V

            memcpy(destBufferB.scanLine(y, 1), videoBuffer.scanLine(y, 1), destBufferB.lineSize(1)); // U
            memcpy(destBufferB.scanLine(y, 2), videoBuffer.scanLine(y, 2), destBufferB.lineSize(2)); // V
          }
        }
      }


      if (fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst)
      {
        destBufferA.setTimeStamp(timeStamp);
        destBufferB.setTimeStamp(timeStamp + (frameTime / 2));

        return SVideoBufferList() << destBufferA << destBufferB;
      }
      else
      {
        destBufferA.setTimeStamp(timeStamp + (frameTime / 2));
        destBufferB.setTimeStamp(timeStamp);

        return SVideoBufferList() << destBufferB << destBufferA;
      }
    }
    else
      return SVideoBufferList() << videoBuffer;
  }

  return SVideoBufferList();
}

void DeinterlaceBob::copyLines(SVideoBuffer &destBuffer, const SVideoBuffer &videoBuffer, int offset, int plane)
{
  const SSize size = destBuffer.format().size();
  if (size.height() > 1)
  {
    memcpy(
        destBuffer.scanLine(0, plane),
        videoBuffer.scanLine(0, plane),
        destBuffer.lineSize(0));

    for (int y=offset, n=size.height()-1; y<n; y+=2)
    {
      Algorithms::Deinterlace::smartBlendFields(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y - 1, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, plane)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y + 1, plane)),
          destBuffer.lineSize(plane));

      memcpy(
          destBuffer.scanLine(y + 1, plane),
          videoBuffer.scanLine(y + 1, plane),
          destBuffer.lineSize(plane));
    }

    if ((size.height() & 1) != (offset & 1))
    {
      memcpy(
          destBuffer.scanLine(size.height() - 1, plane),
          videoBuffer.scanLine(size.height() - 1, plane),
          destBuffer.lineSize(plane));
    }
  }
}


} } // End of namespaces
