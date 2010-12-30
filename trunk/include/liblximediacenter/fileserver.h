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
  FileServerDir               * findDir(const QString &path);

protected:
  inline FileServerDir        * root(void)                                      { return rootDir; }

public:
  mutable QMutex                mutex;

private:
  FileServerDir               * rootDir;
};

class FileServerDir : public QObject
{
Q_OBJECT
public:
  typedef QMap<QString, FileServerDir *>  DirMap;
  typedef QMap<QString, const FileServerDir *>  ConstDirMap;

public:
  explicit                      FileServerDir(FileServer *);
  virtual                       ~FileServerDir();

  virtual void                  addDir(const QString &name, FileServerDir *);
  virtual void                  removeDir(const QString &name);
  virtual void                  clear(void);
  virtual int                   count(void) const;

  virtual const DirMap        & listDirs(void);
  virtual FileServerDir       * findDir(const QString &name);

  inline FileServer           * server(void)                                    { return parent; }
  inline const FileServer     * server(void) const                              { return parent; }
  inline bool                   isEmpty(void) const                             { return count() == 0; }
  inline const ConstDirMap    & listDirs(void) const                            { return reinterpret_cast<const ConstDirMap &>(const_cast<FileServerDir *>(this)->listDirs()); }
  inline const FileServerDir  * findDir(const QString &name) const              { return const_cast<FileServerDir *>(this)->findDir(name); }

private:
  FileServer            * const parent;
  QSet<FileServerDir *>         parentDirs;
  DirMap                        subDirs;
};


} // End of namespace

#endif
