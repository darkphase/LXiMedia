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

#include "mediadatabase.h"
#include "mediaplayersandbox.h"
#include "module.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

MediaDatabase     * MediaDatabase::self = NULL;

MediaDatabase * MediaDatabase::createInstance(BackendServer::MasterServer *masterServer)
{
  if (self)
    return self;
  else
    return self = new MediaDatabase(masterServer);
}

void MediaDatabase::destroyInstance(void)
{
  delete self;
}

MediaDatabase::MediaDatabase(BackendServer::MasterServer *masterServer, QObject *parent)
  : QObject(parent),
    lastPlayedFileName(lastPlayedFile()),
    probeSandbox(masterServer->createSandbox(SSandboxClient::Priority_Low)),
    preProbeItemCount(MediaServer::loadItemCount() * 2),
    preProbingCount(qMin(probeSandbox->maxSocketCount() / 2, QThread::idealThreadCount())),
    preProbing(preProbingCount)
{
  if (self != NULL)
    qFatal("Only one instance of the MediaDatabase class is allowed.");

  connect(probeSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  connect(probeSandbox, SIGNAL(terminated()), SLOT(sandboxTerminated()));

  cacheFile.setFileName(QDir::temp().absoluteFilePath(sApp->tempFileBase() + "mediafilecache"));
  if (cacheFile.open(QFile::ReadWrite))
    qDebug() << "MediaDatabase opened" << cacheFile.fileName();
  else
    qWarning() << "MediaDatabase could not open cache file";
}

MediaDatabase::~MediaDatabase()
{
  self = NULL;

  delete probeSandbox;

  cacheFile.remove();
}

SMediaInfo MediaDatabase::readNodeFormat(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    SMediaInfo node = readNodeCache(filePath);
    if (!node.isNull() && node.isFormatProbed())
      return node;

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probeformat=");
    request.setContent(filePath.toString().toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
    {
      foreach (const SMediaInfo &node, deserializeNodes(response.content()))
      if (!node.isNull())
      {
        writeNodeCache(node);

        return node;
      }
    }
  }

  return SMediaInfo();
}

SMediaInfoList MediaDatabase::readNodeFormat(const QList<Info> &files) const
{
  SMediaInfoList result;

  if (!files.isEmpty())
  {
    QByteArray probeFiles;
    QList<int> probeFilePos;
    foreach (const Info &file, files)
    {
      const SMediaInfo node = readNodeCache(file);
      if (!node.isNull() && node.isFormatProbed())
      {
        result += node;
      }
      else
      {
        probeFiles += file.path.toString().toUtf8() + '\n';
        probeFilePos += result.count();
        result += SMediaInfo();
      }
    }

    if (!probeFiles.isEmpty())
    {
      SSandboxClient::RequestMessage request(probeSandbox);
      request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probeformat=");
      request.setContent(probeFiles);

      const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
      if (response.status() == SHttpEngine::Status_Ok)
      foreach (const SMediaInfo &node, deserializeNodes(response.content()))
      {
        if (!node.isNull())
          writeNodeCache(node);

        if (!probeFilePos.isEmpty())
          result[probeFilePos.takeFirst()] = node;
      }
    }
  }

  return result;
}

SMediaInfo MediaDatabase::readNodeContent(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    SMediaInfo node = readNodeCache(filePath);
    if (!node.isNull() && node.isContentProbed())
      return node;

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probecontent=");
    request.setContent(filePath.toString().toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
    {
      foreach (const SMediaInfo &node, deserializeNodes(response.content()))
      if (!node.isNull())
      {
        writeNodeCache(node);

        return node;
      }
    }
  }

  return SMediaInfo();
}

QByteArray MediaDatabase::readThumbnail(const QUrl &filePath, const QSize &size, const QColor &backgroundColor, const QString &format) const
{
  if (!filePath.isEmpty())
  {
    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest(
        "POST",
        QByteArray(MediaPlayerSandbox::path) + "?readthumbnail="
        "&maxsize=" + SSize(size).toString().toAscii() +
        "&bgcolor=" + backgroundColor.name().mid(1).toAscii() +
        "&format=" + format.toAscii());

    request.setContent(filePath.toString().toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
      return response.content();
  }

  return QByteArray();
}

QByteArray MediaDatabase::readImage(const QUrl &filePath, const QSize &size, const QColor &backgroundColor, const QString &format) const
{
  if (!filePath.isEmpty())
  {
    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest(
        "POST",
        QByteArray(MediaPlayerSandbox::path) + "?readimage="
        "&maxsize=" + SSize(size).toString().toAscii() +
        "&bgcolor=" + backgroundColor.name().mid(1).toAscii() +
        "&format=" + format.toAscii());

    request.setContent(filePath.toString().toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
      return response.content();
  }

  return QByteArray();
}

void MediaDatabase::setLastPlayed(const QUrl &filePath, const QDateTime &lastPlayed)
{
  if (!filePath.isEmpty())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);
    settings.beginGroup("LastPlayed");

    // Remove obsolete items.
    foreach (const QString &key, settings.childKeys())
    if (key.length() != 40)
      settings.remove(key);

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(filePath.toEncoded());
    const QString key = hash.result().toHex();

    if (lastPlayed.isValid())
    {
      const int count =
          settings.value(key).toString().split(',').last().toInt() + 1;

      settings.setValue(
          key,
          lastPlayed.toString(Qt::ISODate) + ',' + QString::number(count));
    }
    else
      settings.remove(key);
  }
}

QPair<QDateTime, int> MediaDatabase::lastPlayed(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);
    settings.beginGroup("LastPlayed");

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(filePath.toEncoded());
    const QString key = hash.result().toHex();

    const QStringList value = settings.value(key).toString().split(',');
    if (value.count() >= 2)
      return qMakePair(QDateTime::fromString(value[0], Qt::ISODate), value[1].toInt());
  }

  return qMakePair(QDateTime(), -1);
}

