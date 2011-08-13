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
#include "mediadatabase.h"
#include "module.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char ConfigServer::dirSplit =
#if defined(Q_OS_UNIX)
    ':';
#elif  defined(Q_OS_WIN)
    ';';
#else
#error Not implemented.
#endif

const Qt::CaseSensitivity ConfigServer::caseSensitivity =
#if defined(Q_OS_UNIX)
    Qt::CaseSensitive;
#elif  defined(Q_OS_WIN)
    Qt::CaseInsensitive;
#else
#error Not implemented.
#endif

ConfigServer::ConfigServer(const QString &, QObject *parent)
  : BackendServer(parent),
    masterServer(NULL)
{
  // Ensure static initializers are invoked.
  drives();
}

ConfigServer::~ConfigServer()
{
}

void ConfigServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  this->mediaDatabase = MediaDatabase::createInstance(masterServer);

  BackendServer::initialize(masterServer);

  masterServer->httpServer()->registerCallback(serverPath(), this);
}

void ConfigServer::close(void)
{
  BackendServer::close();

  masterServer->httpServer()->unregisterCallback(this);
}

QString ConfigServer::pluginName(void) const
{
  return Module::pluginName;
}

QString ConfigServer::serverName(void) const
{
  return QT_TR_NOOP("Settings");
}

QString ConfigServer::serverIconPath(void) const
{
  return "/img/control.png";
}

SHttpServer::ResponseMessage ConfigServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);
    if (file.baseName().isEmpty() || (file.suffix() == "html"))
      return handleHtmlRequest(request, file);
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

const QSet<QString> & ConfigServer::hiddenDirs(void)
{
  static QSet<QString> h;

  if (h.isEmpty())
  {
    const QDir root = QDir::root();

#if defined(Q_OS_UNIX)
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
#endif

#if defined(Q_OS_MACX)
    h += root.absoluteFilePath("Applications");
    h += root.absoluteFilePath("cores");
    h += root.absoluteFilePath("Developer");
    h += root.absoluteFilePath("private");
    h += root.absoluteFilePath("System");
#endif

#if defined(Q_OS_WIN)
    h += root.absoluteFilePath("Program Files");
    h += root.absoluteFilePath("Program Files (x86)");
    h += root.absoluteFilePath("WINDOWS");
#endif

    foreach (const QFileInfo &drive, QDir::drives())
    {
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("lost+found");

#if defined(Q_OS_WIN)
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("RECYCLER");
      h += QDir(drive.absoluteFilePath()).absoluteFilePath("System Volume Information");
#endif
    }
  }

  return h;
}

bool ConfigServer::isHidden(const QString &path)
{
  const QFileInfo info(path);

  QString absoluteFilePath = info.absolutePath();
  if (!absoluteFilePath.endsWith('/')) absoluteFilePath += '/';

  QString canonicalFilePath = (info.exists() ? info.canonicalFilePath() : info.absolutePath());
  if (!canonicalFilePath.endsWith('/')) canonicalFilePath += '/';

  foreach (const QString &hidden, ConfigServer::hiddenDirs())
  {
    const QString path = hidden.endsWith('/') ? hidden : (hidden + '/');
    if (absoluteFilePath.startsWith(path, caseSensitivity) || canonicalFilePath.startsWith(path, caseSensitivity))
      return true;
  }

  return false;
}

} } // End of namespaces
