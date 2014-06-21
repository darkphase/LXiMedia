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

#include "smediafilesystem.h"
#include <LXiCore>

namespace LXiStream {

struct SMediaFilesystem::Data
{
  QUrl path;
  SInterfaces::Filesystem *directory;
};

SMediaFilesystem::SMediaFilesystem(void)
  : d(new Data())
{
  d->directory = NULL;
}

SMediaFilesystem::SMediaFilesystem(const SMediaFilesystem &from)
  : d(new Data())
{
  d->path = from.d->path;

  d->directory = SInterfaces::Filesystem::create(NULL, d->path.scheme(), true);
  if (d->directory)
    d->directory->openDirectory(d->path);
}

SMediaFilesystem::SMediaFilesystem(const QUrl &path)
  : d(new Data())
{
  d->path = path;

  d->directory = SInterfaces::Filesystem::create(NULL, d->path.scheme(), true);
  if (d->directory)
    d->directory->openDirectory(d->path);
}

SMediaFilesystem::~SMediaFilesystem()
{
  delete d->directory;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SMediaFilesystem::protocols(void)
{
  return SInterfaces::Filesystem::available();
}

SMediaFilesystem & SMediaFilesystem::operator=(const SMediaFilesystem &from)
{
  d->path = from.d->path;

  delete d->directory;
  d->directory = SInterfaces::Filesystem::create(NULL, d->path.scheme(), true);
  if (d->directory)
    d->directory->openDirectory(d->path);

  return *this;
}

QUrl SMediaFilesystem::path(void) const
{
  return d->path;
}

QStringList SMediaFilesystem::entryList(QDir::Filters filters, QDir::SortFlags sort) const
{
  if (d->directory)
    return d->directory->entryList(filters, sort);

  return QStringList();
}

SMediaFilesystem::Info SMediaFilesystem::readInfo(const QString &fileName) const
{
  if (d->directory)
    return d->directory->readInfo(fileName);

  return SMediaFilesystem::Info();
}

QUrl SMediaFilesystem::filePath(const QString &fileName) const
{
  if (d->directory)
    return d->directory->filePath(fileName);

  return QUrl();
}

QIODevice * SMediaFilesystem::openFile(const QString &fileName, QIODevice::OpenMode openMode) const
{
  if (d->directory)
    return d->directory->openFile(fileName, openMode);

  return NULL;
}

QIODevice * SMediaFilesystem::open(const QUrl &filePath, QIODevice::OpenMode openMode)
{
  const QString path = filePath.path();
  const int lastSlash = path.lastIndexOf('/');

  QUrl dirPath(filePath);
  dirPath.setPath(path.left(lastSlash + 1));

  SMediaFilesystem instance(dirPath);
  return instance.openFile(path.mid(lastSlash + 1), openMode);
}

} // End of namespace
