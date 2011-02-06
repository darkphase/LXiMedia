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

#ifndef LXMEDIACENTER_FILESERVER_H
#define LXMEDIACENTER_FILESERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiStream>

namespace LXiMediaCenter {

class FileServer;
class FileServerDir;

template<class _type>
class FileServerHandle
{
public:
  inline                        FileServerHandle(_type *dir) : dir(dir)         { if (dir) dir->server()->lock.lockForWrite(); }
  inline                        FileServerHandle(const FileServerHandle &from) : dir(from.dir) { if (dir) dir->server()->lock.lockForWrite(); }
  inline                        ~FileServerHandle()                             { if (dir) dir->server()->lock.unlock(); }

  template<class _casttype>
  inline FileServerHandle(const FileServerHandle<_casttype> &from)
    : dir(qobject_cast<_type *>(from.ptr()))
  {
    if (dir) dir->server()->lock.lockForWrite();
  }

  inline FileServerHandle & operator=(const FileServerHandle &from)
  {
    if (dir) dir->server()->lock.unlock();
    dir = from.dir;
    if (dir) dir->server()->lock.lockForWrite();
    return *this;
  }

  template<class _casttype>
  inline FileServerHandle & operator=(const FileServerHandle<_casttype> &from)
  {
    if (dir) dir->server()->lock.unlock();
    dir = qobject_cast<_type *>(from.ptr());
    if (dir) dir->server()->lock.lockForWrite();
  }

  inline bool                   operator==(_type *cmp) const                    { return dir == cmp; }
  inline bool                   operator!=(_type *cmp) const                    { return dir != cmp; }

  inline _type                * operator->() const                              { return dir; }
  inline _type                * ptr(void) const                                 { return dir; }

private:
  _type                       * dir;
};

typedef FileServerHandle<FileServerDir> FileServerDirHandle;
typedef FileServerHandle<const FileServerDir> FileServerConstDirHandle;

class FileServer
{
friend class FileServerDir;
protected:
  explicit                      FileServer(void);
  virtual                       ~FileServer();

public:
  void                          setRoot(FileServerDir *);
  bool                          addDir(const QString &path, FileServerDir *);
  bool                          removeDir(const QString &path);
  FileServerDirHandle           findDir(const QString &path);

protected:
  inline FileServerDir        * root(void)                                      { return rootDir; }

public:
  mutable QReadWriteLock        lock;

private:
  FileServerDir               * rootDir;
};

class FileServerDir : public QObject
{
Q_OBJECT
public:
  explicit                      FileServerDir(FileServer *);
  virtual                       ~FileServerDir();

  virtual void                  addDir(const QString &name, FileServerDir *);
  virtual void                  removeDir(const QString &name);
  virtual void                  clear(void);
  virtual int                   count(void) const;

  virtual QStringList           listDirs(void);
  inline QStringList            listDirs(void) const                            { return const_cast<FileServerDir *>(this)->listDirs(); };
  FileServerDirHandle           findDir(const QString &name);
  FileServerConstDirHandle      findDir(const QString &name) const;

  inline FileServer           * server(void)                                    { return parent; }
  inline const FileServer     * server(void) const                              { return parent; }
  inline bool                   isEmpty(void) const                             { return count() == 0; }

private:
  FileServer            * const parent;
  QSet<FileServerDir *>         parentDirs;
  QMap<QString, FileServerDir *> subDirs;
};

} // End of namespace

#endif
