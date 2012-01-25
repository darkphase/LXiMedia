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

#ifndef __FILENODE_H
#define __FILENODE_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class FileNode;
typedef QList<FileNode> FileNodeList;

class FileNode : public SMediaInfo
{
public:
  inline                        FileNode(void) : SMediaInfo() { }
  inline                        FileNode(const FileNode &from) : SMediaInfo(from) { }
  inline                        FileNode(const SMediaInfo &from) : SMediaInfo(from) { }
  inline explicit               FileNode(const QUrl &path) : SMediaInfo(path) { }
  inline explicit               FileNode(const QSharedDataPointer<ProbeInfo> &pi) : SMediaInfo(pi) { }

  inline FileNode             & operator=(const FileNode &from) { SMediaInfo::operator=(from); return *this; }
  inline FileNode             & operator=(const SMediaInfo &from) { SMediaInfo::operator=(from); return *this; }

  bool                          isFormatProbed(void) const;
  bool                          isContentProbed(void) const;

  QByteArray                    probeFormat(int = 1);
  QByteArray                    probeContent(int = 1);

  QByteArray                    toByteArray(int = 1) const;
  static FileNode               fromByteArray(const QByteArray &);
};

} } // End of namespaces

#endif
