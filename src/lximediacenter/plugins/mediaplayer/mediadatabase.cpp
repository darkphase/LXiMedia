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
    probeSandbox(masterServer->createSandbox(SSandboxClient::Priority_Low))
{
  if (self != NULL)
    qFatal("Only one instance of the MediaDatabase class is allowed.");

  connect(probeSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  connect(probeSandbox, SIGNAL(terminated()), SIGNAL(aborted()));

  connect (&cacheTimer, SIGNAL(timeout()), SLOT(flushCache()));
  cacheTimer.setSingleShot(true);

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

FileNode MediaDatabase::readNode(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    FileNode node = readNodeCache(filePath);
    if (!node.isNull() && node.isContentProbed())
      return node;

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probecontent=");
    request.setContent(filePath.toString().toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
    {
      const FileNode node = FileNode::fromByteArray(response.content());
      if (!node.isNull())
      {
        writeNodeCache(filePath, response.content());

        return node;
      }
    }
  }

  return FileNode();
}

void MediaDatabase::queueReadNode(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    FileNode node = readNodeCache(filePath);
    if (!node.isNull() && node.isContentProbed())
    {
      emit const_cast<MediaDatabase *>(this)->nodeRead(node);
      return;
    }

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probecontent=");
    request.setContent(filePath.toString().toUtf8());

    probeSandbox->sendRequest(request);
  }
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

void MediaDatabase::setLastPlayed(const FileNode &node, const QDateTime &lastPlayed)
{
  if (!node.isNull())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);
    settings.beginGroup("LastPlayed");

    const QByteArray key =
        node.quickHash().toHex() +
        ("000000000000000" + QByteArray::number(node.size(), 16)).right(16);

    if (lastPlayed.isValid())
      settings.setValue(key, lastPlayed.toString(Qt::ISODate));
    else
      settings.remove(key);
  }
}

QDateTime MediaDatabase::lastPlayed(const FileNode &node) const
{
  if (!node.isNull())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);
    settings.beginGroup("LastPlayed");

    const QByteArray key =
        node.quickHash().toHex() +
        ("000000000000000" + QByteArray::number(node.size(), 16)).right(16);

    return QDateTime::fromString(settings.value(key).toString(), Qt::ISODate);
  }

  return QDateTime();
}

bool MediaDatabase::isEmpty(const QUrl &dirPath) const
{
  int count = 1;

  return listFiles(dirPath, 0, count).isEmpty();
}

FileNodeList MediaDatabase::listItems(const QUrl &dirPath, int start, int &count) const
{
  return readNodeFormat(listFiles(dirPath, start, count));
}

FileNodeList MediaDatabase::representativeItems(const QUrl &filePath) const
{
  QList<Info> probeFiles;

  QList< QPair<QUrl, int> > paths;
  paths += qMakePair(filePath, 4);
  while (!paths.isEmpty())
  {
    const QPair<QUrl, int> path = paths.takeFirst();

    int count = -path.second; // List representative items.
    foreach (const Info &item, listFiles(path.first, 0, count))
    if (item.isDir)
    {
      if (item.path.toString().startsWith(path.first.toString()))
        paths += qMakePair(item.path, 1);
    }
    else
      probeFiles += item;
  }

  if (!probeFiles.isEmpty())
    return readNodeFormat(probeFiles);

  return FileNodeList();
}

void MediaDatabase::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SHttpEngine::Status_Ok)
  {
    const FileNode node = FileNode::fromByteArray(response.content());
    if (!node.isNull())
    {
      writeNodeCache(node.filePath(), response.content());

      emit nodeRead(node);
    }
  }
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
  {
    QDomDocument doc("");
    if (doc.setContent(response.content()))
    {
      QDomElement rootElm = doc.documentElement();
      count = rootElm.attribute("total").toInt();

      for (QDomElement fileElm = rootElm.firstChildElement("file");
           !fileElm.isNull();
           fileElm = fileElm.nextSiblingElement("file"))
      {
        Info info;
        info.path = fileElm.text();
        info.isDir = fileElm.attribute("isDir").toInt() != 0;
        info.isReadable = fileElm.attribute("isReadable").toInt() != 0;
        info.size = fileElm.attribute("size").toLongLong();
        info.lastModified = QDateTime::fromString(fileElm.attribute("lastModified"), Qt::ISODate);

        result += info;
      }
    }
  }

  return result;
}

FileNodeList MediaDatabase::readNodeFormat(const QList<Info> &files) const
{
  FileNodeList result;

  if (!files.isEmpty())
  {
    QByteArray probeFiles;
    QList<int> probeFilePos;
    foreach (const Info &file, files)
    {
      const FileNode node = readNodeCache(file);
      if (!node.isNull() && node.isFormatProbed())
      {
        result += node;
      }
      else
      {
        probeFiles += file.path.toString().toUtf8() + '\n';
        probeFilePos += result.count();
        result += FileNode();
      }
    }

    if (!probeFiles.isEmpty())
    {
      SSandboxClient::RequestMessage request(probeSandbox);
      request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probeformat=");
      request.setContent(probeFiles);

      const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
      if (response.status() == SHttpEngine::Status_Ok)
      foreach (const QByteArray &entry, response.content().split('\n'))
      if (!entry.isEmpty() && !probeFilePos.isEmpty())
      {
        const FileNode node = FileNode::fromByteArray(entry);
        if (!node.isNull())
          writeNodeCache(node.filePath(), entry);

        result[probeFilePos.takeFirst()] = node;
      }
    }
  }

  return result;
}

FileNode MediaDatabase::readNodeCache(const Info &file) const
{
  if (cacheFile.isOpen())
  {
    cacheTimer.start(cacheTimeout);

    foreach (qint64 pos, cachePos.values(qHash(file.path.toString())))
    if (cacheFile.seek(pos))
    {
      const FileNode node = FileNode::fromByteArray(cacheFile.readLine());
      if (!node.isNull())
      {
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

  return FileNode();
}

FileNode MediaDatabase::readNodeCache(const QUrl &filePath) const
{
  if (cacheFile.isOpen())
  {
    cacheTimer.start(cacheTimeout);

    foreach (qint64 pos, cachePos.values(qHash(filePath.toString())))
    if (cacheFile.seek(pos))
    {
      const FileNode node = FileNode::fromByteArray(cacheFile.readLine());
      if (!node.isNull())
      {
        if (node.filePath() == filePath)
          return node;
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

  return FileNode();
}

void MediaDatabase::writeNodeCache(const QUrl &filePath, const QByteArray &data) const
{
  if (cacheFile.isOpen())
  {
    cacheTimer.start(cacheTimeout);

    if (cacheFile.seek(cacheFile.size()))
    {
      cachePos.insert(qHash(filePath.toString()), cacheFile.pos());
      cacheFile.write(data.trimmed());
      cacheFile.write("\n");
    }
  }
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