bool MediaDatabase::isEmpty(const QUrl &dirPath) const
{
  int count = 1;
  bool result = listFiles(dirPath, 0, count).isEmpty();

  preProbeDir(dirPath, 0, false);

  return result;
}

SMediaInfoList MediaDatabase::listItems(const QUrl &dirPath, int start, int &count) const
{
  const QList<Info> files = listFiles(dirPath, start, count);
  const SMediaInfoList nodes = readNodeFormat(files);

  preProbeDir(dirPath, start + files.count(), true);

  QList<QUrl> dirPaths;
  foreach (const Info &file, files)
  if (file.isDir)
    dirPaths += file.path;

  foreach (const QUrl &path, dirPaths)
    preProbeDir(path, 0, false);

  return nodes;
}

SMediaInfoList MediaDatabase::representativeItems(const QUrl &dirPath, int &count) const
{
  count = -count; // List representative items.

  return readNodeFormat(listFiles(dirPath, 0, count));
}

void MediaDatabase::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SHttpEngine::Status_Ok)
  {
    if (response.contentType().endsWith(";mediainfo"))
    {
      foreach (const SMediaInfo &node, deserializeNodes(response.content()))
      if (!node.isNull())
        writeNodeCache(node);
    }
    else if (response.contentType().endsWith(";files"))
    {
      bool content = false;
      if (response.hasField("X-Content"))
        content = response.field("X-Content").toInt() != 0;

      int i = 0, total = 0;
      foreach (const Info &info, deserializeFiles(response.content(), total))
      if (info.isReadable && !info.isDir)
        preProbeQueue.insert(i++, PreProbe(info, 0, content));
    }

    Q_ASSERT(preProbing < preProbingCount);
    preProbing = qMin(preProbing + 1, preProbingCount);

    preProbeNext();
  }
}

void MediaDatabase::preProbeNext(void)
{
  int maxIter = preProbingCount;
  for (QMultiMap<int, PreProbe>::Iterator i = preProbeQueue.begin();
       (i != preProbeQueue.end()) && (preProbing > 0) && (maxIter-- > 0);
       i = preProbeQueue.erase(i))
  {
    if (i.key() < 0)
    {
      QUrl url;
      url.setPath(MediaPlayerSandbox::path);
      url.addQueryItem("listfiles", QString::null);
      url.addQueryItem("priority", QString::number(-1000));
      url.addQueryItem("start", QString::number(i->start));
      url.addQueryItem("count", QString::number(preProbeItemCount));

      SSandboxClient::RequestMessage request(probeSandbox);
      request.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
      request.setField("X-Content", i->content ? "1" : "0");
      request.setContent(i->info.path.toString().toUtf8());

      probeSandbox->sendRequest(request);

      preProbing = qMax(preProbing - 1, 0);
    }
    else
    {
      const SMediaInfo node = readNodeCache(i->info);
      if (node.isNull() || (i->content && !node.isContentProbed()))
      {
        QUrl url;
        url.setPath(MediaPlayerSandbox::path);
        url.addQueryItem(i->content ? "probecontent" : "probeformat", QString::null);
        url.addQueryItem("priority", QString::number(-1001));

        SSandboxClient::RequestMessage request(probeSandbox);
        request.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
        request.setContent(i->info.path.toString().toUtf8());

        probeSandbox->sendRequest(request);

        preProbing = qMax(preProbing - 1, 0);
      }
    }
  }

  if ((preProbing > 0) && !preProbeQueue.isEmpty())
    QTimer::singleShot(0, this, SLOT(preProbeNext()));
}

void MediaDatabase::sandboxTerminated(void)
{
  preProbing = preProbingCount;
  preProbeQueue.clear();
}

