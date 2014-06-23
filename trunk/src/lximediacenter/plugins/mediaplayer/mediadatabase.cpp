/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

MediaDatabase     * MediaDatabase::self = NULL;
const int           MediaDatabase::itemCacheTimeout = 15000;

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
    fileSystemWatcher(this),
#ifdef MEDIADATABASE_USE_SANDBOX
    sandbox(NULL),
    sandboxTimer(this),
#endif
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

  connect(&fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));

#ifdef MEDIADATABASE_USE_SANDBOX
  connect(&sandboxTimer, SIGNAL(timeout()), SLOT(stopSandbox()));
  sandboxTimer.setSingleShot(true);
  sandboxTimer.setTimerType(Qt::VeryCoarseTimer);
#endif
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

#ifdef MEDIADATABASE_USE_SANDBOX
    startSandbox();
    SMediaInfo fileNode = sandbox->probeFormat(filePath);
    writeNodeCache(fileNode);
#else
    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      fileNode.probeFormat();
      writeNodeCache(fileNode);
    }
#endif

    return fileNode;
  }

  return SMediaInfo();
}

SMediaInfoList MediaDatabase::readNodeFormat(const QList<Info> &files) const
{
  SMediaInfoList result;

  if (!files.isEmpty())
  {
    QList<QUrl> probeFiles;
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
        probeFiles += file.path;
        probeFilePos += result.count();
        result += SMediaInfo();
      }
    }

#ifdef MEDIADATABASE_USE_SANDBOX
    if (!probeFiles.isEmpty())
    {
      startSandbox();
      foreach (const SMediaInfo &node, sandbox->probeFormat(probeFiles))
      {
        writeNodeCache(node);
        result[probeFilePos.takeFirst()] = node;
      }
    }
#else
    foreach (const QUrl &path, probeFiles)
      result[probeFilePos.takeFirst()] = readNodeFormat(path);
#endif
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

#ifdef MEDIADATABASE_USE_SANDBOX
    startSandbox();
    SMediaInfo fileNode = sandbox->probeContent(filePath);
    writeNodeCache(fileNode);
#else
    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      fileNode.probeContent();
      writeNodeCache(fileNode);
    }
#endif

    return fileNode;
  }

  return SMediaInfo();
}

