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

bool BufferReader::isDiscPath(const QUrl &path)
{
  // libdvdnav only supports local files.
  if (path.scheme() == "file")
  {
    if (path.path().endsWith(".iso", Qt::CaseInsensitive))
      return true;

//  libdvdread crashes when reading separate files, so only ISO for now.
//    QDir dir(path.path());
//    foreach (const QString &name, dir.entryList(QStringList("video_ts"), QDir::Dirs))
//    if (dir.cd(name))
//    {
//      if (!dir.entryList(QStringList("video_ts.ifo"), QDir::Files).isEmpty())
//        return true;
//
//      dir.cdUp();
//    }
  }

  return false;
}

BufferReader::BufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    dvdDevice(this),
    currentTitle(-1),
    currentChapter(-1),
    produceCallback(NULL),
    bufferReader(NULL)
{
}

BufferReader::~BufferReader()
{
  delete bufferReader;

  if (dvdDevice.isOpen())
    dvdDevice.close();
}

bool BufferReader::openFormat(const QString &)
{
  return true;
}

bool BufferReader::selectTitle(int titleId)
{
  if ((titleId >= 0) && (titleId < dvdDevice.numTitles()))
  {
    titleChapters.clear();
    titleDuration = STime();

    currentTitle = titleId;
    currentChapter = -1;

    if (dvdDevice.reset())
    {
      titleChapters = dvdDevice.chapters();
      if (!titleChapters.isEmpty())
        titleDuration = titleChapters.takeLast();
    }

    delete bufferReader;
    bufferReader = NULL;

    return true;
  }

  return false;
}

bool BufferReader::openBufferReader(void)
{
  delete bufferReader;

  bufferReader = SInterfaces::BufferReader::create(this, "mpeg", false);
  if (bufferReader)
  {
    if (bufferReader->start(&dvdDevice, produceCallback, true))
    {
      if (!selectedStreams.isEmpty())
        bufferReader->selectStreams(0, selectedStreams);

      return true;
    }

    delete bufferReader;
    bufferReader = NULL;
  }

  return false;
}

bool BufferReader::start(QIODevice *device, SInterfaces::BufferReader::ProduceCallback *pc, bool)
{
  if (dvdDevice.isOpen())
    qFatal("BufferReader already opened a stream.");

  path = "";

  QFile * const file = qobject_cast<QFile *>(device);
  if (file)
    path = file->fileName();

  if (!path.isEmpty() && dvdDevice.open(QIODevice::ReadOnly))
  {
    currentTitle = -1;
    currentChapter = -1;
    titleChapters.clear();
    titleDuration = STime();

    produceCallback = pc;
    delete bufferReader;
    bufferReader = NULL;

    return true;
  }

  return false;
}

void BufferReader::stop(void)
{
  dvdDevice.close();

  currentTitle = -1;
  currentChapter = -1;
  titleChapters.clear();
  titleDuration = STime();

  produceCallback = NULL;
  delete bufferReader;
  bufferReader = NULL;
}

bool BufferReader::process(void)
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle < 0)
      selectTitle(0);

    if (bufferReader == NULL)
      openBufferReader();

    if (bufferReader)
      return bufferReader->process();
  }

  return false;
}

STime BufferReader::duration(void) const
{
  return titleDuration;
}

bool BufferReader::setPosition(STime pos)
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle < 0)
      selectTitle(0);

    int bestChapter = -1;
    STime bestDelta;
    for (int i=0, n=titleChapters.count(); i<n; i++)
    {
      const STime delta = pos - titleChapters[i];
      if (delta.isPositive() && (!bestDelta.isValid() || (delta < bestDelta)))
      {
        bestChapter = i;
        bestDelta = delta;
      }
    }

    if (bestChapter >= 0)
      currentChapter = bestChapter;
    else
      currentChapter = -1;

    if (dvdDevice.reset())
    {
      openBufferReader();

      if (bufferReader)
        return bufferReader->setPosition(pos - (currentChapter >= 0 ? titleChapters[currentChapter] : STime::null));
    }
  }

  return false;
}

STime BufferReader::position(void) const
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle < 0)
      return STime::null;

    if (bufferReader)
      return bufferReader->position() + (currentChapter >= 0 ? titleChapters[currentChapter] : STime::null);
    else
      return STime::null;
  }

  return STime();
}

