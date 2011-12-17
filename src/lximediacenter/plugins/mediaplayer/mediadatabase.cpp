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
  connect(probeSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  connect(probeSandbox, SIGNAL(terminated()), SIGNAL(aborted()));

  connect (&cacheTimer, SIGNAL(timeout()), SLOT(flushCache()));
  cacheTimer.setSingleShot(true);

  cacheFile.setFileTemplate(QDir::temp().absoluteFilePath(QFileInfo(qApp->applicationFilePath()).baseName() + ".XXXXXX.mediafilecache"));
  if (cacheFile.open())
    qDebug() << "MediaDatabase opened" << cacheFile.fileName();
  else
    qWarning() << "MediaDatabase could not open cache file";
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
        writeNodeCache(filePath, response.content());

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

QByteArray MediaDatabase::readImage(const QString &filePath, const QSize &size, const QColor &backgroundColor, const QString &format) const
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

    const QByteArray key = node.quickHash().toBase64();
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

    const QByteArray key = node.quickHash().toBase64();
    return settings.value(key, QDateTime()).toDateTime();
  }

  return QDateTime();
}

int MediaDatabase::countItems(const QString &filePath) const
{
  int result = 0;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
    result += dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files).count();

  return result;
}

FileNodeList MediaDatabase::listItems(const QString &filePath, unsigned start, unsigned count) const
{
  const bool returnAll = count == 0;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
  {
    QStringList probeFiles;

    const QStringList items =
        dir.entryList(
            QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files,
            QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    for (int i=start, n=0; (i<items.count()) && (returnAll || (n<int(count))); i++, n++)
      probeFiles += dir.cleanPath(dir.absoluteFilePath(items[i]));

    if (!probeFiles.isEmpty())
      return readNodeFormat(probeFiles);
  }

  return FileNodeList();
}

FileNodeList MediaDatabase::representativeItems(const QString &filePath) const
{
  QStringList probeFiles;

  QList< QPair<QString, int> > paths;
  paths += qMakePair(filePath, 8);
  while (!paths.isEmpty())
  {
    const QPair<QString, int> path = paths.takeFirst();

    QDir dir(path.first);
    if (!path.first.isEmpty() && dir.exists())
    {
      const QStringList items =
          dir.entryList(
              QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files,
              QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

      for (int n = items.count(), ni = qMax(1, n / path.second), i = ni / 2; i < n; i += ni)
      {
        const QFileInfo info(dir.absoluteFilePath(items[i]));
        if (info.isDir())
          paths += qMakePair(info.absoluteFilePath(), 1);
        else
          probeFiles += dir.cleanPath(info.absoluteFilePath());
      }
    }
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

void MediaDatabase::flushCache(void)
{
  if (cacheFile.isOpen())
  {
    qDebug() << "MediaDatabase flushing" << cacheFile.fileName();

    cacheFile.resize(0);
    cachePos.clear();
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
        probeFiles += filePath.toUtf8() + '\n';
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

FileNode MediaDatabase::readNodeCache(const QString &filePath) const
{
  if (cacheFile.isOpen())
  {
    cacheTimer.start(cacheTimeout);

    foreach (qint64 pos, cachePos.values(qHash(filePath)))
    if (cacheFile.seek(pos))
    {
      const FileNode node = FileNode::fromByteArray(cacheFile.readLine());
      if (!node.isNull())
      if (node.filePath() == filePath)
      if (QFileInfo(filePath).lastModified() <= node.lastModified())
        return node;
    }
  }

  return FileNode();
}

void MediaDatabase::writeNodeCache(const QString &filePath, const QByteArray &data) const
{
  if (cacheFile.isOpen())
  {
    cacheTimer.start(cacheTimeout);

    if (cacheFile.seek(cacheFile.size()))
    {
      cachePos.insert(qHash(filePath), cacheFile.pos());
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
      Module::pluginName + ".LastPlayed.db";
}

} } // End of namespaces
