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

#ifndef LXSTREAM_SMEDIAFILESYSTEM_H
#define LXSTREAM_SMEDIAFILESYSTEM_H

#include <QtCore>
#include <LXiCore>
#include "sinterfaces.h"
#include "export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SMediaFilesystem
{
public:
  typedef SInterfaces::Filesystem::Info Info;

public:
                                SMediaFilesystem(void);
                                SMediaFilesystem(const SMediaFilesystem &);
  explicit                      SMediaFilesystem(const QUrl &path);
  virtual                       ~SMediaFilesystem();

  static QStringList            protocols(void);

  SMediaFilesystem            & operator=(const SMediaFilesystem &);

  QUrl                          path(void) const;

  QStringList                   entryList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
  QUrl                          filePath(const QString &fileName) const;
  Info                          readInfo(const QString &fileName) const;
  QIODevice                   * openFile(const QString &fileName, QIODevice::OpenMode) const;

  static QIODevice            * open(const QUrl &filePath, QIODevice::OpenMode);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
