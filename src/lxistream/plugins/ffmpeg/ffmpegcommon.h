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

#ifndef __FFMPEGCOMMON_H
#define __FFMPEGCOMMON_H

#include <QtCore>
#include <LXiStream>
#include <stdint.h>

extern "C"
{
#define UINT64_C Q_UINT64_C
#define INT64_C Q_INT64_C
#ifndef USE_FFMPEG_OLD_PATH
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#else
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#endif
}

namespace LXiStream {
namespace FFMpegBackend {

class FFMpegCommon
{
public:
  static void                   init(bool verbose = false);

  static void                   disableLog(bool);

  static ::CodecID              toFFMpegCodecID(const QByteArray &);
  inline static ::CodecID       toFFMpegCodecID(const QString &codec)           { return toFFMpegCodecID(codec.toAscii()); }
  static const char           * fromFFMpegCodecID(::CodecID);
  static ::PixelFormat          toFFMpegPixelFormat(SVideoFormat::Format);
  static SVideoFormat::Format   fromFFMpegPixelFormat(::PixelFormat);
  static int64_t                toFFMpegChannelLayout(SAudioFormat::Channels);
  static SAudioFormat::Channels fromFFMpegChannelLayout(int64_t, int);

  static ::AVPacket             toAVPacket(const SEncodedAudioBuffer &, const ::AVStream * = NULL);
  static ::AVPacket             toAVPacket(const SEncodedVideoBuffer &, const ::AVStream * = NULL);
  static ::AVPacket             toAVPacket(const SEncodedDataBuffer &, const ::AVStream * = NULL);

  static int                    decodeThreadCount(::CodecID);
  static int                    encodeThreadCount(::CodecID);
  static int                    execute(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);

private:
  static void                   log(void * ptr, int level, const char* fmt, va_list vl);
  static int                    lock(void **mutex, AVLockOp op);

private:
  static const int              threadLimit = 8;
  static int                    logLevel;
  static bool                   logDisabled;
};

} } // End of namespaces

#endif
