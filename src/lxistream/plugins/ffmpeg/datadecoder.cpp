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

#include "datadecoder.h"

namespace LXiStream {
namespace FFMpegBackend {


DataDecoder::DataDecoder(const QString &, QObject *parent)
  : SInterfaces::DataDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL)
{
}

DataDecoder::~DataDecoder()
{
  if (codecHandle && contextHandle)
    avcodec_close(contextHandle);

  if (contextHandle)
  {
    if (contextHandle->extradata)
      delete [] contextHandle->extradata;

    av_free(contextHandle);
  }
}

bool DataDecoder::openCodec(const SDataCodec &c, Flags)
{
  if (contextHandle)
    qFatal("DataDecoder already opened a codec.");

  inCodec = c;

  if ((codecHandle = avcodec_find_decoder(FFMpegCommon::toFFMpegCodecID(inCodec))) == NULL)
  {
    qCritical() << "DataDecoder: Data codec not found " << inCodec.codec();
    return false;
  }

  contextHandle = avcodec_alloc_context();
  contextHandle->codec_type = CODEC_TYPE_SUBTITLE;

  contextHandle->extradata_size = inCodec.extraData().size();
  if (contextHandle->extradata_size > 0)
  {
    contextHandle->extradata = new uint8_t[contextHandle->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
    memcpy(contextHandle->extradata, inCodec.extraData().data(), contextHandle->extradata_size);
    memset(contextHandle->extradata + contextHandle->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
  }
  else
    contextHandle->extradata = NULL;

  if (avcodec_open(contextHandle, codecHandle) < 0)
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

              memcpy(buffer.palette(rectId),
                     reinterpret_cast<const SPixels::RGBAPixel *>(subtitle.rects[i]->pict.data[1]),
                     rect.paletteSize * sizeof(SPixels::RGBAPixel));

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

  return output;
}


} } // End of namespaces
