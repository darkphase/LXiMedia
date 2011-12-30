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

#include "smbfilesystem.h"
#include <libsmbclient.h>

namespace LXiStream {
namespace SMBClientBackend {

const char  SMBFilesystem::scheme[] = "smb";

void SMBFilesystem::init(void)
{
  ::smbc_init(&SMBFilesystem::authenticate, 0);
}

void SMBFilesystem::close(void)
{
  paths().clear();
}

QByteArray SMBFilesystem::version(void)
{
  return ::smbc_version();
}

SMBFilesystem::SMBFilesystem(const QString &, QObject *parent)
  : SInterfaces::Filesystem(parent)
{
}

bool SMBFilesystem::openDirectory(const QUrl &path)
{
  QString share = path.path();
  if (share.startsWith('/'))
  {
    const int s = share.indexOf('/', 1);
    if (s > 1)
      share = share.mid(1, s - 1);
    else
      share = share.mid(1);
  }
  else
    return false;

  share = path.host() + '/' + share;

  QMap<QString, QUrl>::ConstIterator i = paths().find(share);
  if (i == paths().end())
    i = paths().insert(share, path);

  this->path = path;

  return true;
}

QStringList SMBFilesystem::entryList(QDir::Filters filter, QDir::SortFlags sort) const
{
  QMultiMap<QString, QString> result;

  const int dh = ::smbc_opendir(path.toEncoded(QUrl::RemoveUserInfo));
  if (dh >= 0)
  {
    static const int bufSize = 65536;
    char buffer[bufSize];

    for (int size = ::smbc_getdents(dh, reinterpret_cast<struct ::smbc_dirent *>(buffer), bufSize);
         size > 0;
         size = ::smbc_getdents(dh, reinterpret_cast<struct ::smbc_dirent *>(buffer), bufSize))
    {
      for (int i=0; i<size; )
      {
        struct ::smbc_dirent * const dirent = reinterpret_cast<struct ::smbc_dirent *>(buffer + i);
        if (dirent->dirlen > 0)
        {
          const QString name = QString::fromUtf8(dirent->name);

          QString index;
          if ((sort & QDir::Unsorted) == QDir::Name)
          {
            if ((sort & QDir::IgnoreCase) != 0)
              index = name.toLower();
            else
              index = name;
          }
          else
            index = ("0000000" + QString::number(result.count(), 16)).right(8);

          if (((dirent->smbc_type == SMBC_WORKGROUP) ||
               (dirent->smbc_type == SMBC_SERVER) ||
               (dirent->smbc_type == SMBC_FILE_SHARE) ||
               (dirent->smbc_type == SMBC_DIR)) &&
              ((filter & QDir::Dirs) != 0))
          {
            if ((sort & QDir::DirsFirst) != 0)
              index = 'A' + index;
            else if ((sort & QDir::DirsLast) != 0)
              index = 'C' + index;
            else
              index = 'B' + index;

            if ((name == ".") || (name == ".."))
            {
              if ((filter & QDir::NoDotAndDotDot) == 0)
                result.insert(index, name);
            }
            else if (name.startsWith('.'))
            {
              if ((filter & QDir::Hidden) != 0)
                result.insert(index, name);
            }
            else
              result.insert(index, name);
          }
          else if ((dirent->smbc_type == SMBC_FILE) && ((filter & QDir::Files) != 0))
          {
            index = 'B' + index;

            if (name.startsWith('.'))
            {
              if ((filter & QDir::Hidden) != 0)
                result.insert(index, name);
            }
            else
              result.insert(index, name);
          }

          i += dirent->dirlen;
        }
        else
          break;
      }
    }

    ::smbc_closedir(dh);
  }

  return result.values();
}

QUrl SMBFilesystem::filePath(const QString &fileName) const
{
  QUrl result = path;

  if (!fileName.isEmpty() && (fileName != "."))
  {
    QString path = result.path();
    if (!path.endsWith('/'))
      path += '/';

    path += fileName;

    result.setPath(path);
  }

  return result;
}

SMBFilesystem::Info SMBFilesystem::readInfo(const QString &fileName) const
{
  Info result;

  struct ::stat st;
  if (::smbc_stat(filePath(fileName).toEncoded(QUrl::RemoveUserInfo), &st) == 0)
  {
    result.isDir = (st.st_mode & S_IFDIR) != 0;
    result.isReadable = (st.st_mode & (S_IFDIR | S_IFREG)) != 0;
    result.size = st.st_size;
    result.lastModified = QDateTime::fromTime_t(st.st_mtime);
  }

  return result;
}

QIODevice * SMBFilesystem::openFile(const QString &fileName) const
{
  File * const file = new File(filePath(fileName));
  if (file->open(File::ReadOnly))
    return file;

  delete file;
  return NULL;
}

QMap<QString, QUrl> & SMBFilesystem::paths(void)
{
  static QMap<QString, QUrl> p;

  return p;
}

void SMBFilesystem::authenticate(
    const char *srv, const char *shr, char * /*wg*/, int /*wglen*/,
    char *un, int unlen, char *pw, int pwlen)
{
  QMap<QString, QUrl>::ConstIterator i = paths().find(
      QString::fromUtf8(srv) + '/' + QString::fromUtf8(shr));
  
  if (i != paths().end())
  {
    qstrncpy(un, i->userName().toUtf8(), unlen);
    qstrncpy(pw, i->password().toUtf8(), pwlen);
  }
}

SMBFilesystem::File::File(const QUrl &path)
  : path(path),
    fd(-1)
{
}

SMBFilesystem::File::~File()
{
  if (fd >= 0)
    ::smbc_close(fd);
}

bool SMBFilesystem::File::open(OpenMode mode)
{
  if ((fd < 0) && (mode == QIODevice::ReadOnly))
  if ((fd = ::smbc_open(path.toEncoded(QUrl::RemoveUserInfo), O_RDONLY, 0)) >= 0)
    return QIODevice::open(mode);

  return false;
}

void SMBFilesystem::File::close(void)
{
  QIODevice::close();

  if (fd >= 0)
    ::smbc_close(fd);

  fd = -1;
}

bool SMBFilesystem::File::isSequential(void) const
{
  return false;
}

bool SMBFilesystem::File::seek(qint64 pos)
{
  if (fd >= 0)
  if (::smbc_lseek(fd, pos, SEEK_SET) != off_t(-1))
    return QIODevice::seek(pos);

  return false;
}

qint64 SMBFilesystem::File::size(void) const
{
  if (fd >= 0)
  {
    struct ::stat st;
    if (::smbc_fstat(fd, &st) == 0)
      return st.st_size;
  }

  return -1;
}

qint64 SMBFilesystem::File::readData(char *data, qint64 maxSize)
{
  if (fd >= 0)
    return ::smbc_read(fd, data, maxSize);

  return -1;
}

qint64 SMBFilesystem::File::writeData(const char *, qint64)
{
  return -1;
}

} } // End of namespaces
