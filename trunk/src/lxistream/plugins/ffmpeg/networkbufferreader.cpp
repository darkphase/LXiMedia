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

#include "networkbufferreader.h"
#include <QtXml>

namespace LXiStream {
namespace FFMpegBackend {

NetworkBufferReader::NetworkBufferReader(const QString &, QObject *parent)
  : SInterfaces::NetworkBufferReader(parent)
{
}

NetworkBufferReader::~NetworkBufferReader()
{
}

bool NetworkBufferReader::openProtocol(const QString &)
{
  return true;
}

bool NetworkBufferReader::start(const QUrl &url, ProduceCallback *produceCallback)
{
  if (url.scheme() == "mms")
  {
    // Attempt different MMS protocols
    QUrl rurl = url;

    rurl.setScheme("mmst");
    if (NetworkBufferReader::start(rurl, produceCallback))
      return true;

    rurl.setScheme("mmsh");
    if (NetworkBufferReader::start(rurl, produceCallback))
      return true;

    rurl.setScheme("rtsp");
    if (NetworkBufferReader::start(rurl, produceCallback))
      return true;

    return false;
  }
  else if (url.scheme() == "http")
  {
    // Try to resolve ASF redirects.
    const QList<QUrl> resolved = resolveAsf(url);
    if (!resolved.isEmpty())
    {
      foreach (const QUrl &rurl, resolved)
      if (NetworkBufferReader::start(rurl, produceCallback))
        return true;

      return false;
    }
  }

  qDebug() << url.toEncoded();

  ::AVFormatContext * formatContext = NULL;
  if (::avformat_open_input(&formatContext, url.toEncoded(), NULL, NULL) == 0)
    return BufferReaderBase::start(produceCallback, formatContext, true);

  return false;
}

void NetworkBufferReader::stop(void)
{
  BufferReaderBase::stop();
}

bool NetworkBufferReader::process(void)
{
  return BufferReaderBase::demux(BufferReaderBase::read());
}

QList<QUrl> NetworkBufferReader::resolveAsf(const QUrl &/*url*/)
{
/*  if (url.scheme() == "http")
  {
    SHttpClient httpClient;

    SHttpEngine::RequestMessage request(&httpClient);
    request.setHost(url.host(), url.port(80));
    request.setField("Icy-MetaData", "1");

    request.setRequest("HEAD", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

    SHttpEngine::ResponseMessage response = httpClient.blockingRequest(request, 5000);
    if ((response.status() == SHttpEngine::Status_Ok) &&
        (response.contentType() == "video/x-ms-asf"))
    {
      request.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

      response = httpClient.blockingRequest(request, 5000);
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
            {
              if (href.startsWith("http://"))
                result.append(resolveAsf(href));
              else
                result.append(href);
            }
          }

          if (!result.isEmpty())
            return result;
        }
  //      else
  //        qDebug() << line << col << error << corrected;
      }
    }
  }*/

  return QList<QUrl>();
}

} } // End of namespaces
