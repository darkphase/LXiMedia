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

#include "deinterlace.h"

// Implemented in deinterlace.mix.c
extern "C" void LXiStream_Common_Deinterlace_mixFields(const char *, char *, const char *, unsigned);

namespace LXiStream {
namespace Common {


SVideoBufferList DeinterlaceBlend::processBuffer(const SVideoBuffer &videoBuffer)
{
  SVideoBuffer videoBufferA = videoBuffer;

  if (!videoBufferA.isNull())
  {
    SVideoFormat format = videoBufferA.format();
    const SVideoFormat::FieldMode fieldMode = format.fieldMode();
    format.setFieldMode(SVideoFormat::FieldMode_Progressive);

    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst))
    {
      const SSize size = format.size();

      if ((format == SVideoFormat::Format_YUYV422) ||
          (format == SVideoFormat::Format_UYVY422))
      {
        videoBufferA.overrideFormat(format);

        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }
      }
      else if ((format == SVideoFormat::Format_YUV410P) ||
               (format == SVideoFormat::Format_YUV411P) ||
               (format == SVideoFormat::Format_YUV420P) ||
               (format == SVideoFormat::Format_YUV422P) ||
               (format == SVideoFormat::Format_YUV444P))
      {
        videoBufferA.overrideFormat(format);

        // Y
        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }

        int wr = 1, hr = 1;
        if (format.planarYUVRatio(wr, hr))
        if (hr == 1)
        {
          // U
          for (int i=1; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferA.scanLine(i - 1, 1);
            char * const line1 = videoBufferA.scanLine(i, 1);
            const char * const line2 = videoBufferA.scanLine(i + 1, 1);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(1));
          }

          // V
          for (int i=1; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferA.scanLine(i - 1, 2);
            char * const line1 = videoBufferA.scanLine(i, 2);
            const char * const line2 = videoBufferA.scanLine(i + 1, 2);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(2));
          }
        }
      }
      else if ((format == SVideoFormat::Format_BGR32) ||
               (format == SVideoFormat::Format_RGB32))
      {
        videoBufferA.overrideFormat(format);

        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }
      }
    }

    return SVideoBufferList() << videoBufferA;
  }

  return SVideoBufferList();
}

SVideoBufferList DeinterlaceBob::processBuffer(const SVideoBuffer &videoBuffer)
{
  SVideoBuffer videoBufferA = videoBuffer;

  if (!videoBufferA.isNull())
  {
    const STime timeStamp = videoBufferA.timeStamp();
    const STime frameTime =
        (videoBufferA.format().frameRate().isValid())
        ? STime(1, videoBufferA.format().frameRate())
        : STime(1, SInterval::fromFrequency(25));

    SVideoBuffer videoBufferB = videoBufferA;

    SVideoFormat format = videoBufferA.format();
    const SVideoFormat::FieldMode fieldMode = format.fieldMode();
    format.setFieldMode(SVideoFormat::FieldMode_Progressive);

    if ((fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst) ||
        (fieldMode == SVideoFormat::FieldMode_InterlacedBottomFirst))
    {
      const SSize size = format.size();

      if ((format == SVideoFormat::Format_YUYV422) ||
          (format == SVideoFormat::Format_UYVY422))
      {
        videoBufferA.overrideFormat(format);
        videoBufferB.overrideFormat(format);

        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }

        for (int i=2; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferB.scanLine(i - 1, 0);
          char * const line1 = videoBufferB.scanLine(i, 0);
          const char * const line2 = videoBufferB.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferB.lineSize(0));
        }
      }
      else if ((format == SVideoFormat::Format_YUV410P) ||
               (format == SVideoFormat::Format_YUV411P) ||
               (format == SVideoFormat::Format_YUV420P) ||
               (format == SVideoFormat::Format_YUV422P) ||
               (format == SVideoFormat::Format_YUV444P))
      {
        videoBufferA.overrideFormat(format);
        videoBufferB.overrideFormat(format);

        // Y
        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }

        for (int i=2; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferB.scanLine(i - 1, 0);
          char * const line1 = videoBufferB.scanLine(i, 0);
          const char * const line2 = videoBufferB.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferB.lineSize(0));
        }

        int wr = 1, hr = 1;
        if (format.planarYUVRatio(wr, hr))
        if (hr == 1)
        {
          // U
          for (int i=1; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferA.scanLine(i - 1, 1);
            char * const line1 = videoBufferA.scanLine(i, 1);
            const char * const line2 = videoBufferA.scanLine(i + 1, 1);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(1));
          }

          for (int i=2; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferB.scanLine(i - 1, 1);
            char * const line1 = videoBufferB.scanLine(i, 1);
            const char * const line2 = videoBufferB.scanLine(i + 1, 1);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferB.lineSize(1));
          }

          // V
          for (int i=1; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferA.scanLine(i - 1, 2);
            char * const line1 = videoBufferA.scanLine(i, 2);
            const char * const line2 = videoBufferA.scanLine(i + 1, 2);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(2));
          }

          for (int i=2; i<size.height()-1; i+=2)
          {
            const char * const line0 = videoBufferB.scanLine(i - 1, 2);
            char * const line1 = videoBufferB.scanLine(i, 2);
            const char * const line2 = videoBufferB.scanLine(i + 1, 2);

            LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferB.lineSize(2));
          }
        }
      }
      else if ((format == SVideoFormat::Format_BGR32) ||
               (format == SVideoFormat::Format_RGB32))
      {
        videoBufferA.overrideFormat(format);
        videoBufferB.overrideFormat(format);

        for (int i=1; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferA.scanLine(i - 1, 0);
          char * const line1 = videoBufferA.scanLine(i, 0);
          const char * const line2 = videoBufferA.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferA.lineSize(0));
        }

        for (int i=2; i<size.height()-1; i+=2)
        {
          const char * const line0 = videoBufferB.scanLine(i - 1, 0);
          char * const line1 = videoBufferB.scanLine(i, 0);
          const char * const line2 = videoBufferB.scanLine(i + 1, 0);

          LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, videoBufferB.lineSize(0));
        }
      }
    }

    if (fieldMode == SVideoFormat::FieldMode_InterlacedTopFirst)
    {
      videoBufferA.setTimeStamp(timeStamp);
      videoBufferB.setTimeStamp(timeStamp + (frameTime / 2));

      return SVideoBufferList() << videoBufferA << videoBufferB;
    }
    else
    {
      videoBufferA.setTimeStamp(timeStamp + (frameTime / 2));
      videoBufferB.setTimeStamp(timeStamp);

      return SVideoBufferList() << videoBufferB << videoBufferA;
    }
  }

  return SVideoBufferList();
}


} } // End of namespaces
