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

#include "configserver.h"
#include "mediaplayerbackend.h"


#ifndef Q_OS_WIN
  const char        ConfigServer::dirSplit = ':';
#else
  const char        ConfigServer::dirSplit = ';';
#endif

ConfigServer::ConfigServer(Plugin *plugin, MasterServer *server)
  : BackendServer(QT_TR_NOOP("Settings"), plugin, server),
    plugin(plugin)
{
  // Ensure static initializers are invoked.
  drives();

  masterServer()->httpServer()->registerCallback(httpPath(), this);
}

ConfigServer::~ConfigServer()
{
  masterServer()->httpServer()->unregisterCallback(this);
}

SHttpServer::SocketOp ConfigServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.isEmpty() || file.endsWith(".html"))
    return handleHtmlRequest(request, socket, file);

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

const QSet<QString> & ConfigServer::hiddenDirs(void)
{
  static QSet<QString> h;

  if (__builtin_expect(h.isEmpty(), false))
  {
    const QDir root = QDir::root();

#ifndef Q_OS_WIN
    h += root.absoluteFilePath("bin");
    h += root.absoluteFilePath("boot");
    h += root.absoluteFilePath("dev");
    h += root.absoluteFilePath("etc");
    h += root.absoluteFilePath("lib");
    h += root.absoluteFilePath("proc");
    h += root.absoluteFilePath("sbin");
    h += root.absoluteFilePath("sys");
    h += root.absoluteFilePath("tmp");
    h += root.absoluteFilePath("usr");
    h += root.absoluteFilePath("var");
#else // Windows (paths need to be lower case)
    h += root.absoluteFilePath("windows").toLower();
    h += root.absoluteFilePath("program files").toLower();
#endif

    foreach (const QFileInfo &drive, QDir::drives())
    {
#ifndef Q_OS_WIN
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("lost+found");
#else // Windows (paths need to be lower case)
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("lost+found").toLower();
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("recycler").toLower();
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("system volume information").toLower();
#endif
    }
  }

  return h;
}
