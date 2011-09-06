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

#include "photoserver.h"
#include <LXiStreamGui>
#include "mediaplayersandbox.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

PhotoServer::PhotoServer(MediaDatabase::Category category, QObject *parent)
  : PlaylistServer(category, parent, tr("Slideshow"))
{
}

PhotoServer::~PhotoServer()
{
}

PhotoServer::Stream * PhotoServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  const MediaServer::File file(request);
  if (file.fileName().startsWith("playlist."))
  {
    SSandboxClient::Priority priority = SSandboxClient::Priority_Normal;
    if (file.url().queryItemValue("priority") == "low")
      priority = SSandboxClient::Priority_Low;
    else if (file.url().queryItemValue("priority") == "high")
      priority = SSandboxClient::Priority_High;

    SSandboxClient * const sandbox = masterServer->createSandbox(priority);
    connect(sandbox, SIGNAL(consoleLine(QString)), SLOT(consoleLine(QString)));
    sandbox->ensureStarted();

    QStringList albums;
    albums += request.directory().mid(serverPath().length() - 1);

    QMultiMap<QString, QByteArray> files;
    while (!albums.isEmpty())
    {
      const QString path = albums.takeFirst();

      foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(MediaDatabase::Category_Photos, path))
      {
        const FileNode node = mediaDatabase->readNode(file.uid);
        if (!node.isNull())
          files.insert(path + file.name, node.toByteArray(-1));
      }

      foreach (const QString &album, mediaDatabase->allAlbums(MediaDatabase::Category_Photos))
      if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
        albums += album;
    }

    if (!files.isEmpty())
    {
      PluginSettings settings(pluginName());

      QUrl rurl;
      rurl.setPath(MediaPlayerSandbox::path + file.fileName());
      rurl.addQueryItem("playslideshow", QString::null);
      rurl.addQueryItem("slideduration", QString::number(settings.value("SlideDuration", 7500).toInt()));
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, file.url().queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      QByteArray content;
      foreach (const QByteArray &line, files)
        content += line + '\n';

      Stream *stream = new Stream(this, sandbox, request.path());
      if (stream->setup(rurl, content))
        return stream;

      delete stream;
    }

    disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
    masterServer->recycleSandbox(sandbox);

    return NULL;
  }
  else
    return MediaPlayerServer::streamVideo(request);
}

QList<PhotoServer::Item> PhotoServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> items;
  foreach (Item item, PlaylistServer::listItems(path, start, count))
  {
    if (!item.isDir)
    {
      item.played = false; // Not useful for photos.
      item.seekable = false;
    }

    items += item;
  }

  return items;
}

SHttpServer::ResponseMessage PhotoServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);

    // Check for DLNA request.
    const QString contentFeatures = QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii());
    const MediaProfiles::ImageProfile imageProfile = MediaProfiles::imageProfileFor(contentFeatures);
    switch (imageProfile)
    {
    case MediaProfiles::JPEG_TN:
    case MediaProfiles::JPEG_SM:
    case MediaProfiles::JPEG_MED:
    case MediaProfiles::JPEG_LRG:
      return sendPhoto(request, MediaDatabase::fromUidString(file.fileName()), "jpeg");

    case MediaProfiles::PNG_TN:
    case MediaProfiles::PNG_LRG:
      return sendPhoto(request, MediaDatabase::fromUidString(file.fileName()), "png");
    }

    if (file.fileName().endsWith(".png", Qt::CaseInsensitive) &&
        !file.fileName().endsWith("-thumb.png", Qt::CaseInsensitive))
    {
      return sendPhoto(request, MediaDatabase::fromUidString(file.fileName()), "png");
    }
    else if (file.fileName().endsWith(".jpeg", Qt::CaseInsensitive) &&
             !file.fileName().endsWith("-thumb.jpeg", Qt::CaseInsensitive))
    {
      return sendPhoto(request, MediaDatabase::fromUidString(file.fileName()), "jpeg");
    }
    else if (file.fileName().endsWith(".html", Qt::CaseInsensitive) &&
             !file.fileName().startsWith("playlist.")) // Show photo
    {
      return handleHtmlRequest(request, file);
    }
  }

  return PlaylistServer::httpRequest(request, socket);
}

SHttpServer::ResponseMessage PhotoServer::sendPhoto(const SHttpServer::RequestMessage &request, MediaDatabase::UniqueID uid, const QString &format) const
{
  const MediaServer::File file(request);
  const FileNode node = mediaDatabase->readNode(uid);

  if (!node.isNull())
  if (node.containsImage())
  {
    SSize size = node.programs().first().imageCodec.size();
    if (file.url().hasQueryItem("resolution"))
    {
      const QStringList sizeTxt = file.url().queryItemValue("resolution").split('x');
      if (sizeTxt.count() >= 2)
      {
        size.setWidth(qMax(0, sizeTxt[0].toInt()));
        size.setHeight(qMax(0, sizeTxt[1].toInt()));
      }
    }

    const QString contentFeatures = QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii());
    const MediaProfiles::ImageProfile imageProfile = MediaProfiles::imageProfileFor(contentFeatures);
    if (imageProfile != 0) // DLNA stream.
      MediaProfiles::correctFormat(imageProfile, size);

    SImage image(node.filePath());
    if (!image.isNull())
    {
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setField("Cache-Control", "no-cache");
      response.setContentType("image/" + format.toLower());

      if (!contentFeatures.isEmpty())
      {
        response.setField("transferMode.dlna.org", "Interactive");
        response.setField("contentFeatures.dlna.org", contentFeatures);
      }

      if (!request.isHead())
      {
        QByteArray jpgData;
        QBuffer buffer(&jpgData);
        buffer.open(QAbstractSocket::WriteOnly);

        if (!size.isNull())
          image = image.scaled(size.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        image.save(&buffer, format.toUpper().toAscii(), 80);
        buffer.close();

        response.setContent(buffer.data());
      }

      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

} } // End of namespaces
