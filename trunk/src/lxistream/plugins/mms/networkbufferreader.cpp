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
    ::mms_close(mmsHandle);
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
    mmsHandle = ::mms_connect(NULL, NULL, url.toEncoded(), 1048576);
    if (mmsHandle)
    {
      bufferReader = SInterfaces::BufferReader::create(this, "asf", false);
      if (bufferReader)
      {
        if (bufferReader->start(this, pc, programId, true))
          return true;

        delete bufferReader;
        bufferReader = NULL;
      }

      ::mms_close(mmsHandle);
      mmsHandle = NULL;
    }
  }

  return false;
}

void NetworkBufferReader::stop(void)
{
  if (mmsHandle)
  {
    ::mms_close(mmsHandle);
    mmsHandle = NULL;
  }

  delete bufferReader;
  bufferReader = NULL;
}

bool NetworkBufferReader::buffer(void)
{
  return true;
}

STime NetworkBufferReader::bufferDuration(void) const
{
  return STime::null;
}

bool NetworkBufferReader::process(void)
{
  return bufferReader->process();
}

STime NetworkBufferReader::duration(void) const
{
  if (mmsHandle)
    return STime::fromMSec(::mms_get_time_length(mmsHandle) * 1000.0);

  return STime();
}

bool NetworkBufferReader::setPosition(STime pos)
{
  if (mmsHandle)
    return ::mms_time_seek(NULL, mmsHandle, double(pos.toMSec()) / 1000.0) == 0;

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
    return ::mms_read(NULL, mmsHandle, reinterpret_cast<char *>(buffer), size);

  return -1;
}

qint64 NetworkBufferReader::seek(qint64 offset, int whence)
{
  return -1;
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
          return result;
      }
      else
        qDebug() << line << col << error << corrected;
    }
  }

  return QList<QUrl>() << url;
}

} } // End of namespaces
