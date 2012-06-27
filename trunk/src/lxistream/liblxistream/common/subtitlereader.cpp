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

#include "subtitlereader.h"

namespace LXiStream {
namespace Common {

SubtitleReader::SubtitleReader(const QString &, QObject *parent)
  : BufferReader(parent),
    subtitleBuffer(&subtitleData),
    ioDevice(NULL),
    produceCallback(NULL),
    open(false),
    utf8(false),
    language(NULL),
    selected(false),
    pos(STime::null)
{
}

SubtitleReader::~SubtitleReader()
{
}

bool SubtitleReader::openFormat(const QString &)
{
  return true;
}

bool SubtitleReader::start(QIODevice *baseIoDevice, ProduceCallback *produceCallback, bool streamed)
{
  if (baseIoDevice)
  {
    if (streamed)
    {
      subtitleData = baseIoDevice->read(16777216);
      if (!subtitleData.isEmpty() && subtitleBuffer.open(QBuffer::ReadOnly))
        ioDevice = &subtitleBuffer;
    }
    else
      ioDevice = baseIoDevice;

    const QByteArray sample = ioDevice->read(262144);
    ioDevice->seek(0);

    if (!sample.isEmpty())
    {
      this->produceCallback = produceCallback;
      open = true;
      utf8 = SStringParser::isUtf8(sample);
      dataCodec = SDataCodec(utf8 ? "sub_rawutf8" : "sub_raw8bit");
      language = SStringParser::languageOf(sample);
      selected = true;
      pos = STime::null;

      if (!utf8)
      {
        const char * const codepage = SStringParser::codepageFor(language);
        if (codepage)
          dataCodec.setCodepage(codepage);
      }

      return true;
    }
  }

  return false;
}

void SubtitleReader::stop(void)
{
  if (subtitleBuffer.isOpen())
    subtitleBuffer.close();

  subtitleData.clear();

  ioDevice = NULL;
  produceCallback = NULL;
  open = false;
  language = NULL;
}

STime SubtitleReader::duration(void) const
{
  return STime::null;
}

bool SubtitleReader::setPosition(STime pos)
{
  if (pos < this->pos)
    ioDevice->seek(0);

  this->pos = pos;

  return true;
}

STime SubtitleReader::position(void) const
{
  return pos;
}

QList<SubtitleReader::Chapter> SubtitleReader::chapters(void) const
{
  return QList<Chapter>();
}

int SubtitleReader::numTitles(void) const
{
  return open ? 1 : 0;
}

QList<SubtitleReader::AudioStreamInfo> SubtitleReader::audioStreams(int) const
{
  return QList<AudioStreamInfo>();
}

QList<SubtitleReader::VideoStreamInfo> SubtitleReader::videoStreams(int) const
{
  return QList<VideoStreamInfo>();
}

QList<SubtitleReader::DataStreamInfo> SubtitleReader::dataStreams(int) const
{
  QList<DataStreamInfo> result;

  if (open)
  {
    result += DataStreamInfo(
        StreamId(StreamId::Type_Subtitle, 0x0001),
        language,
        QString::null,
        dataCodec);
  }

  return result;
}

void SubtitleReader::selectStreams(int, const QVector<StreamId> &streams)
{
  selected = false;
  foreach (StreamId stream, streams)
    selected |= stream.id == 0x0001;
}

bool SubtitleReader::process(void)
{
  const SEncodedDataBuffer buffer = readNextSrtSubtitle();
  if (produceCallback)
    produceCallback->produce(buffer);

  return !buffer.isNull();
}

SEncodedDataBuffer SubtitleReader::readNextSrtSubtitle(void)
{
  int phase = 0;
  STime startTime = STime::null, stopTime = STime::null;
  QByteArray data;
  QByteArray line;

  if (open && selected)
  while ((line = ioDevice->readLine()).length() > 0)
  {
    if (phase == 0)
    { // Get ID
      bool ok = false;
      line.trimmed().toInt(&ok);

      if (ok)
        phase++;
    }
    else if (phase == 1)
    { // Get times
      const QStringList times = QString(line.trimmed()).split("-->", QString::SkipEmptyParts);
      if (times.count() >= 2)
      {
        QTime start = QTime::fromString(times[0].trimmed());
        QTime stop = QTime::fromString(times[1].trimmed());

        if (start.isValid() && stop.isValid())
        {
          startTime = STime::fromMSec(QTime(0, 0).msecsTo(start));
          stopTime = STime::fromMSec(QTime(0, 0).msecsTo(stop));
          phase++;
        }
      }
    }
    else if (phase == 2)
    { // Read subtitles
      const QByteArray t = line.trimmed();

      if (t.length() > 0)
      {
        if (data.isEmpty())
          data = t;
        else
          data += '\n' + t;
      }
      else if (stopTime > pos)
      { // We're finished
        if (!data.isEmpty())
        {
          SEncodedDataBuffer buffer(dataCodec, data);
          buffer.setPresentationTimeStamp(startTime);
          buffer.setDuration(stopTime - startTime);

          pos = stopTime;

          return buffer;
        }
        else
          return SEncodedDataBuffer();
      }
      else // Parse the next subtitle
      {
        data.clear();
        phase = 0;
      }
    }
  }

  return SEncodedDataBuffer();
}

} } // End of namespaces
