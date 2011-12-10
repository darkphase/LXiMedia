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
const int           MediaDatabase::cacheSize = 4096;

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
  connect(probeSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  connect(probeSandbox, SIGNAL(terminated()), SIGNAL(aborted()));
}

MediaDatabase::~MediaDatabase()
{
  self = NULL;

  delete probeSandbox;
}

FileNode MediaDatabase::readNode(const QString &filePath) const
{
  if (!filePath.isEmpty())
  {
    FileNode node = readNodeCache(filePath);
    if (!node.isNull() && node.isContentProbed())
      return node;

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probecontent=");
    request.setContent(filePath.toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
    {
      const FileNode node = FileNode::fromByteArray(response.content());
      if (!node.isNull())
      {
        writeNodeCache(node);

        return node;
      }
    }
  }

  return FileNode();
}

void MediaDatabase::queueReadNode(const QString &filePath) const
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
    request.setContent(filePath.toUtf8());

    probeSandbox->sendRequest(request);
  }
}

QByteArray MediaDatabase::readImage(const QString &filePath, const QSize &size, const QString &format) const
{
  if (!filePath.isEmpty())
  {
    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?readimage=&maxsize=" + SSize(size).toString().toAscii() + "&format=" + format.toAscii());
    request.setContent(filePath.toUtf8());

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

    const QByteArray key = node.fastHash().toBase64();
    if (lastPlayed.isValid())
      settings.setValue(key, lastPlayed);
    else
      settings.remove(key);
  }
}

QDateTime MediaDatabase::lastPlayed(const FileNode &node) const
{
  if (!node.isNull())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);

    const QByteArray key = node.fastHash().toBase64();
    return settings.value(key, QDateTime()).toDateTime();
  }

  return QDateTime();
}

bool MediaDatabase::hasAlbum(const QString &filePath) const
{
  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
    return true;

  return false;
}

int MediaDatabase::countAlbums(const QString &filePath) const
{
  int result = 0;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
    result += dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count();

  return result;
}

QStringList MediaDatabase::getAlbums(const QString &filePath, unsigned start, unsigned count) const
{
  const bool returnAll = count == 0;
  QStringList result;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
  {
    const QStringList albums = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (int i=start, n=0; (i<albums.count()) && (returnAll || (n<int(count))); i++, n++)
      result += albums[i];
  }

  return result;
}

int MediaDatabase::countAlbumFiles(const QString &filePath) const
{
  int result = 0;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
    result += dir.entryList(QDir::Files).count();

  return result;
}

FileNodeList MediaDatabase::getAlbumFiles(const QString &filePath, unsigned start, unsigned count) const
{
  QStringList probeFiles;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
  {
    const bool returnAll = count == 0;

    const QStringList files = dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase);
    for (int i=start, n=0; (i<files.count()) && (returnAll || (n<int(count))); i++, n++)
      probeFiles += dir.cleanPath(dir.absoluteFilePath(files[i]));
  }

  return readNodeFormat(probeFiles);
}

void MediaDatabase::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if (response.status() == SHttpEngine::Status_Ok)
  {
    const FileNode node = FileNode::fromByteArray(response.content());
    if (!node.isNull())
    {
      writeNodeCache(node);

      emit nodeRead(node);
    }
  }
}

FileNodeList MediaDatabase::readNodeFormat(const QStringList &filePaths) const
{
  FileNodeList result;

  if (!filePaths.isEmpty())
  {
    QByteArray probeFiles;
    QList<int> probeFilePos;
    foreach (const QString &filePath, filePaths)
    {
      const FileNode node = readNodeCache(filePath);
      if (!node.isNull() && node.isFormatProbed())
      {
        result += node;
      }
      else
      {
        probeFiles += filePath + '\n';
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
          writeNodeCache(node);

        result[probeFilePos.takeFirst()] = node;
      }
    }
  }

  return result;
}

FileNode MediaDatabase::readNodeCache(const QString &filePath) const
{
  QMap<QString, QByteArray>::ConstIterator i = nodeCache.find(filePath);
  if (i != nodeCache.end())
  {
    const FileNode node = FileNode::fromByteArray(qUncompress(*i));
    if (QFileInfo(filePath).lastModified() <= node.lastModified())
      return node;
  }

  return FileNode();
}

void MediaDatabase::writeNodeCache(const FileNode &node) const
{
  QString filePath = node.filePath();
  filePath.squeeze();

  QMap<QString, QByteArray>::Iterator i = nodeCache.find(filePath);
  if (i != nodeCache.end())
  {
    *i = qCompress(node.toByteArray(-1), 9);
  }
  else
  {
    i = nodeCache.insert(filePath, qCompress(node.toByteArray(-1), 9));
    cacheQueue.enqueue(filePath);
  }

  i->squeeze();

  while (cacheQueue.size() > cacheSize)
    nodeCache.remove(cacheQueue.dequeue());
}

QString MediaDatabase::lastPlayedFile(void)
{
  const QFileInfo settingsFile = QSettings().fileName();

  return
      settingsFile.absolutePath() + "/" +
      settingsFile.completeBaseName() + "." +
      Module::pluginName + ".LastPlayed.db";
}

} } // End of namespaces
