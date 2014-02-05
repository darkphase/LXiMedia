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
#include "module.h"
#include <LXiStreamGui>
#include <QtConcurrent>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

MediaDatabase     * MediaDatabase::self = NULL;

MediaDatabase * MediaDatabase::createInstance()
{
  if (self)
    return self;
  else
    return self = new MediaDatabase();
}

void MediaDatabase::destroyInstance(void)
{
  delete self;
}

MediaDatabase::MediaDatabase(QObject *parent)
  : QThread(parent),
    lastPlayedFileName(lastPlayedFile()),
    mutex(QMutex::NonRecursive),
    preProbeRunning(true),
    preProbeItemCount(MediaServer::loadItemCount() * 2),
    preProbingCount(QThread::idealThreadCount()),
    preProbing(preProbingCount)
{
  if (self != NULL)
    qFatal("Only one instance of the MediaDatabase class is allowed.");

  cacheFile.setFileName(QDir::temp().absoluteFilePath(sApp->tempFileBase() + "mediafilecache"));
  if (cacheFile.open(QFile::ReadWrite))
    qDebug() << "MediaDatabase opened" << cacheFile.fileName();
  else
    qWarning() << "MediaDatabase could not open cache file";

  start(QThread::LowPriority);
}

MediaDatabase::~MediaDatabase()
{
  preProbeRunning = false;
  preProbeWaiting.wakeOne();
  wait();

  self = NULL;

  cacheFile.remove();
}

SMediaInfo MediaDatabase::readNodeFormat(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    SMediaInfo node = readNodeCache(filePath);
    if (!node.isNull() && node.isFormatProbed())
      return node;

    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      fileNode.probeFormat();
      writeNodeCache(fileNode);
    }

    return fileNode;
  }

  return SMediaInfo();
}

SMediaInfoList MediaDatabase::readNodeFormat(const QList<Info> &files) const
{
  SMediaInfoList result;

  if (!files.isEmpty())
  {
    QStringList probeFiles;
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
        probeFiles += file.path.toString();
        probeFilePos += result.count();
        result += SMediaInfo();
      }
    }

    if (!probeFiles.isEmpty())
    {
      struct T
      {
        static SMediaInfo probeFileFormat(const QString &path)
        {
          SMediaInfo fileNode(path);
          if (!fileNode.isNull())
            fileNode.probeFormat();

          return fileNode;
        }
      };

      QList< QFuture<SMediaInfo> > futures;
      foreach (const QString &path, probeFiles)
      if (!path.isEmpty())
        futures += QtConcurrent::run(&T::probeFileFormat, path);

      foreach (const QFuture<SMediaInfo> &future, futures)
      {
        writeNodeCache(future.result());
        result[probeFilePos.takeFirst()] = future.result();
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

    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      fileNode.probeContent();
      writeNodeCache(fileNode);
    }

    return fileNode;
  }

  return SMediaInfo();
}

SImage MediaDatabase::readThumbnail(const QUrl &filePath, const QSize &size) const
{
  if (!filePath.isEmpty())
  {
    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      const SVideoBuffer thumbnail = fileNode.readThumbnail(size);
      if (!thumbnail.isNull())
        return SImage(thumbnail, true);
    }
  }

  return SImage();
}

QImage MediaDatabase::readImage(const QUrl &filePath, const QSize &size, const QColor &backgroundColor) const
{
  QImage image = SImage::fromFile(filePath, size);
  if (!image.isNull())
  {
    QSize maxsize = size;
    if (maxsize.isNull())
      maxsize = image.size();

    if (maxsize != image.size())
    {
      QImage result(maxsize, QImage::Format_ARGB32);
      QPainter p;
      p.begin(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
        p.fillRect(result.rect(), backgroundColor);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
        p.drawImage(
            (result.width() / 2) - (image.width() / 2),
            (result.height() / 2) - (image.height() / 2),
            image);
      p.end();
      image = result;
    }

    return image;
  }

  return QImage();
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

void MediaDatabase::run()
{
  while (preProbeRunning)
  {
    int preProbeKey;
    PreProbe preProbe;
    {
      QMutexLocker l(&mutex);

      while (preProbeRunning && preProbeQueue.empty())
        preProbeWaiting.wait(&mutex);

      QMultiMap<int, PreProbe>::Iterator i = preProbeQueue.begin();
      if (i != preProbeQueue.end())
      {
        preProbeKey = i.key();
        preProbe = i.value();
        preProbeQueue.erase(i);
      }
      else
        continue;
    }

    if (preProbeKey < 0)
    {
      int count = 0;
      const QList<Info> files = listFiles(preProbe.info.path, 0, count);
      {
        QMutexLocker l(&mutex);

        int n = 0;
        foreach (const Info &info, files)
          preProbeQueue.insert(n++, PreProbe(info, 0, preProbe.content));
      }
    }
    else
    {
      const SMediaInfo node = readNodeCache(preProbe.info);
      if (node.isNull() || (preProbe.content && !node.isContentProbed()))
      {
        SMediaInfo fileNode(preProbe.info.path);
        if (!fileNode.isNull())
        {
          if (preProbe.content)
            fileNode.probeContent();
          else
            fileNode.probeFormat();

          writeNodeCache(fileNode);
        }
      }
    }
  }
}

void MediaDatabase::flushCache(void) const
{
  if (cacheFile.isOpen())
  {
    qWarning() << "MediaDatabase flushing" << cacheFile.fileName();

    cacheFile.resize(0);
    cachePos.clear();
  }
}

QList<MediaDatabase::Info> MediaDatabase::listFiles(const QUrl &dirPath, int start, int &count) const
{
  QList<Info> result;

  const bool returnAll = count == 0;
  int n = 0;

  SMediaFilesystem filesystem(dirPath);
  const QStringList items = filesystem.entryList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files,
        QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

  foreach (const QString &item, items)
  {
    // Filter items that need to be hidden.
    if (!item.endsWith(".db" , Qt::CaseInsensitive) &&
        !item.endsWith(".idx", Qt::CaseInsensitive) &&
        !item.endsWith(".nfo", Qt::CaseInsensitive) &&
        !item.endsWith(".srt", Qt::CaseInsensitive) &&
        !item.endsWith(".sub", Qt::CaseInsensitive) &&
        !item.endsWith(".txt", Qt::CaseInsensitive))
    {
      if (n++ >= start)
      {
        if (returnAll || (result.count() < count))
          result += Info(filesystem.readInfo(item), filesystem.filePath(item));
        else
          break;
      }
    }
  }

  count = items.count();

  return result;
}

SMediaInfo MediaDatabase::readNodeCache(const Info &file) const
{
  QMutexLocker l(&mutex);

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
  QMutexLocker l(&mutex);

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
  QMutexLocker l(&mutex);

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
  QMutexLocker l(&mutex);

  Info info;
  info.isDir = true;
  info.isReadable = true;
  info.path = dirPath;

  preProbeQueue.insert(-1, PreProbe(info, start, content));
  preProbeWaiting.wakeOne();
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
