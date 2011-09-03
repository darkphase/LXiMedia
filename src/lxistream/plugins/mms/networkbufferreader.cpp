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

#include "networkbufferreader.h"
#include <LXiServer>

namespace LXiStream {
namespace MMSBackend {

NetworkBufferReader::NetworkBufferReader(const QString &, QObject *parent)
  : SInterfaces::NetworkBufferReader(parent),
    SInterfaces::BufferReader::ReadCallback(QString::null),
    mmsHandle(NULL),
    bufferReader(NULL)
{
}

NetworkBufferReader::~NetworkBufferReader()
{
  if (mmsHandle)
  {
    ::mmsx_close(mmsHandle);
    mmsHandle = NULL;
  }

  delete bufferReader;
}

bool NetworkBufferReader::openProtocol(const QString &)
{
  return true;
}

bool NetworkBufferReader::start(const QUrl &url, ProduceCallback *pc, quint16 programId)
{
  foreach (const QUrl &mmsUrl, resolve(url))
  {
    qDebug() << "MMS connect" << mmsUrl.toString();
    mmsHandle = ::mmsx_connect(NULL, NULL, mmsUrl.toEncoded(), 1048576);
    if (mmsHandle)
    {
      qDebug() << "MMS connected";

      bufferReader = SInterfaces::BufferReader::create(this, "asf", true);
      if (bufferReader)
      {
        qDebug() << "MMS bufferReader->start";
        if (bufferReader->start(this, pc, programId, false))
        {
          qDebug() << "MMS bufferReader->start finished";
          return true;
        }

        qDebug() << "MMS bufferReader->start failed";

        delete bufferReader;
        bufferReader = NULL;
      }

      ::mmsx_close(mmsHandle);
      mmsHandle = NULL;
    }
  }

  return false;
}

void NetworkBufferReader::stop(void)
{
  if (mmsHandle)
  {
    ::mmsx_close(mmsHandle);
    mmsHandle = NULL;
  }

  delete bufferReader;
  bufferReader = NULL;
}

bool NetworkBufferReader::process(void)
{
  return bufferReader->process();
}

bool NetworkBufferReader::buffer(void)
{
  return bufferReader->buffer();
}

STime NetworkBufferReader::bufferDuration(void) const
{
  return bufferReader->bufferDuration();
}

STime NetworkBufferReader::duration(void) const
{
  if (mmsHandle)
    return STime::fromMSec(::mmsx_get_time_length(mmsHandle) * 1000.0);

  return STime();
}

bool NetworkBufferReader::setPosition(STime pos)
{
  if (mmsHandle)
    return ::mmsx_time_seek(NULL, mmsHandle, double(pos.toMSec()) / 1000.0) == 0;

  return false;
}

STime NetworkBufferReader::position(void) const
{
  return STime();
}

QList<NetworkBufferReader::Chapter> NetworkBufferReader::chapters(void) const
{
  return bufferReader->chapters();
}

QList<NetworkBufferReader::AudioStreamInfo> NetworkBufferReader::audioStreams(void) const
{
  return bufferReader->audioStreams();
}

QList<NetworkBufferReader::VideoStreamInfo> NetworkBufferReader::videoStreams(void) const
{
  return bufferReader->videoStreams();
}

QList<NetworkBufferReader::DataStreamInfo> NetworkBufferReader::dataStreams(void) const
{
  return bufferReader->dataStreams();
}

void NetworkBufferReader::selectStreams(const QList<StreamId> &streams)
{
  bufferReader->selectStreams(streams);
}

qint64 NetworkBufferReader::read(uchar *buffer, qint64 size)
{
  if (mmsHandle)
  {
    int result = ::mmsx_read(NULL, mmsHandle, reinterpret_cast<char *>(buffer), size);
    
    qDebug() << "MMS NetworkBufferReader::read" << size << result;
    
    return result;
  }

  return -1;
}

qint64 NetworkBufferReader::seek(qint64 offset, int whence)
{
  if (whence != -1)
  {
    qint64 result = ::mmsx_seek(NULL, mmsHandle, offset, whence);

    qDebug() << "MMS NetworkBufferReader::seek" << offset << whence << result;

    return result;
  }
  else
  {
    qint64 result = ::mmsx_get_current_pos(mmsHandle);

    qDebug() << "MMS NetworkBufferReader::seek" << offset << whence << result;

    return result;
  }
}

QList<QUrl> NetworkBufferReader::resolve(const QUrl &url)
{
  if (url.scheme().startsWith("http"))
  {
    SHttpEngine::RequestMessage request(NULL);
    request.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
    request.setHost(url.host(), url.port(80));

    const SHttpEngine::ResponseMessage response = SHttpClient::blockedRequest(request, 5000);
    if (response.status() == SHttpEngine::Status_Ok)
    {
      // Correct corrupted Microsoft XMLs
      QByteArray corrected = response.content();
      corrected.replace("&amp;", "&").replace("&", "&amp;");

      QDomDocument doc;
      QString error; int line = 0, col = 0;
      if (doc.setContent(corrected, &error, &line, &col))
      {
        const QDomElement root = doc.documentElement();
        const QDomElement entry = root.firstChildElement("Entry");

        QList<QUrl> result;
        for (QDomElement ref = entry.firstChildElement("Ref");
             !ref.isNull();
             ref = ref.nextSiblingElement("Ref"))
        {
          const QString href = ref.attribute("href");
          if (!href.isEmpty())
            result.append(resolve(href));
        }

        if (!result.isEmpty())
        {
          for (int i=0; i<result.count(); i++)
            qSwap(result[qrand() % result.count()], result[qrand() % result.count()]);

          return result;
        }
      }
      else
        qDebug() << line << col << error << corrected;
    }
  }

  return QList<QUrl>() << url;
}

} } // End of namespaces
