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

#include "discreader.h"

namespace LXiStream {
namespace DVDNavBackend {

const char * const DiscReader::formatName = "dvd";

QString DiscReader::discPath(const QString &path)
{
  return isExtractedDiscPath(path) ? path.left(path.length() - 22) : path;
}

bool DiscReader::isExtractedDiscPath(const QString &path)
{
  return path.endsWith("/VIDEO_TS/VIDEO_TS.IFO", Qt::CaseInsensitive);
}

bool DiscReader::isDiscPath(const QString &path)
{
  const QString canonicalPath = QFileInfo(discPath(path)).canonicalFilePath();

  if (canonicalPath.endsWith(".iso", Qt::CaseInsensitive) ||
#ifdef Q_OS_UNIX
      canonicalPath.startsWith("/dev/") ||
#endif
      QDir(canonicalPath).entryList().contains("VIDEO_TS", Qt::CaseInsensitive))
  {
    return true;
  }

  return false;
}

DiscReader::DiscReader(const QString &, QObject *parent)
  : SInterfaces::DiscReader(parent),
    mutex(QMutex::Recursive),
    dvdHandle(NULL),
    currentTitle(0),
    currentChapter(-1),
    seekEnabled(false),
    playing(false),
    skipStill(false),
    skipWait(false)
{
}

DiscReader::~DiscReader()
{
  if (dvdHandle)
    ::dvdnav_close(dvdHandle);
}

bool DiscReader::openPath(const QString &, const QString &path)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (::dvdnav_open(&dvdHandle, discPath(path).toUtf8()) == DVDNAV_STATUS_OK)
    return true;

  dvdHandle = NULL;
  return false;
}

QString DiscReader::title(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  {
    const char *title = NULL;
    if (::dvdnav_get_title_string(dvdHandle, &title) == DVDNAV_STATUS_OK)
    if (title)
    if (qstrlen(title) > 0)
      return QString::fromUtf8(title);

    if (::dvdnav_path(dvdHandle, &title) == DVDNAV_STATUS_OK)
      return QFileInfo(QString::fromUtf8(title)).completeBaseName();
  }

  return QString::null;
}

unsigned DiscReader::numTitles(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  {
    int32_t titles = 0;
    if (::dvdnav_get_number_of_titles(dvdHandle, &titles) == DVDNAV_STATUS_OK)
      return titles;
  }

  return 0;
}

bool DiscReader::playTitle(unsigned title)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  if (::dvdnav_title_play(dvdHandle, title + 1) == DVDNAV_STATUS_OK)
  {
    titleChapters.clear();
    titleDuration = STime();

    currentTitle = title;
    currentChapter = -1;
    playing = true;
    skipStill = false;
    skipWait = false;

    const SInterval clock(1, 90000);
    uint64_t * chapters = NULL;
    uint64_t duration = 0;
    const uint32_t count = ::dvdnav_describe_title_chapters(dvdHandle, title + 1, &chapters, &duration);
    if (chapters)
    {
      titleChapters += STime::null;

      for (uint32_t i=0; i<count; i++)
        titleChapters += STime(chapters[i], clock);

      titleDuration = STime(duration, clock);

      ::free(chapters);
    }

    return true;
  }

  return false;
}

STime DiscReader::duration(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  return titleDuration;
}

bool DiscReader::setPosition(STime pos)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  {
    if (pos.isNull())
    {
      if (::dvdnav_title_play(dvdHandle, currentTitle + 1) == DVDNAV_STATUS_OK)
      {
        currentChapter = -1;
        playing = true;
        skipStill = false;
        skipWait = false;
  
        return true;
      }
    }
    else if (!titleChapters.isEmpty())
    {
      int bestChapter = -1;
      STime bestDelta;
      for (int i=0; i<titleChapters.count(); i++)
      {
        const STime delta = qAbs(titleChapters[i] - pos);
        if (!bestDelta.isValid() || (delta < bestDelta))
        {
          bestChapter = i;
          bestDelta = delta;
        }
      }
  
      if (bestChapter >= 0)
      if (::dvdnav_part_play(dvdHandle, currentTitle + 1, bestChapter + 1) == DVDNAV_STATUS_OK)
      {
        currentChapter = bestChapter;
        playing = true;
        skipStill = false;
        skipWait = false;
  
        return true;
      }
    }
  }

  return false;
}

