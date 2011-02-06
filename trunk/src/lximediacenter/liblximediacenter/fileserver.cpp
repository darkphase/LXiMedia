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

#include "fileserver.h"

namespace LXiMediaCenter {


FileServer::FileServer(void)
  : lock(QReadWriteLock::Recursive),
    rootDir(NULL)
{
}

FileServer::~FileServer()
{
  delete rootDir;
}

void FileServer::setRoot(FileServerDir *root)
{
  Q_ASSERT(root);

  delete rootDir;
  rootDir = root;
}

bool FileServer::addDir(const QString &path, FileServerDir *subDir)
{
  Q_ASSERT(!path.isEmpty());
  Q_ASSERT(subDir);

  QString cleanPath = path;
  while (cleanPath.endsWith('/'))
    cleanPath = cleanPath.left(cleanPath.length() - 1);

  const int pos = cleanPath.lastIndexOf('/');
  if (pos < 0)
    return false;

  FileServerDirHandle dir = findDir(cleanPath.left(pos + 1));
  if (dir == NULL)
    return false;

  const QString name = cleanPath.mid(pos + 1);
  if (name.length() == 0)
    return false;

  dir->addDir(name, subDir);
  return true;
}

bool FileServer::removeDir(const QString &path)
{
  Q_ASSERT(!path.isEmpty());

  QString cleanPath = path;
  while (cleanPath.endsWith('/'))
    cleanPath = cleanPath.left(cleanPath.length() - 1);

  const int pos = cleanPath.lastIndexOf('/');
  if (pos < 0)
    return false;

  FileServerDirHandle dir = findDir(cleanPath.left(pos + 1));
  if (dir == NULL)
    return false;

  const QString name = cleanPath.mid(pos + 1);
  if (name.length() == 0)
    return false;

  dir->removeDir(name);
  return true;
}

FileServerDirHandle FileServer::findDir(const QString &path)
{
  Q_ASSERT(!path.isEmpty());

  FileServerDirHandle dir = rootDir;

  QString cleanPath = path;
  while (cleanPath.startsWith('/'))
    cleanPath = cleanPath.mid(1);

  for (int pos = cleanPath.indexOf('/'); (dir != NULL) && (pos >= 0); pos = cleanPath.indexOf('/'))
  {
    dir = dir->findDir(cleanPath.left(pos));
    cleanPath = cleanPath.mid(pos + 1);
  }

  return dir;
}


FileServerDir::FileServerDir(FileServer *parent)
              :QObject(),
               parent(parent),
               parentDirs(),
               subDirs()
{
  Q_ASSERT(parent);
}

FileServerDir::~FileServerDir()
{
  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  foreach (FileServerDir *parentDir, parentDirs)
  {
    for (QMap<QString, FileServerDir *>::ConstIterator i=parentDir->subDirs.begin(); i!=parentDir->subDirs.end(); i++)
    if (i.value() == this)
    {
      parentDir->removeDir(i.key());
      break;
    }
  }

  for (QMap<QString, FileServerDir *>::Iterator i=subDirs.begin(); i!=subDirs.end(); i++)
  {
    (*i)->parentDirs.remove(this);
    if ((*i)->parentDirs.isEmpty())
      delete *i;
  }
}

void FileServerDir::addDir(const QString &name, FileServerDir *dir)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(dir);

  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  QMap<QString, FileServerDir *>::Iterator i = subDirs.find(name);
  if (i != subDirs.end())
  {
    (*i)->parentDirs.remove(this);
    subDirs.erase(i);
  }

  subDirs.insert(name, dir);
  dir->parentDirs.insert(this);
}

void FileServerDir::removeDir(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  QMap<QString, FileServerDir *>::Iterator i = subDirs.find(name);
  if (i != subDirs.end())
  {
    (*i)->parentDirs.remove(this);
    subDirs.erase(i);
  }
}

void FileServerDir::clear(void)
{
  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  foreach (FileServerDir *dir, subDirs)
    delete dir;

  subDirs.clear();
}

int FileServerDir::count(void) const
{
  return listDirs().count();
}

QStringList FileServerDir::listDirs(void)
{
  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  return subDirs.keys();
}

FileServerDirHandle FileServerDir::findDir(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  QMap<QString, FileServerDir *>::ConstIterator i = subDirs.find(name);
  if (i == subDirs.end())
  {
    // Try to update the dir
    listDirs();
    i = subDirs.find(name);
  }

  if (i != subDirs.end())
    return *i;

  return NULL;
}

FileServerConstDirHandle FileServerDir::findDir(const QString &name) const
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&parent->lock, __FILE__, __LINE__);

  QMap<QString, FileServerDir *>::ConstIterator i = subDirs.find(name);
  if (i == subDirs.end())
  {
    // Try to update the dir
    listDirs();
    i = subDirs.find(name);
  }

  if (i != subDirs.end())
    return *i;

  return NULL;
}



} // End of namespace
