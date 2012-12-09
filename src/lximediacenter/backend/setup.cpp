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

#include "setup.h"

#ifdef Q_OS_WIN
# include <windows.h>
# ifndef SHTDN_REASON_FLAG_PLANNED
#  define SHTDN_REASON_FLAG_PLANNED 0x80000000
# endif
#endif

Setup::Setup(bool allowShutdown, QObject *parent)
  : MediaServer(parent),
    allowShutdown(allowShutdown)
{
}

void Setup::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;

  MediaServer::initialize(masterServer);
}

void Setup::close(void)
{
  MediaServer::close();
}

QString Setup::serverName(void) const
{
  return tr("Setup");
}

QString Setup::serverIconPath(void) const
{
  return "/img/settings.png";
}

QByteArray Setup::frontPageContent(void)
{
  return QByteArray();
}

QByteArray Setup::settingsContent(void)
{
  return QByteArray();
}

Setup::Stream * Setup::streamVideo(const SHttpServer::RequestMessage &request)
{
  if (QUrl(request.path()).path().endsWith("/shutdown") && allowShutdown)
  {
    qDebug() << "Shut down requested.";

#if defined(Q_OS_LINUX)
    // This only works if "your_username ALL = NOPASSWD: /sbin/shutdown" is added to sudoers.
    if (!QProcess::startDetached("sudo", QStringList() << "shutdown" << "-h" << "now"))
      qWarning() << "Failed to shut down.";
#elif defined(Q_OS_WIN)
    HANDLE token;
    ::OpenProcessToken(
          ::GetCurrentProcess(),
          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
          &token);

    TOKEN_PRIVILEGES privileges;
    memset(&privileges, 0, sizeof(privileges));
    ::LookupPrivilegeValue(
          NULL,
          SE_SHUTDOWN_NAME,
          &privileges.Privileges[0].Luid);

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    ::AdjustTokenPrivileges(
          token,
          FALSE,
          &privileges,
          0, NULL, 0);

    static const wchar_t message[] = L"Shutdown requested by LXiMediaCenter.";

    if (::GetLastError() != ERROR_SUCCESS)
    {
      qDebug() << "Failed to enable shutdown privilege.";
    }
    else if (::InitiateSystemShutdown(NULL, const_cast<wchar_t *>(message), 30, TRUE, FALSE) == FALSE)
    {
      qDebug() << "InitiateSystemShutdown failed" << ::GetLastError() << " trying WTSShutdownSystem.";

      HMODULE lib = ::LoadLibraryA("wtsapi32.dll");
      if (lib)
      {
        FARPROC proc = ::GetProcAddress(lib, "WTSShutdownSystem");
        if (proc)
        if (((BOOL (WINAPI *)(HANDLE, DWORD))proc)(NULL, 0x00000008) == FALSE)
          qWarning() << "WTSShutdownSystem failed.";

        ::FreeLibrary(lib);
      }
      else
        qWarning() << "Failed to load wtsapi32.dll.";
    }
#endif
  }

  return NULL;
}

SHttpServer::ResponseMessage Setup::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

QList<Setup::Item> Setup::listItems(const QString &virtualPath, int start, int &count)
{
  const bool returnAll = count == 0;

  QList<Item> items;
  if (allowShutdown)
    items += getItem(virtualPath + "/shutdown");

  QList<Item> result;
  for (int i=start, n=0; (i<items.count()) && (returnAll || (n<count)); i++, n++)
    result += items[i];

  count = items.count();

  return result;
}

Setup::Item Setup::getItem(const QString &virtualPath)
{
  Item item;
  if (virtualPath.endsWith("/shutdown"))
  {
    item.isDir = false;
    item.path = virtualPath;
    item.url = item.path;
    item.iconUrl = "/img/close.png";
    item.type = Item::Type_VideoBroadcast;
    item.title = tr("Shut down PC");
  }

  return item;
}

Setup::ListType Setup::listType(const QString &)
{
  return ListType_Thumbnails;
}
