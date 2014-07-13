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

#include "connection_manager.h"

void pupnp::connection_manager::add_video_protocols()
{
  /////////////////////////////////////////////////////////////////////////////
  // MPEG1
  add_source_video_protocol(
        "MPEG1",
        "video/mpeg", "mpg",
        44100, 2, 352, 288, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp1v,vb=1536",
        "mpeg1");

  add_source_video_protocol(
        "MPEG1",
        "video/mpeg", "mpg",
        44100, 2, 320, 240, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp1v,vb=1536",
        "mpeg1");


  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 PAL/NTSC
  add_source_video_protocol(
        "MPEG_PS_PAL",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_PAL_XAC3",
        "video/mpeg", "mpg",
        44100, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192,fps=25",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_NTSC",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_NTSC_XAC3",
        "video/mpeg", "mpg",
        44100, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192,fps=29.97",
        "ps");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 SD
  add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 720, 576, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 720, 576, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 704, 480, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 704, 480, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_SD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ts");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 SD - nonstandard program stream
  add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 720, 576, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 720, 576, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 720, 576, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 704, 480, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 704, 480, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_SD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 704, 480, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=8192",
        "ps");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 720p
  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1280, 720, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1280, 720, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1280, 720, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ts");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 720p - nonstandard program stream
  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1280, 720, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1280, 720, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1280, 720, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=16384",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1280, 720, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=16384",
        "ps");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 1080p
  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_EU_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1920, 1080, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 2, 1920, 1080, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ts");

  add_source_video_protocol(
        "MPEG_TS_HD_NA_ISO",
        "video/x-mpegts", "ts",
        44100, 6, 1920, 1080, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ts");

  /////////////////////////////////////////////////////////////////////////////
  // MPEG2 1080p - nonstandard program stream
  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 24.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 25.0f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1920, 1080, 24.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_EU_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1920, 1080, 25.0f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 2, 1920, 1080, 29.97f,
        "acodec=mpga,ab=256", "vcodec=mp2v,vb=24576",
        "ps");

  add_source_video_protocol(
        "MPEG_PS_HD_NA_NONSTD",
        "video/mpeg", "mpg",
        44100, 6, 1920, 1080, 29.97f,
        "acodec=a52,ab=640", "vcodec=mp2v,vb=24576",
        "ps");

}
