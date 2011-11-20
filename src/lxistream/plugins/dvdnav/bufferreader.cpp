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

#include "bufferreader.h"

namespace LXiStream {
namespace DVDNavBackend {

const char BufferReader::formatName[] = "disc.dvd";

QString BufferReader::discPath(const QString &path)
{
  return isExtractedDiscPath(path) ? path.left(path.length() - 22) : path;
}

bool BufferReader::isExtractedDiscPath(const QString &path)
{
  return path.endsWith("/video_ts/video_ts.ifo", Qt::CaseInsensitive);
}

bool BufferReader::isDiscPath(const QString &path)
{
  const QString canonicalPath = QFileInfo(discPath(path)).canonicalFilePath();

  if (canonicalPath.endsWith(".iso", Qt::CaseInsensitive) ||
#ifdef Q_OS_UNIX
      canonicalPath.startsWith("/dev/") ||
#endif
      QDir(canonicalPath).entryList().contains("video_ts", Qt::CaseInsensitive))
  {
    return true;
  }

  return false;
}

BufferReader::BufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    SInterfaces::BufferReader::ReadCallback(),
    mutex(QMutex::Recursive),
    dvdHandle(NULL),
    currentTitle(0),
    currentChapter(-1),
    produceCallback(NULL),
    bufferReader(NULL),
    seekEnabled(false),
    flushing(false),
    playing(false),
    skipStill(false),
    skipWait(false)
{
}

BufferReader::~BufferReader()
{
  delete bufferReader;

  if (dvdHandle)
    ::dvdnav_close(dvdHandle);
}

bool BufferReader::openFormat(const QString &)
{
  return true;
}

bool BufferReader::openFile(const QString &filePath)
{
  if (::dvdnav_open(&dvdHandle, discPath(filePath).toUtf8()) == DVDNAV_STATUS_OK)
  {
    return true;
  }

  dvdHandle = NULL;
  return false;
}

QString BufferReader::discTitle(void) const
{
  QMutexLocker l(&mutex);

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

unsigned BufferReader::numTitles(void) const
{
  QMutexLocker l(&mutex);

  if (dvdHandle)
  {
    int32_t titles = 0;
    if (::dvdnav_get_number_of_titles(dvdHandle, &titles) == DVDNAV_STATUS_OK)
      return titles;
  }

  return 0;
}

bool BufferReader::selectTitle(quint16 programId)
{
  if (::dvdnav_title_play(dvdHandle, programId + 1) == DVDNAV_STATUS_OK)
  {
    titleChapters.clear();
    titleDuration = STime();

    currentTitle = programId;
    currentChapter = -1;
    playing = true;
    skipStill = false;
    skipWait = false;

    const SInterval clock(1, 90000);
    uint64_t * chapters = NULL;
    uint64_t duration = 0;
    const uint32_t count = ::dvdnav_describe_title_chapters(dvdHandle, programId + 1, &chapters, &duration);
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

bool BufferReader::reopenBufferReader(void)
{
  delete bufferReader;

  bufferReader = SInterfaces::BufferReader::create(this, "mpeg", false);
  if (bufferReader)
  {
    if (bufferReader->start(this, produceCallback, 0, true))
    {
      if (!selectedStreams.isEmpty())
        bufferReader->selectStreams(selectedStreams);

      return true;
    }

    delete bufferReader;
    bufferReader = NULL;
  }

  return false;
}

bool BufferReader::start(SInterfaces::BufferReader::ReadCallback *rc, SInterfaces::BufferReader::ProduceCallback *pc, quint16 programId, bool)
{
  QMutexLocker l(&mutex);

  if (dvdHandle)
    qFatal("BufferReader already opened a stream.");

//  if (openFile(rc->path))
//  {
//    produceCallback = pc;
//
//    if (selectTitle(programId))
//    if (reopenBufferReader())
//      return true;
//
//    ::dvdnav_close(dvdHandle);
//  }

  dvdHandle = NULL;
  return false;
}

void BufferReader::stop(void)
{
  if (dvdHandle)
  {
    ::dvdnav_close(dvdHandle);
    dvdHandle = NULL;

    currentTitle = 0;
    currentChapter = 0;
    titleChapters.clear();
    titleDuration = STime();

    seekEnabled = flushing = playing = skipStill = skipWait = false;
  }

  produceCallback = NULL;
  delete bufferReader;
  bufferReader = NULL;
}

bool BufferReader::process(void)
{
  return bufferReader->process();
}

STime BufferReader::duration(void) const
{
  QMutexLocker l(&mutex);

  return titleDuration;
}

bool BufferReader::setPosition(STime pos)
{
  QMutexLocker l(&mutex);

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

        return reopenBufferReader();
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

        flushing = true;
        bufferReader->setPosition(STime::fromMin(1));
        bufferReader->setPosition(STime::null);
        flushing = false;

        return reopenBufferReader();
      }
    }
  }

  return false;
}

STime BufferReader::position(void) const
{
  QMutexLocker l(&mutex);

  if (dvdHandle)
    return STime(::dvdnav_get_current_time(dvdHandle), SInterval(1, 90000));

  return STime();
}

QList<BufferReader::Chapter> BufferReader::chapters(void) const
{
  QMutexLocker l(&mutex);

  QList<Chapter> chapters;
  foreach (const STime &time, titleChapters)
  {
    SInterfaces::FormatProber::Chapter chapter;
    chapter.title = QTime(0, 0).addSecs(time.toSec()).toString("h:mm:ss");
    chapter.begin = time;
    chapters += chapter;
  }

  return chapters;
}

QList<BufferReader::AudioStreamInfo> BufferReader::audioStreams(void) const
{
  QMutexLocker l(&mutex);

  return filterAudioStreams(bufferReader->audioStreams());
}

QList<BufferReader::AudioStreamInfo> BufferReader::filterAudioStreams(const QList<AudioStreamInfo> &streams) const
{
  QMutexLocker l(&mutex);

  QList<AudioStreamInfo> result = streams;
  qSort(result);

  if (dvdHandle)
  for (int i=0; i<result.count(); i++)
  {
    const uint16_t lang = ::dvdnav_audio_stream_to_lang(dvdHandle, i);
    if (lang != 0xFFFF)
    {
      const char language[] = { char(lang >> 8), char(lang & 0xFF), '\0' };
      result[i].language = language;
    }
  }

  return result;
}

QList<BufferReader::VideoStreamInfo> BufferReader::videoStreams(void) const
{
  QMutexLocker l(&mutex);

  return filterVideoStreams(bufferReader->videoStreams());
}

QList<BufferReader::VideoStreamInfo> BufferReader::filterVideoStreams(const QList<VideoStreamInfo> &streams) const
{
  QList<VideoStreamInfo> result = streams;
  qSort(result);

  return result;
}

QList<BufferReader::DataStreamInfo> BufferReader::dataStreams(void) const
{
  QMutexLocker l(&mutex);

  return filterDataStreams(bufferReader->dataStreams());
}

QList<BufferReader::DataStreamInfo> BufferReader::filterDataStreams(const QList<DataStreamInfo> &streams) const
{
  QMutexLocker l(&mutex);

  QList<DataStreamInfo> result = streams;
  qSort(result);

  if (dvdHandle)
  {
    // Match the subtitle streams.
    bool found = false;
    for (int i=0; i<0x1F; i++)
    {
      const uint16_t lang = ::dvdnav_spu_stream_to_lang(dvdHandle, i);
      if (lang != 0xFFFF)
      {
        const qint8 lid = ::dvdnav_get_spu_logical_stream(dvdHandle, i);
        const quint16 id = (lid >= 0 ? int(lid) : i) + 0x20;

        for (int i=0; i<result.count(); i++)
        if (((result[i].type & StreamId::Type_Flag_Native) != 0) && result[i].id == id)
        {
          result[i].language[0] = (lang >> 8) & 0xFF;
          result[i].language[1] = lang & 0xFF;
          result[i].language[2] = 0;
          result[i].language[3] = 0;

          found = true;
        }
      }
    }

    // Remove any unidentified subtitle streams.
    if (found)
    for (QList<DataStreamInfo>::Iterator i=result.begin(); i!=result.end(); )
    if (i->language.isEmpty())
      i = result.erase(i);
    else
      i++;
  }

  return result;
}

void BufferReader::selectStreams(const QVector<StreamId> &streams)
{
  selectedStreams = streams;
  if (bufferReader)
    bufferReader->selectStreams(selectedStreams);
}

qint64 BufferReader::read(uchar *buffer, qint64 size)
{
  QMutexLocker l(&mutex);

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

qint64 BufferReader::seek(qint64 offset, int whence)
{
  if (flushing)
    return 0;

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