QList<BufferReader::Chapter> BufferReader::chapters(void) const
{
  QList<Chapter> result;

  if (dvdDevice.isOpen())
  {
    if (currentTitle < 0)
      const_cast<BufferReader *>(this)->selectTitle(0);

    foreach (const STime &time, titleChapters)
    {
      SInterfaces::FormatProber::Chapter chapter;
      chapter.title = QTime(0, 0).addSecs(time.toSec()).toString("h:mm:ss");
      chapter.begin = time;
      result += chapter;
    }
  }

  return result;
}

int BufferReader::numTitles(void) const
{
  return dvdDevice.numTitles();
}

QList<BufferReader::AudioStreamInfo> BufferReader::audioStreams(int title) const
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle != title)
      const_cast<BufferReader *>(this)->selectTitle(title);

    if (bufferReader == NULL)
      const_cast<BufferReader *>(this)->openBufferReader();

    if (bufferReader)
      return filterAudioStreams(bufferReader->audioStreams(0));
  }

  return QList<AudioStreamInfo>();
}

QList<BufferReader::AudioStreamInfo> BufferReader::filterAudioStreams(const QList<AudioStreamInfo> &streams) const
{
  QList<AudioStreamInfo> result = streams;
  qSort(result);

  for (int i=0; i<result.count(); i++)
  {
    const quint16 lang = dvdDevice.audioLanguage(i);
    if (lang != 0xFFFF)
    {
      const char language[] = { char(lang >> 8), char(lang & 0xFF), '\0' };
      result[i].language = language;
    }
  }

  return result;
}

QList<BufferReader::VideoStreamInfo> BufferReader::videoStreams(int title) const
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle != title)
      const_cast<BufferReader *>(this)->selectTitle(title);

    if (bufferReader == NULL)
      const_cast<BufferReader *>(this)->openBufferReader();

    if (bufferReader)
      return filterVideoStreams(bufferReader->videoStreams(0));
  }

  return QList<VideoStreamInfo>();
}

QList<BufferReader::VideoStreamInfo> BufferReader::filterVideoStreams(const QList<VideoStreamInfo> &streams) const
{
  QList<VideoStreamInfo> result = streams;
  qSort(result);

  return result;
}

QList<BufferReader::DataStreamInfo> BufferReader::dataStreams(int title) const
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle != title)
      const_cast<BufferReader *>(this)->selectTitle(title);

    if (bufferReader == NULL)
      const_cast<BufferReader *>(this)->openBufferReader();

    if (bufferReader)
      return filterDataStreams(bufferReader->dataStreams(0));
  }

  return QList<DataStreamInfo>();
}

