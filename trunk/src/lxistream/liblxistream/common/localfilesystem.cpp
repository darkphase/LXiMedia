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

#include "localfilesystem.h"

namespace LXiStream {
namespace Common {

const char LocalFilesystem::scheme[] = "file";

LocalFilesystem::LocalFilesystem(const QString &, QObject *parent)
  : SInterfaces::Filesystem(parent),
    valid(false),
    root(true)
{
}

bool LocalFilesystem::openDirectory(const QUrl &path)
{
  if (path.path().isEmpty())
  {
    dir = QDir();
    root = valid = true;
    return true;
  }
  else
  {
    const QFileInfo info = path.path();
    if (!isHidden(info.canonicalFilePath()))
    {
      dir = QDir(info.absoluteFilePath());
      valid = true;
      root = false;
      return dir.exists();
    }
  }

  dir = QDir();
  root = valid = false;
  return false;
}

QStringList LocalFilesystem::entryList(QDir::Filters filter, QDir::SortFlags sort) const
{
  QStringList result;

  if (root)
  {
    foreach (const QFileInfo &info, dir.drives())
    {
      QString path = info.absolutePath();
      while (path.endsWith('/'))
        path = path.left(path.length() - 1);

      result += path;
    }
  }
  else if (valid)
  {
    foreach (const QString &fileName, dir.entryList(filter, sort))
    if (!fileName.startsWith('.'))
      result += fileName;
  }

  return result;
}

QUrl LocalFilesystem::filePath(const QString &fileName) const
{
  QUrl result;
  result.setScheme(scheme);
  if (root)
    result.setPath(fileName + '/');
  else if (valid)
    result.setPath(dir.absoluteFilePath(fileName));

  return result;
}

LocalFilesystem::Info LocalFilesystem::readInfo(const QString &fileName) const
{
  Info result;

  if (root)
  {
    foreach (const QFileInfo &info, QDir::drives())
    if (info.fileName() == fileName)
    {
      result.isDir = info.isDir();
      result.isReadable = info.isReadable();
      result.size = info.size();
      result.lastModified = info.lastModified();
    }
  }
  else if (valid)
  {
    const QFileInfo info(dir.absoluteFilePath(fileName));

    result.isDir = info.isDir();
    result.isReadable = info.isReadable();
    result.size = info.size();
    result.lastModified = info.lastModified();

    if (result.isDir)
      result.isReadable = result.isReadable && info.isExecutable() && !isHidden(info.canonicalFilePath());
  }

  return result;
}

QIODevice * LocalFilesystem::openFile(const QString &fileName) const
{
  if (!root && valid)
  {
    const QString path = dir.absoluteFilePath(fileName);
    if (!isHidden(QFileInfo(path).canonicalFilePath()))
    {
      QFile * const file = new QFile(path);
      if (file->open(QFile::ReadOnly))
        return file;

      delete file;
    }
  }

  return NULL;
}

bool LocalFilesystem::isHidden(const QString &path)
{
  static const Qt::CaseSensitivity caseSensitivity =
#if defined(Q_OS_UNIX)
      Qt::CaseSensitive;
#elif  defined(Q_OS_WIN)
      Qt::CaseInsensitive;
#else
#error Not implemented.
#endif

  static QStringList hiddenDirs;
  if (hiddenDirs.isEmpty())
  {
    const QDir root = QDir::root();

    hiddenDirs += QDir::tempPath();

#if defined(Q_OS_UNIX)
    hiddenDirs += root.absoluteFilePath("bin");
    hiddenDirs += root.absoluteFilePath("boot");
    hiddenDirs += root.absoluteFilePath("dev");
    hiddenDirs += root.absoluteFilePath("etc");
    hiddenDirs += root.absoluteFilePath("lib");
    hiddenDirs += root.absoluteFilePath("proc");
    hiddenDirs += root.absoluteFilePath("sbin");
    hiddenDirs += root.absoluteFilePath("sys");
    hiddenDirs += root.absoluteFilePath("usr");
    hiddenDirs += root.absoluteFilePath("var");
#endif

#if defined(Q_OS_MACX)
    hiddenDirs += root.absoluteFilePath("Applications");
    hiddenDirs += root.absoluteFilePath("cores");
    hiddenDirs += root.absoluteFilePath("Developer");
    hiddenDirs += root.absoluteFilePath("private");
    hiddenDirs += root.absoluteFilePath("System");
#endif

#if defined(Q_OS_WIN)
    hiddenDirs += root.absoluteFilePath("Program Files");
    hiddenDirs += root.absoluteFilePath("Program Files (x86)");
    hiddenDirs += root.absoluteFilePath("WINDOWS");
#endif

    foreach (const QFileInfo &drive, QDir::drives())
    {
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("lost+found");

#if defined(Q_OS_WIN)
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("RECYCLER");
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("System Volume Information");
#endif
    }
  }

  if (!path.isEmpty())
  {
    QString dirPath = path;
    if (!dirPath.endsWith('/'))
      dirPath += '/';

    foreach (const QString &hidden, hiddenDirs)
    {
      const QString path = hidden.endsWith('/') ? hidden : (hidden + '/');
      if (dirPath.startsWith(path, caseSensitivity))
        return true;
    }
  }

  return false;
}

} } // End of namespaces
