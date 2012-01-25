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

#include "datadecoder.h"
#include "bufferreader.h"
#include "networkbufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {


DataDecoder::DataDecoder(const QString &, QObject *parent)
  : SInterfaces::DataDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false)
{
}

DataDecoder::~DataDecoder()
{
  if (contextHandle && contextHandleOwner)
  {
    if (codecHandle)
      ::avcodec_close(contextHandle);

    ::av_free(contextHandle);
  }
}

bool DataDecoder::openCodec(const SDataCodec &c, SInterfaces::AbstractBufferReader *bufferReader, Flags)
{
  if (contextHandle)
    qFatal("DataDecoder already opened a codec.");

  inCodec = c;

  if ((codecHandle = avcodec_find_decoder(FFMpegCommon::toFFMpegCodecID(inCodec))) == NULL)
  {
    qCritical() << "DataDecoder: Data codec not found " << inCodec.codec();
    return false;
  }

  BufferReaderBase * ffBufferReader = qobject_cast<BufferReader *>(bufferReader);
  if (ffBufferReader == NULL)
    ffBufferReader = qobject_cast<NetworkBufferReader *>(bufferReader);

  if (ffBufferReader && (inCodec.streamId() >= 0))
  {
    contextHandle = ffBufferReader->getStream(inCodec.streamId())->codec;
    contextHandleOwner = false;
  }
  else
  {
    contextHandle = avcodec_alloc_context();
    contextHandleOwner = true;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    contextHandle->codec_type = AVMEDIA_TYPE_SUBTITLE;
#else
    contextHandle->codec_type = CODEC_TYPE_SUBTITLE;
#endif
    contextHandle->flags2 |= CODEC_FLAG2_CHUNKS;
  }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 6, 0)
  if (avcodec_open(contextHandle, codecHandle) < 0)
#else
  if (avcodec_open2(contextHandle, codecHandle, NULL) < 0)
#endif
  {
    qCritical() << "DataDecoder: Could not open data codec " << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  return true;
}

SDataBufferList DataDecoder::decodeBuffer(const SEncodedDataBuffer &dataBuffer)
{
  SDataBufferList output;
  if (!dataBuffer.isNull() && codecHandle)
  {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
    ::AVPacket packet = FFMpegCommon::toAVPacket(dataBuffer);
#endif

    const uint8_t *inPtr = reinterpret_cast<const uint8_t *>(dataBuffer.data());
    int inSize = dataBuffer.size();

    STime timeStamp = dataBuffer.presentationTimeStamp();
    if (!timeStamp.isValid())
      timeStamp = dataBuffer.decodingTimeStamp();

    while (inSize > 0)
    {
      int gotSubtitle = 0;
      ::AVSubtitle subtitle;

      // decode the subtitle
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      const int len = ::avcodec_decode_subtitle(contextHandle, &subtitle, &gotSubtitle, (uint8_t *)inPtr, inSize);
#else
      packet.data = (uint8_t *)inPtr;
      packet.size = inSize;
      const int len = ::avcodec_decode_subtitle2(contextHandle, &subtitle, &gotSubtitle, &packet);
#endif

      if (len >= 0)
      {
        if (gotSubtitle != 0)
        {
          QStringList text;
          QList<SSubpictureBuffer::Rect> rects;
          for (unsigned i=0; i<subtitle.num_rects; i++)
          if ((subtitle.rects[i]->type == ::SUBTITLE_BITMAP) ||
              ((subtitle.rects[i]->type == ::SUBTITLE_NONE) &&
               (subtitle.rects[i]->pict.data[0] != NULL) &&
               (subtitle.rects[i]->pict.data[1] != NULL)))
          {
            SSubpictureBuffer::Rect rect;
            rect.x = subtitle.rects[i]->x;
            rect.y = subtitle.rects[i]->y;
            rect.width = subtitle.rects[i]->w;
            rect.height = subtitle.rects[i]->h;
            rect.lineStride = SBuffer::align(rect.width, SBuffer::minimumAlignVal);
            rect.paletteSize = subtitle.rects[i]->nb_colors;

            rects += rect;
          }
          else if (subtitle.rects[i]->type == ::SUBTITLE_TEXT)
            text += QString::fromUtf8(subtitle.rects[i]->text);

          if (!rects.isEmpty())
          {
            SSubpictureBuffer buffer(rects);
            for (unsigned i=0, rectId = 0; (i<subtitle.num_rects) && (rectId<unsigned(rects.count())); i++)
            if (((subtitle.rects[i]->type == ::SUBTITLE_BITMAP) ||
                 (subtitle.rects[i]->type == ::SUBTITLE_NONE)) &&
                (subtitle.rects[i]->pict.data[0] != NULL) &&
                (subtitle.rects[i]->pict.data[1] != NULL))
            {
              const SSubpictureBuffer::Rect rect = rects[rectId];

              // Copy palette and make sure it is monochrome.
              const SPixels::RGBAPixel * const srcPalette = reinterpret_cast<const SPixels::RGBAPixel *>(subtitle.rects[i]->pict.data[1]);
              SPixels::RGBAPixel * const dstPalette = buffer.palette(rectId);
              for (int j=0; j<rect.paletteSize; j++)
              {
                dstPalette[j].r = dstPalette[j].g = dstPalette[j].b =
                    qMax(srcPalette[j].r, qMax(srcPalette[j].g, srcPalette[j].b));

                dstPalette[j].a = srcPalette[j].a;
              }

              // And copy the lines
              const int srcLineStride = qMax(subtitle.rects[i]->w, subtitle.rects[i]->pict.linesize[0]);
              quint8 * const lines = buffer.lines(rectId);
              for (int y=0; y<subtitle.rects[i]->h; y++)
              {
                const quint8 * const srcLine = subtitle.rects[i]->pict.data[0] + (srcLineStride * y);
                quint8 * const dstLine = lines + (rect.lineStride * y);

                memcpy(dstLine, srcLine, rect.width);
              }

              rectId++;
            }

            if (dataBuffer.presentationTimeStamp().isValid())
              buffer.setTimeStamp(dataBuffer.presentationTimeStamp() + STime::fromMSec(subtitle.start_display_time));
            else if (dataBuffer.decodingTimeStamp().isValid())
              buffer.setTimeStamp(dataBuffer.decodingTimeStamp() + STime::fromMSec(subtitle.start_display_time));

            buffer.setDuration(STime::fromMSec(subtitle.end_display_time - subtitle.start_display_time));

            output += buffer;
          }

          if (!text.isEmpty())
          {
            SSubtitleBuffer buffer(text);

            if (dataBuffer.presentationTimeStamp().isValid())
              buffer.setTimeStamp(dataBuffer.presentationTimeStamp() + STime::fromMSec(subtitle.start_display_time));
            else if (dataBuffer.decodingTimeStamp().isValid())
              buffer.setTimeStamp(dataBuffer.decodingTimeStamp() + STime::fromMSec(subtitle.start_display_time));

            buffer.setDuration(STime::fromMSec(subtitle.end_display_time - subtitle.start_display_time));

            output += buffer;
          }
        }

        inPtr += len;
        inSize -= len;
        continue;
      }

      break;
    }
  }
  else
    output << SDataBuffer(); // Flush

  return output;
}


} } // End of namespaces
