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

#include "backend.h"

void backend::add_video_protocols()
{
  // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

  /////////////////////////////////////////////////////////////////////////////
  // MPEG1
  connection_manager.add_source_video_protocol(
        "MPEG1",
        "video/mpeg", "mpg",
        44100, 2, 352, 288, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp1v", "mpeg1",
        "vb=4096,venc=ffmpeg{keyint=0,vt=2048}",
        "vb=2048,venc=ffmpeg{bframes=0,vt=1024}");

  connection_manager.add_source_video_protocol(
        "MPEG1",
        "video/mpeg", "mpg",
        44100, 2, 320, 240, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp1v", "mpeg1",
        "vb=4096,venc=ffmpeg{keyint=0,vt=2048}",
        "vb=2048,venc=ffmpeg{bframes=0,vt=1024}");


  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 PAL/NTSC
  connection_manager.add_source_video_protocol(
        "MPEG_PS_PAL",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_PAL_XAC3",
        "video/mpeg", "mpg",
        48000, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_NTSC",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_NTSC_XAC3",
        "video/mpeg", "mpg",
        48000, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 SD
  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 720, 576, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 720, 576, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 704, 480, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 704, 480, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 SD - nonstandard program stream
  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 720, 576, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 704, 480, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 720p
  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1280, 720, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1280, 720, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 720p - nonstandard program stream
  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1280, 720, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1280, 720, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 1080p
  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1920, 1080, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        48000, 6, 1920, 1080, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ts",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 1080p - nonstandard program stream
  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1920, 1080, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

  connection_manager.add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        48000, 6, 1920, 1080, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v", "ps",
        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

}