QList<BufferReader::DataStreamInfo> BufferReader::filterDataStreams(const QList<DataStreamInfo> &streams) const
{
  QList<DataStreamInfo> result = streams;
  qSort(result);

  // Match the subtitle streams.
  bool found = false;
  for (int i=0; i<0x1F; i++)
  {
    const quint16 lang = dvdDevice.subtitleLanguage(i);
    if (lang != 0xFFFF)
    {
      const qint8 lid = dvdDevice.subtitleLogicalStream(i);
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

  return result;
}

void BufferReader::selectStreams(int title, const QVector<StreamId> &streams)
{
  if (dvdDevice.isOpen())
  {
    if (currentTitle != title)
      selectTitle(title);

    selectedStreams = streams;

    if (bufferReader)
      bufferReader->selectStreams(0, selectedStreams);
    else
      openBufferReader(); // Will select the streams.
  }
}


BufferReader::DvdDevice::DvdDevice(BufferReader *parent)
  : parent(parent),
    dvdHandle(NULL)
{
}

QString BufferReader::DvdDevice::discName(void) const
{
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

int BufferReader::DvdDevice::numTitles(void) const
{
  if (dvdHandle)
  {
    int32_t titles = 0;
    if (::dvdnav_get_number_of_titles(dvdHandle, &titles) == DVDNAV_STATUS_OK)
      return titles;
  }

  return 0;
}

QList<STime> BufferReader::DvdDevice::chapters(void) const
{
  QList<STime> result;

  if (dvdHandle)
  {
    const SInterval clock(1, 90000);

    uint64_t * chapters = NULL;
    uint64_t duration = 0;
    const uint32_t count = ::dvdnav_describe_title_chapters(dvdHandle, parent->currentTitle + 1, &chapters, &duration);
    if (chapters)
    {
      result += STime::null;

      for (uint32_t i=0; i<count; i++)
        result += STime(chapters[i], clock);

      result += STime(duration, clock);

      ::free(chapters);
    }
  }

  return result;
}

quint16 BufferReader::DvdDevice::audioLanguage(int stream) const
{
  if (dvdHandle)
    return ::dvdnav_audio_stream_to_lang(dvdHandle, stream);

  return 0xFFFF;
}

quint16 BufferReader::DvdDevice::subtitleLanguage(int stream) const
{
  if (dvdHandle)
    return ::dvdnav_spu_stream_to_lang(dvdHandle, stream);

  return 0xFFFF;
}

qint8 BufferReader::DvdDevice::subtitleLogicalStream(int stream) const
{
  if (dvdHandle)
    return ::dvdnav_get_spu_logical_stream(dvdHandle, stream);

  return 0;
}

bool BufferReader::DvdDevice::open(OpenMode mode)
{
  if (!isOpen() && (mode == QIODevice::ReadOnly))
  {
    if (::dvdnav_open(&dvdHandle, parent->path.toUtf8()) == DVDNAV_STATUS_OK)
    {
      ::dvdnav_set_readahead_flag(dvdHandle, 0);

      return QIODevice::open(mode);
    }
    else
      qWarning() << "DVDNav:" << ::dvdnav_err_to_string(dvdHandle);

    dvdHandle = NULL;
  }

  return false;
}

void BufferReader::DvdDevice::close(void)
{
  if (dvdHandle)
    ::dvdnav_close(dvdHandle);

  dvdHandle = NULL;
  QIODevice::close();
}

bool BufferReader::DvdDevice::reset(void)
{
  if (resetStream())
    return QIODevice::reset();

  return false;
}

bool BufferReader::DvdDevice::seek(qint64 newPos)
{
  qint64 curPos = QIODevice::pos();

  if (((curPos + QIODevice::bytesAvailable()) >= newPos) && (curPos <= newPos))
  {
    return QIODevice::seek(newPos);
  }
  else if (dvdHandle)
  {
    if ((newPos - curPos) < 0)
    if (resetStream())
      curPos = 0;

    if (QIODevice::seek(curPos))
    {
      qint64 delta = newPos - curPos;
      while (delta > 0)
      {
        char buffer[blockSize];

        const qint64 r = QIODevice::read(buffer, qMin(delta, qint64(sizeof(buffer))));
        if (r > 0)
          delta -= r;
        else
          break;
      }

      return delta == 0;
    }
  }

  return false;
}

qint64 BufferReader::DvdDevice::size(void) const
{
  if (dvdHandle)
  {
    uint32_t pos = 0, len = 0;
    if (::dvdnav_get_position(dvdHandle, &pos, &len) == DVDNAV_STATUS_OK)
      return qint64(len) * blockSize;
    else
      qWarning() << "DVDNav:" << ::dvdnav_err_to_string(dvdHandle);
  }

  return -1;
}

qint64 BufferReader::DvdDevice::readData(char *data, qint64 maxSize)
{
  Q_ASSERT(maxSize >= blockSize);

  if (dvdHandle && (maxSize >= blockSize))
  {
    qint64 bytes = 0;
    for (unsigned i=0; (bytes <= (maxSize - blockSize)) && (i < 1024); i++)
    {
      int32_t event = 0, len = maxSize - bytes;
      if (::dvdnav_get_next_block(dvdHandle, reinterpret_cast<uint8_t *>(data) + bytes, &event, &len) == DVDNAV_STATUS_OK)
      {
        if (event == DVDNAV_BLOCK_OK)
          bytes += len;
        else if (event == DVDNAV_STILL_FRAME)
          ::dvdnav_still_skip(dvdHandle);
        else if (event == DVDNAV_WAIT)
          ::dvdnav_wait_skip(dvdHandle);
        else if (event == DVDNAV_STOP)
          return -1;
      }
      else
      {
        qWarning() << "DVDNav:" << ::dvdnav_err_to_string(dvdHandle);
        return -1;
      }
    }

    return bytes;
  }

  return -1;
}

qint64 BufferReader::DvdDevice::writeData(const char *, qint64)
{
  return -1;
}

bool BufferReader::DvdDevice::resetStream(void)
{
  if (parent->currentChapter >= 0)
  {
    if (::dvdnav_part_play(dvdHandle, parent->currentTitle + 1, parent->currentChapter + 1) == DVDNAV_STATUS_OK)
      return true;
  }
  else if (::dvdnav_title_play(dvdHandle, parent->currentTitle + 1) == DVDNAV_STATUS_OK)
    return true;

  return false;
}

} } // End of namespaces