STime DiscReader::position(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
    return STime(::dvdnav_get_current_time(dvdHandle), SInterval(1, 90000));

  return STime();
}

void DiscReader::annotateChapters(QList<Chapter> &chapters) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  chapters.clear();
  foreach (const STime &time, titleChapters)
  {
    SInterfaces::FormatProber::Chapter chapter;
    chapter.title = QTime(0, 0).addSecs(time.toSec()).toString("h:mm:ss");
    chapter.begin = time;
    chapters += chapter;
  }
}

void DiscReader::annotateAudioStreams(QList<AudioStreamInfo> &audioStreams) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  for (int i=0; i<audioStreams.count(); i++)
  {
    const uint16_t lang = ::dvdnav_audio_stream_to_lang(dvdHandle, i);
    if (lang != 0xFFFF)
    {
      audioStreams[i].language[0] = (lang >> 8) & 0xFF;
      audioStreams[i].language[1] = lang & 0xFF;
      audioStreams[i].language[2] = 0;
      audioStreams[i].language[3] = 0;
    }
  }
}

void DiscReader::annotateVideoStreams(QList<VideoStreamInfo> &) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

}

void DiscReader::annotateDataStreams(QList<DataStreamInfo> &dataStreams) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  {
    dataStreams.clear();

    const bool translate = ::dvdnav_get_spu_logical_stream(dvdHandle, 255) == -1;

    for (int i=0; i<0x1F; i++)
    {
      const uint16_t lang = ::dvdnav_spu_stream_to_lang(dvdHandle, i);
      if (lang != 0xFFFF)
      {
        const char language[3] = { (lang >> 8) & 0xFF, lang & 0xFF, 0 };
        const qint8 lid = ::dvdnav_get_spu_logical_stream(dvdHandle, i);
        const quint16 id = (lid >= 0 ? int(lid) : i) + 0x20;

        dataStreams +=
            DataStreamInfo(StreamId::Type_Subtitle, id, language, SDataCodec("SUB/DVD"));
      }
    }
  }
}

qint64 DiscReader::read(uchar *buffer, qint64 size)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dvdHandle)
  {
    if (playing && (size >= blockSize))
    {
      if (skipStill)
        ::dvdnav_still_skip(dvdHandle);

      if (skipWait)
        ::dvdnav_wait_skip(dvdHandle);

      qint64 bytes = 0;
      for (unsigned i=0; (bytes <= (size - blockSize)) && (i < 1024); i++)
      {
        int32_t event = 0, len = blockSize;
        if (::dvdnav_get_next_block(dvdHandle, buffer + bytes, &event, &len) == DVDNAV_STATUS_OK)
        {
          if (event == DVDNAV_BLOCK_OK)
          {
            bytes += len;
          }
          else if (event == DVDNAV_STILL_FRAME)
          {
            skipStill = true;
            break;
          }
          else if (event == DVDNAV_STOP)
          {
            playing = false;
            break;
          }
          else if (event == DVDNAV_WAIT)
          {
            skipWait = true;
            break;
          }
        }
        else
        {
          qWarning() << ::dvdnav_err_to_string(dvdHandle);

          if (seekEnabled)
          {
            seekEnabled = false;

            if (currentChapter < 0)
            {
              if (::dvdnav_title_play(dvdHandle, currentTitle + 1) != DVDNAV_STATUS_OK)
                break;
            }
            else if (::dvdnav_part_play(dvdHandle, currentTitle + 1, currentChapter + 1) != DVDNAV_STATUS_OK)
              break;
          }
          else
            break;
        }
      }

      return bytes > 0 ? bytes : Q_INT64_C(-1);
    }
    else
      qWarning() << "Read buffer is too small" << size;
  }

  return -1;
}

qint64 DiscReader::seek(qint64 offset, int whence)
{
  if (seekEnabled && dvdHandle && playing)
  {
    if (whence != -1)
    {
      if (::dvdnav_sector_search(dvdHandle, offset / blockSize, whence) == DVDNAV_STATUS_OK)
        return 0;
    }
    else
    {
      uint32_t pos = 0, len = 0;
      if (::dvdnav_get_position(dvdHandle, &pos, &len) == DVDNAV_STATUS_OK)
        return qint64(len) * blockSize;
    }
  }

  return -1;
}

} } // End of namespaces