SImage MediaDatabase::readThumbnail(const QUrl &filePath, const QSize &maxSize) const
{
  if (!filePath.isEmpty())
  {
#ifdef MEDIADATABASE_USE_SANDBOX
    startSandbox();
    return sandbox->readThumbnail(filePath, maxSize);
#else
    SMediaInfo fileNode(filePath);
    if (!fileNode.isNull())
    {
      const SVideoBuffer thumbnail = fileNode.readThumbnail(maxSize);
      if (!thumbnail.isNull())
        return SImage(thumbnail, true);
    }
#endif
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

void MediaDatabase::setLastPlaybackPosition(const QUrl &filePath, int position)
{
  if (!filePath.isEmpty())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(filePath.toEncoded());
    const QString key = hash.result().toHex();

    settings.beginGroup("LastPlaybackPosition");
    settings.setValue(key, position);
    settings.endGroup();

    emit itemChanged(filePath);
  }
}

int MediaDatabase::getLastPlaybackPosition(const QUrl &filePath) const
{
  if (!filePath.isEmpty())
  {
    QSettings settings(lastPlayedFileName, QSettings::IniFormat);

    // Upgrade old items.
    {
      QStringList oldKeys;
      settings.beginGroup("LastPlayed");
        foreach (const QString &key, settings.childKeys())
        {
          settings.remove(key);
          if (key.length() == 40)
            oldKeys += key;
        }
      settings.endGroup();

      if (!oldKeys.isEmpty())
      {
        settings.beginGroup("LastPlaybackPosition");
        foreach (const QString &key, oldKeys)
          settings.setValue(key, 99999);
        settings.endGroup();
      }
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(filePath.toEncoded());
    const QString key = hash.result().toHex();

    settings.beginGroup("LastPlaybackPosition");
    if (settings.contains(key))
      return settings.value(key).toInt();
  }

  return -1;
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
#ifdef MEDIADATABASE_USE_SANDBOX
  Sandbox *sandbox = NULL;
  QTime sandboxUsed;
#endif

  QList<QUrl> preProbePaths;
  bool preProbePathsContent = false;

  while (preProbeRunning)
  {
    int preProbeKey;
    PreProbe preProbe;
    {
      QMutexLocker l(&mutex);

      if (!preProbePaths.isEmpty() && preProbeRunning && preProbeQueue.empty())
      {
        l.unlock();

#ifdef MEDIADATABASE_USE_SANDBOX
        preProbeItems(sandbox, sandboxUsed, preProbePaths, preProbePathsContent);
#else
        preProbeItems(preProbePaths, preProbePathsContent);
#endif

        l.relock();
      }

#ifdef MEDIADATABASE_USE_SANDBOX
      while (sandbox && preProbeRunning && preProbeQueue.empty())
      if (!preProbeWaiting.wait(&mutex, qMax(0, sandboxTimeout - qAbs(sandboxUsed.elapsed()))))
      {
        delete sandbox;
        sandbox = NULL;
      }
#endif

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
      if (!preProbePaths.isEmpty())
      {
#ifdef MEDIADATABASE_USE_SANDBOX
        preProbeItems(sandbox, sandboxUsed, preProbePaths, preProbePathsContent);
#else
        preProbeItems(preProbePaths, preProbePathsContent);
#endif
      }

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
        if (!preProbePaths.isEmpty() &&
            ((preProbePathsContent != preProbe.content) ||
             (preProbePaths.count() >= (QThread::idealThreadCount() * 4))))
        {
#ifdef MEDIADATABASE_USE_SANDBOX
          preProbeItems(sandbox, sandboxUsed, preProbePaths, preProbePathsContent);
#else
          preProbeItems(preProbePaths, preProbePathsContent);
#endif
        }

        preProbePathsContent = preProbe.content;
        preProbePaths += preProbe.info.path;
      }
    }
  }

#ifdef MEDIADATABASE_USE_SANDBOX
  delete sandbox;
#endif
}

#ifdef MEDIADATABASE_USE_SANDBOX
void MediaDatabase::preProbeItems(Sandbox *&sandbox, QTime &sandboxUsed, QList<QUrl> &paths, bool content)
{
  sandboxUsed.start();
  if (sandbox == NULL)
    sandbox = new Sandbox(true);

  const SMediaInfoList nodes = content
      ? sandbox->probeContent(paths)
      : sandbox->probeFormat(paths);

  foreach (const SMediaInfo &node, nodes)
    writeNodeCache(node);

  paths.clear();
}
#else
void MediaDatabase::preProbeItems(QList<QUrl> &paths, bool content)
{
  foreach (const QUrl &path, paths)
  {
    SMediaInfo fileNode(path);
    if (!fileNode.isNull())
    {
      if (content)
        fileNode.probeContent();
      else
        fileNode.probeFormat();

      writeNodeCache(fileNode);
    }
  }

  paths.clear();
}
#endif

void MediaDatabase::directoryChanged(const QString &path)
{
  QUrl url;
  url.setScheme("file");
  url.setPath(path);

  {
    QMutexLocker l(&itemCacheMutex);

    QMap<QUrl, QPair<QStringList, QTime> >::Iterator i = itemCache.find(url);
    if (i != itemCache.end())
      i->second.addSecs(-itemCacheTimeout * 2);
  }

  emit itemChanged(url);
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

#ifdef MEDIADATABASE_USE_SANDBOX
void MediaDatabase::startSandbox(void) const
{
  if (sandbox == NULL)
    sandbox = new Sandbox(false, const_cast<MediaDatabase *>(this));

  sandboxTimer.start(sandboxTimeout);
}

void MediaDatabase::stopSandbox(void)
{
  delete sandbox;
  sandbox = NULL;
}
#endif

QList<MediaDatabase::Info> MediaDatabase::listFiles(const QUrl &dirPath, int start, int &count) const
{
  QList<Info> result;

  const bool returnAll = count == 0;

  SMediaFilesystem filesystem(dirPath);
  QStringList items;
  {
    QMutexLocker l(&itemCacheMutex);

    QMap<QUrl, QPair<QStringList, QTime> >::Iterator i = itemCache.find(dirPath);
    if ((i == itemCache.end()) ||
        ((start == 0) && (count >= 0) && (qAbs(i->second.elapsed()) > itemCacheTimeout)))
    {
      l.unlock();

      items = filesystem.entryList(
            QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files,
            QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

      // Filter items that need to be hidden.
      for (QStringList::Iterator j=items.begin(); j!=items.end(); )
      if (j->endsWith(".db" , Qt::CaseInsensitive) ||
          j->endsWith(".idx", Qt::CaseInsensitive) ||
          j->endsWith(".nfo", Qt::CaseInsensitive) ||
          j->endsWith(".srt", Qt::CaseInsensitive) ||
          j->endsWith(".sub", Qt::CaseInsensitive) ||
          j->endsWith(".txt", Qt::CaseInsensitive))
      {
        j = items.erase(j);
      }
      else
        j++;

      if (dirPath.scheme() == "file")
      {
        const QFileInfo info = dirPath.path();
        if (info.exists())
          fileSystemWatcher.addPath(info.absoluteFilePath());
      }

      l.relock();

      i = itemCache.insert(dirPath, qMakePair(items, QTime()));
    }
    else
      items = i->first;

    i->second.start();
  }

  if (count >= 0)
  {
    for (int i=start, n=0; (i<items.count()) && (returnAll || (n<count)); i++, n++)
      result += Info(filesystem.readInfo(items[i]), filesystem.filePath(items[i]));
  }
  else
  {
    for (int n = items.count(), ni = qMax(1, (n + (-count - 1)) / -count), i = ni / 2; i < n; i += ni)
      result += Info(filesystem.readInfo(items[i]), filesystem.filePath(items[i]));
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

  if (!node.isNull() && cacheFile.isOpen() && cacheFile.seek(cacheFile.size()))
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

QString MediaDatabase::lastPlayedFile(void)
{
  const QFileInfo settingsFile = QSettings().fileName();

  return
      settingsFile.absolutePath() + "/" +
      settingsFile.completeBaseName() + "." +
      QString(Module::pluginName).replace(" ", "") +
      ".LastPlayed.db";
}

#ifdef MEDIADATABASE_USE_SANDBOX
MediaDatabase::Sandbox::Sandbox(bool lowprio, QObject *parent)
  : QProcess(parent),
    lowprio(lowprio)
{
  restart();
}

MediaDatabase::Sandbox::~Sandbox()
{
  if (state() == QProcess::Running)
  {
    write("quit\n");
    do waitForBytesWritten(); while (bytesToWrite() && (state() == QProcess::Running));
    if (!waitForFinished(sandboxTimeout))
      kill();
  }
  else if (state() != QProcess::NotRunning)
    kill();
}

SMediaInfo MediaDatabase::Sandbox::probeFormat(const QUrl &path)
{
  return probe("format", path);
}

SMediaInfoList MediaDatabase::Sandbox::probeFormat(const QList<QUrl> &paths)
{
  return probe("format", paths);
}

SMediaInfo MediaDatabase::Sandbox::probeContent(const QUrl &path)
{
  return probe("content", path);
}

SMediaInfoList MediaDatabase::Sandbox::probeContent(const QList<QUrl> &paths)
{
  return probe("content", paths);
}

void MediaDatabase::Sandbox::restart()
{
  close();

  start(qApp->applicationFilePath(), QStringList() << "--sandbox");
  waitForStarted();
  if (lowprio)
  {
    write("setprio low\n");
    do waitForBytesWritten(); while (bytesToWrite() && (state() == QProcess::Running));
  }
}

SMediaInfo MediaDatabase::Sandbox::probe(const QByteArray &type, const QUrl &path)
{
  const SMediaInfoList result = probe("probe " + type + " " + path.toEncoded());
  if (!result.isEmpty())
    return result.first();

  return SMediaInfo();
}

SMediaInfoList MediaDatabase::Sandbox::probe(const QByteArray &type, const QList<QUrl> &paths)
{
  static const int maxlen = 65536; // Keep in sync with line in sandbox.cpp

  SMediaInfoList result;
  if (!paths.isEmpty())
  {
    QByteArray cmd = "probe " + type;
    int pos = 0;
    for (int i = 0; i < paths.size(); i++)
    {
      const QByteArray path = paths[i].toEncoded();
      if (cmd.length() + path.length() >= (maxlen - 8))
      {
        SMediaInfoList list = probe(cmd);
        if (state() != QProcess::Running)
        {
          // Crashed; probe one-by-one to get good ones.
          for (int j = pos; j < i; j++)
            result += probe("probe " + type + " " + paths[j].toEncoded());
        }

        pos = i;
        result += list;
        cmd = "probe " + type;
      }

      cmd += " " + path;
    }

    SMediaInfoList list = probe(cmd);
    if (state() != QProcess::Running)
    {
      // Crashed; probe one-by-one to get good ones.
      for (int j = pos; j < paths.size(); j++)
        result += probe("probe " + type + " " + paths[j].toEncoded());
    }

    result += list;
  }

  return result;
}

SMediaInfoList MediaDatabase::Sandbox::probe(const QByteArray &cmd)
{
  if (state() != QProcess::Running)
    restart();

  write(cmd + "\n");
  do waitForBytesWritten(); while (bytesToWrite() && (state() == QProcess::Running));
  do waitForReadyRead(); while (!canReadLine() && (state() == QProcess::Running));
  QByteArray data = readLine();
  if (!data.isEmpty() && !data.startsWith('#'))
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

  if (state() != QProcess::Running)
    qWarning() << "Sandbox crashed while processing:" << cmd;

  return SMediaInfoList();
}

SImage MediaDatabase::Sandbox::readThumbnail(const QUrl &path, const QSize &maxSize)
{
  if (state() != QProcess::Running)
    restart();

  const QByteArray cmd = "thumb " + SSize(maxSize).toString().toLatin1() + " " + path.toEncoded();
  write(cmd + "\n");
  do waitForBytesWritten(); while (bytesToWrite() && (state() == QProcess::Running));
  do waitForReadyRead(); while (!canReadLine() && (state() == QProcess::Running));
  QByteArray data = readLine();
  if (!data.isEmpty() && !data.startsWith('#'))
    return QImage::fromData(QByteArray::fromBase64(data));

  if (state() != QProcess::Running)
    qWarning() << "Sandbox crashed while processing:" << cmd;

  return SImage();
}
#endif

} } // End of namespaces
