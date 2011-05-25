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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

// This enables multithreaded encoding and decoding.
#define OPT_ENABLE_THREADS

// This enables resending the last encoded buffer in case the same buffer is
// encoded twice (only if encoding intra-frames only).
#define OPT_RESEND_LAST_FRAME

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

#ifdef OPT_ENABLE_THREADS
  static int                    decodeThreadCount(::CodecID);
  static int                    encodeThreadCount(::CodecID);
  static int                    execute(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  static int                    execute2(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
#endif
#endif

private:
#ifdef OPT_ENABLE_THREADS
  static int                    executeTask(int (*func)(::AVCodecContext *, void *), ::AVCodecContext *c, void *arg);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  static int                    execute2Task(int (*func)(::AVCodecContext *, void *, int, int), ::AVCodecContext *c, void *arg, int jobnr, int threadnr);
#endif
#endif

  static void                   log(void * ptr, int level, const char* fmt, va_list vl);
  static int                    lock(void **mutex, AVLockOp op);

private:
  static const int              threadLimit = 8;
  static int                    logLevel;
  static bool                   logDisabled;
};

} } // End of namespaces

#endif
