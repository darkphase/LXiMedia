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

#ifndef LXSTREAMCOMMON_LOCALFILESYSTEM_H
#define LXSTREAMCOMMON_LOCALFILESYSTEM_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {

class LocalFilesystem : public SInterfaces::Filesystem
{
Q_OBJECT
public:
                                LocalFilesystem(const QString &, QObject *parent);

public: // From SInterfaces::Filesystem
  virtual bool                  openDirectory(const QUrl &);

  virtual QStringList           entryList(QDir::Filters, QDir::SortFlags) const;
  virtual QUrl                  filePath(const QString &fileName) const;
  virtual Info                  readInfo(const QString &fileName) const;
  virtual QIODevice           * openFile(const QString &fileName, QIODevice::OpenMode) const;

public:
  static const char             scheme[];

private:
  static bool                   isHidden(const QString &);

private:
  QDir                          dir;
  bool                          valid;
  bool                          root;
};

} } // End of namespaces

#endif
