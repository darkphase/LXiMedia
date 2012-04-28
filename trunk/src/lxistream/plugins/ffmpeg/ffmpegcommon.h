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

#ifndef __FFMPEGCOMMON_H
#define __FFMPEGCOMMON_H

#include <QtCore>
#include <LXiStream>
#include <stdint.h>

extern "C"
{
#undef UINT64_C
#define UINT64_C Q_UINT64_C
#undef INT64_C
#define INT64_C Q_INT64_C
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

// This enables multithreaded encoding and decoding.
#define OPT_ENABLE_THREADS

namespace LXiStream {
namespace FFMpegBackend {

class FFMpegCommon
{
public:
  class SyncMemory : public SBuffer::Memory
  {
  public:
                                SyncMemory(int capacity, QSemaphore *);
    virtual                     ~SyncMemory();

  private:
    QSemaphore          * const semaphore;
  };

public:
  static void                   init(bool verbose = false);

  static void                   disableLog(bool);

  static ::AVSampleFormat       toFFMpegSampleFormat(SAudioFormat::Format);
  static SAudioFormat::Format   fromFFMpegSampleFormat(::AVSampleFormat);
  static ::PixelFormat          toFFMpegPixelFormat(SVideoFormat::Format);
  static SVideoFormat::Format   fromFFMpegPixelFormat(::PixelFormat);
  static int64_t                toFFMpegChannelLayout(SAudioFormat::Channels);
  static SAudioFormat::Channels fromFFMpegChannelLayout(int64_t, int);

  static bool                   isAudioFormat(const QByteArray &name);
  static bool                   isVideoFormat(const QByteArray &name);

  static ::AVPacket             toAVPacket(const SEncodedAudioBuffer &, ::AVStream * = NULL);
  static ::AVPacket             toAVPacket(const SEncodedVideoBuffer &, ::AVStream * = NULL);
  static ::AVPacket             toAVPacket(const SEncodedDataBuffer &, ::AVStream * = NULL);

#ifdef OPT_ENABLE_THREADS
  static int                    decodeThreadCount(::CodecID);
  static int                    encodeThreadCount(::CodecID);
  static int                    execute(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
  static int                    execute2(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
#endif

private:
  static void                   log(void * ptr, int level, const char* fmt, va_list vl);
  static int                    lock(void **mutex, AVLockOp op);

private:
  static const int              threadLimit = 16;
  static int                    logLevel;
  static bool                   logDisabled;
};

} } // End of namespaces

#endif
