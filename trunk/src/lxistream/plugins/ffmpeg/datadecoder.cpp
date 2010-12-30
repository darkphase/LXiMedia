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
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

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

  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

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

SSubtitleBufferList DataDecoder::decodeBuffer(const SEncodedDataBuffer &dataBuffer)
{
  SSubtitleBufferList output;
  if (!dataBuffer.isNull())
  {
    if (codecHandle)
    {
      const uint8_t *inPtr = reinterpret_cast<const uint8_t *>(dataBuffer.data());
      int inSize = dataBuffer.size();

      STime timeStamp = dataBuffer.presentationTimeStamp();
      if (!timeStamp.isValid())
        timeStamp = dataBuffer.decodingTimeStamp();

      while (inSize > 0)
      {
        int gotSubtitle = 0;
        AVSubtitle subtitle;

        // decode the subtitle
        int len = avcodec_decode_subtitle(contextHandle, &subtitle, &gotSubtitle, (uint8_t *)inPtr, inSize);
        if (len >= 0)
        {
          if (gotSubtitle != 0)
          {
          }

          inPtr += len;
          inSize -= len;
          continue;
        }

        break;
      }
    }
  }

  return output;
}


} } // End of namespaces