void MediaDatabase::flushCache(void) const
{
  if (cacheFile.isOpen())
  {
    qDebug() << "MediaDatabase flushing" << cacheFile.fileName();

    cacheFile.resize(0);
    cachePos.clear();
  }
}

QList<MediaDatabase::Info> MediaDatabase::listFiles(const QUrl &dirPath, int start, int &count) const
{
  SSandboxClient::RequestMessage request(probeSandbox);
  request.setRequest(
      "POST",
      QByteArray(MediaPlayerSandbox::path) +
      "?listfiles=&start=" + QByteArray::number(start) +
      "&count=" + QByteArray::number(count));
  request.setContent(dirPath.toString().toUtf8());

  QList<Info> result;
  const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
  if (response.status() == SHttpEngine::Status_Ok)
    result = deserializeFiles(response.content(), count);

  return result;
}

SMediaInfo MediaDatabase::readNodeCache(const Info &file) const
{
  if (cacheFile.isOpen())
  {
    foreach (qint64 pos, cachePos.values(qHash(file.path.toString())))
    if (cacheFile.seek(pos))
    {
      QXmlStreamReader reader(&cacheFile);
      if (reader.readNextStartElement())
      {
        SMediaInfo node;
        if (node.deserialize(reader))
        if ((node.filePath() == file.path) && (node.size() == file.size) &&
            (qAbs(node.lastModified().secsTo(file.lastModified)) <= 2))
        {
          return node;
        }
      }
      else // Corrupted cache
      {
        flushCache();
        break;
      }
    }
    else // Corrupted cache
    {
      flushCache();
      break;
    }
  }

  return SMediaInfo();
}

SMediaInfo MediaDatabase::readNodeCache(const QUrl &filePath) const
{
  if (cacheFile.isOpen())
  {
    foreach (qint64 pos, cachePos.values(qHash(filePath.toString())))
    if (cacheFile.seek(pos))
    {
      QXmlStreamReader reader(&cacheFile);
      if (reader.readNextStartElement())
      {
        SMediaInfo node;
        if (node.deserialize(reader))
        if (node.filePath() == filePath)
        {
          return node;
        }
      }
      else // Corrupted cache
      {
        flushCache();
        break;
      }
    }
    else // Corrupted cache
    {
      flushCache();
      break;
    }
  }

  return SMediaInfo();
}

void MediaDatabase::writeNodeCache(const SMediaInfo &node) const
{
  if (cacheFile.isOpen() && cacheFile.seek(cacheFile.size()))
  {
    cachePos.insert(qHash(node.filePath().toString()), cacheFile.pos());

    {
      QXmlStreamWriter writer(&cacheFile);
      writer.setAutoFormatting(false);
      node.serialize(writer);
    }

    cacheFile.write("\n");
  }
}

void MediaDatabase::preProbeDir(const QUrl &dirPath, int start, bool content) const
{
  Info info;
  info.isDir = true;
  info.isReadable = true;
  info.path = dirPath;

  preProbeQueue.insert(-1, PreProbe(info, start, content));

  QTimer::singleShot(250, const_cast<MediaDatabase *>(this), SLOT(preProbeNext()));
}

SMediaInfoList MediaDatabase::deserializeNodes(const QByteArray &data)
{
  SMediaInfoList result;

  QXmlStreamReader reader(data);
  if (reader.readNextStartElement() && (reader.name() == "nodes"))
  while (reader.readNextStartElement())
  {
    if (reader.name() == "mediainfo")
    {
      SMediaInfo node;
      if (node.deserialize(reader))
        result += node;
    }
    else
      reader.skipCurrentElement();
  }

  return result;
}

QList<MediaDatabase::Info> MediaDatabase::deserializeFiles(const QByteArray &data, int &total)
{
  QList<Info> result;

  QXmlStreamReader reader(data);
  if (reader.readNextStartElement() && (reader.name() == "files"))
  {
    total = reader.attributes().value("total").toString().toInt();

    while (reader.readNextStartElement())
    {
      if (reader.name() == "file")
      {
        Info info;
        info.isDir = reader.attributes().value("isdir") == "true";
        info.isReadable = reader.attributes().value("isreadable") == "true";
        info.size = reader.attributes().value("size").toString().toLongLong();
        info.lastModified = QDateTime::fromString(reader.attributes().value("lastmodified").toString(), Qt::ISODate);

        info.path = reader.readElementText();

        result += info;
      }
      else
        reader.skipCurrentElement();
    }
  }

  return result;
}

QString MediaDatabase::lastPlayedFile(void)
{
  const QFileInfo settingsFile = QSettings().fileName();

  return
      settingsFile.absolutePath() + "/" +
      settingsFile.completeBaseName() + "." +
      QString(Module::pluginName).replace(" ", "") +
      ".LastPlayed.db";
}

} } // End of namespaces
