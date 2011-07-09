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

#include "videoresizer.h"
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

VideoResizer::VideoResizer(const QString &scheme, QObject *parent)
  : SInterfaces::VideoResizer(parent),
    filterFlags(algoFlags(scheme)),
    scaleSize(),
    scaleAspectRatioMode(Qt::KeepAspectRatio),
    lastFormat(SVideoFormat::Format_Invalid),
    destFormat(SVideoFormat::Format_Invalid),
    numContexts(1),
    swsContext(NULL)
{
}

VideoResizer::~VideoResizer()
{
  freeContext();
}

int VideoResizer::algoFlags(const QString &name)
{
  if (name == "lanczos")
    return SWS_LANCZOS;
  else if (name == "bicubic")
    return SWS_BICUBIC;
  else // if (name == "bilinear")
#ifndef Q_OS_MACX
    return SWS_FAST_BILINEAR; // Crashes on Mac
#else
    return SWS_BILINEAR;
#endif
}

void VideoResizer::setSize(const SSize &size)
{
  scaleSize = size;
}

SSize VideoResizer::size(void) const
{
  return scaleSize;
}

void VideoResizer::setAspectRatioMode(Qt::AspectRatioMode a)
{
  scaleAspectRatioMode = a;
}

Qt::AspectRatioMode VideoResizer::aspectRatioMode(void) const
{
  return scaleAspectRatioMode;
}

bool VideoResizer::needsResize(const SVideoFormat &format)
{
  if (lastFormat != format)
  {
    lastFormat = format;

    QSize size = lastFormat.size().absoluteSize();
    size.scale(scaleSize.absoluteSize(), scaleAspectRatioMode);
    if (scaleSize.aspectRatio() > 1.0)
      size.setWidth(int(size.width() / scaleSize.aspectRatio()));
    else if (scaleSize.aspectRatio() < 1.0)
      size.setHeight(int(size.height() / (1.0f / scaleSize.aspectRatio())));

    destFormat = SVideoFormat(
        lastFormat.format(),
        SSize(unsigned(size.width()) & ~0x0Fu, size.height()),
        lastFormat.frameRate(),
        lastFormat.fieldMode());

    freeContext();

    if (destFormat.size() != format.size())
    {
      const ::PixelFormat pf = FFMpegCommon::toFFMpegPixelFormat(lastFormat);

      swsContext = new ::SwsContext *[numContexts];
      for (int i=0; i<numContexts; i++)
        swsContext[i] = NULL;

      for (int i=0; i<numContexts; i++)
      {
        swsContext[i] = ::sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                         destFormat.size().width(), destFormat.size().height(), pf,
                                         filterFlags,
                                         NULL, NULL, NULL);
        if (swsContext[i] == NULL)
        { // Fallback
          swsContext[i] = ::sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                           destFormat.size().width(), destFormat.size().height(), pf,
                                           0,
                                           NULL, NULL, NULL);
        }

        if (swsContext[i] == NULL)
        {
          freeContext();
          break;
        }
      }
    }
  }

  return swsContext != NULL;
}

SVideoBuffer VideoResizer::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!scaleSize.isNull() && !videoBuffer.isNull() && VideoResizer::needsResize(videoBuffer.format()))
  {
    SVideoBuffer destBuffer(destFormat);

    quint8      * dest[4]         = { (quint8 *)destBuffer.scanLine(0, 0),
                                      (quint8 *)destBuffer.scanLine(0, 1),
                                      (quint8 *)destBuffer.scanLine(0, 2),
                                      (quint8 *)destBuffer.scanLine(0, 3) };
    int           dstLineSize[4]  = { destBuffer.lineSize(0),
                                      destBuffer.lineSize(1),
                                      destBuffer.lineSize(2),
                                      destBuffer.lineSize(3) };
    bool success = true;
    if (numContexts > 1)
    {
      QList< QFuture<bool> > futures;
      for (int i=0; i<numContexts; i++)
        futures.append(QtConcurrent::run(this, &VideoResizer::processSlice, i, videoBuffer, (quint8 **)dest, (int *)dstLineSize));

      for (int i=0; i<futures.count(); i++)
      {
        futures[i].waitForFinished();
        success &= futures[i].result();
      }
    }
    else
      success = processSlice(0, videoBuffer, dest, dstLineSize);

    if (success)
    {
      destBuffer.setTimeStamp(videoBuffer.timeStamp());
      return destBuffer;
    }
  }

  return videoBuffer;
}

bool VideoResizer::processSlice(int slice, const SVideoBuffer &videoBuffer, quint8 **dest, int *dstLineSize)
{
  int wf = 0, hf = 0;
  const bool planar = destFormat.planarYUVRatio(wf, hf);
  if (!planar)
    wf = hf = 1;

  const int yheight = lastFormat.size().height();
  const int yslice = (((yheight / numContexts) + 15) & ~0x0F);
  const int ytop = yslice * slice, uvtop = ytop / hf;
  const int yoverlap = (filterFlags & SWS_LANCZOS) ? 12 : ((filterFlags & SWS_BICUBIC) ? 6 : 2);

  quint8      * source[4]       = { (quint8 *)videoBuffer.scanLine(ytop, 0),
                                    (quint8 *)videoBuffer.scanLine(uvtop, 1),
                                    (quint8 *)videoBuffer.scanLine(uvtop, 2),
                                    (quint8 *)videoBuffer.scanLine(uvtop, 3) };
  int           srcLineSize[4]  = { videoBuffer.lineSize(0),
                                    videoBuffer.lineSize(1),
                                    videoBuffer.lineSize(2),
                                    videoBuffer.lineSize(3) };

  const bool result =
      ::sws_scale(swsContext[slice], source, srcLineSize,
                  ytop, (slice < (numContexts - 1)) ? (yslice + yoverlap) : (yheight - ytop),
                  dest, dstLineSize) >= 0;

  // Correct chroma lines on the top
  if ((slice == 0) && planar && destFormat.isYUV() && dest[1] && dest[2] && result)
  {
    const int skip = 4 / hf;

    uint8_t * const ref[2] = {
      dest[1] + (dstLineSize[1] * skip),
      dest[2] + (dstLineSize[2] * skip) };

    for (int y=0; y<skip-1; y++)
    {
      memcpy(dest[1] + (dstLineSize[1] * y), ref[0], dstLineSize[1]);
      memcpy(dest[2] + (dstLineSize[2] * y), ref[1], dstLineSize[2]);
    }
  }

  return result;
}

void VideoResizer::freeContext(void)
{
  if (swsContext)
  {
    for (int i=0; i<numContexts; i++)
      ::sws_freeContext(swsContext[i]);

    delete [] swsContext;
    swsContext = NULL;
  }
}

} } // End of namespaces
