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
    imdbClient(masterServer->imdbClient()),
    probeSandbox(masterServer->createSandbox(SSandboxClient::Priority_Low))
{
}

MediaDatabase::~MediaDatabase()
{
  self = NULL;

  delete probeSandbox;
}

FileNode MediaDatabase::readNode(const QString &filePath, const QSize &thumbSize) const
{
  if (!filePath.isEmpty())
  {
    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probecontent=&thumbsize=" + SSize(thumbSize).toString().toAscii());
    request.setContent(filePath.toUtf8());

    const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
    if (response.status() == SHttpEngine::Status_Ok)
      return FileNode::fromByteArray(response.content());
  }

  return FileNode();
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

void MediaDatabase::setLastPlayed(const QString &filePath, const QDateTime &lastPlayed)
{
  if (!filePath.isEmpty())
  {
    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = filePath;
    key.replace('/', '|');
    key.replace('\\', '|');

    if (lastPlayed.isValid())
      settings.setValue(key, lastPlayed);
    else
      settings.remove(key);
  }
}

QDateTime MediaDatabase::lastPlayed(const QString &filePath) const
{
  if (!filePath.isEmpty())
  {
    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = filePath;
    key.replace('/', '|');
    key.replace('\\', '|');

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
  QTime timer; timer.start();

  const bool returnAll = count == 0;
  FileNodeList result;

  QDir dir(filePath);
  if (!filePath.isEmpty() && dir.exists())
  {
    QByteArray probeFiles;
    int numProbeFiles = 0;

    const QStringList files = dir.entryList(QDir::Files, QDir::Name);
    for (int i=start, n=0; (i<files.count()) && (returnAll || (n<int(count))); i++, n++)
    {
      probeFiles += dir.cleanPath(dir.absoluteFilePath(files[i])).toUtf8() + '\n';
      numProbeFiles++;
    }

    if (!probeFiles.isEmpty())
    {
      SSandboxClient::RequestMessage request(probeSandbox);
      request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probeformat=");
      request.setContent(probeFiles);

      const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
      if (response.status() == SHttpEngine::Status_Ok)
      foreach (const QByteArray &entry, response.content().split('\n'))
      if (!entry.isEmpty())
        result += FileNode::fromByteArray(entry);
    }
  }

  return result;
}

} } // End of namespaces
