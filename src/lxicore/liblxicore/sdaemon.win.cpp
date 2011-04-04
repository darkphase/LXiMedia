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

#include "sdaemon.h"
#include <windows.h>
#include <tchar.h>

namespace LXiCore {

struct SDaemon::Data
{
  static SDaemon              * instance;
  static TCHAR                  serviceName[512];
  static SERVICE_STATUS         serviceStatus;
  static SERVICE_STATUS_HANDLE  serviceStatusHandle;

  static void                   startDispatcher(void);
  static bool                   install(bool startImmediately = true);
  static bool                   uninstall(void);

  static void WINAPI            serviceControlHandler(DWORD);
  static void WINAPI            serviceMain(DWORD, TCHAR*[]);
};

SDaemon                       * SDaemon::Data::instance = NULL;
TCHAR                           SDaemon::Data::serviceName[512];
SERVICE_STATUS                  SDaemon::Data::serviceStatus;
SERVICE_STATUS_HANDLE           SDaemon::Data::serviceStatusHandle = NULL;

SDaemon::SDaemon(const QString &name)
{
  Data::instance = this;
  memcpy(Data::serviceName, name.unicode(), qMin((name.length() + 1) * sizeof(*Data::serviceName), sizeof(Data::serviceName)));
  Data::serviceName[(sizeof(Data::serviceName) / sizeof(*Data::serviceName)) - 1] = 0;
  Data::serviceStatusHandle = NULL;
}

SDaemon::~SDaemon()
{
  Data::instance = NULL;
}

int SDaemon::main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Starting service dispatcher." << std::endl
              << "Specify --help for more information." << std::endl;

    Data::startDispatcher();

    return 0;
  }
  else if ((strcmp(argv[1], "--install") == 0) || (strcmp(argv[1], "-i") == 0))
  {
    if (Data::install())
    {
      std::cout << "Installed successfully." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "Could not install service." << std::endl;
      return 1;
    }
  }
  else if ((strcmp(argv[1], "--uninstall") == 0) || (strcmp(argv[1], "-u") == 0))
  {
    if (Data::uninstall())
    {
      std::cout << "Uninstalled successfully." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "Could not uninstall service." << std::endl;
      return 1;
    }
  }
  else if ((strcmp(argv[1], "--run") == 0) || (strcmp(argv[1], "-r") == 0))
  {
    return run();
  }
  else
  {
    std::cerr << Data::name << " service" << std::endl
              << "Usage: " << argv[0] << " --install | -i    (Installs the service)"  << std::endl
              << "       " << argv[0] << " --uninstall | -u  (Uninstalls the service)"  << std::endl
              << "       " << argv[0] << " --run | -r        (Runs the service directly)" << std::endl;

    return 1;
  }

  return 0;
}

void SDaemon::Data::startDispatcher(void)
{
  SERVICE_TABLE_ENTRY serviceTable[] =
  {
    { Data::serviceName, Data::serviceMain },
    { 0, 0 }
  };

  ::StartServiceCtrlDispatcher(serviceTable);
}

bool SDaemon::Data::install(bool startImmediately)
{
  SC_HANDLE serviceControlManager = ::OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
  if (serviceControlManager)
  {
    TCHAR path[_MAX_PATH + 1];
    if (::GetModuleFileName(0, path, sizeof(path)/sizeof(path[0]) ) > 0)
    {
      SC_HANDLE service = ::CreateService(
          serviceControlManager,
          serviceName, serviceName,
          SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
          SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
          0, 0, 0, 0, 0);

      if (service)
      {
        if (startImmediately)
          ::StartService(service, 0, NULL);

        ::CloseServiceHandle(service);
      }
    }

    ::CloseServiceHandle(serviceControlManager);
  }

  return true;
}

bool SDaemon::Data::uninstall(void)
{
  SC_HANDLE serviceControlManager = ::OpenSCManager(0, 0, SC_MANAGER_CONNECT);
  if (serviceControlManager)
  {
    SC_HANDLE service = ::OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
    if (service)
    {
      SERVICE_STATUS serviceStatus;
      ::ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus);

      for (unsigned i=0; i<30; i++)
      {
        if (::QueryServiceStatus(service, &serviceStatus))
        if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
        {
          ::DeleteService(service);
          break;
        }

        ::Sleep(1000);
      }

      ::CloseServiceHandle(service);
    }

    ::CloseServiceHandle(serviceControlManager);
  }

  return true;
}

void WINAPI SDaemon::Data::serviceMain(DWORD argc, TCHAR *argv[])
{
  ::OutputDebugStringA("WindowsService::serviceMain");

  serviceStatus.dwServiceType = SERVICE_WIN32;
  serviceStatus.dwCurrentState = SERVICE_STOPPED;
  serviceStatus.dwControlsAccepted = 0;
  serviceStatus.dwWin32ExitCode = NO_ERROR;
  serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
  serviceStatus.dwCheckPoint = 0;
  serviceStatus.dwWaitHint = 0;

  serviceStatusHandle = ::RegisterServiceCtrlHandler(serviceName, serviceControlHandler);

  if ((serviceStatusHandle) && (instance != NULL))
  {
    // Set service status to starting
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);

    // Run the service
    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
    instance->run();

    // Stop the service
    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);

    // Set service status to stopped
    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
  }
}

void WINAPI SDaemon::Data::serviceControlHandler(DWORD controlCode)
{
  if (instance != NULL)
  switch (controlCode)
  {
  case SERVICE_CONTROL_INTERROGATE:
    break;

  case SERVICE_CONTROL_SHUTDOWN:  // Windows is shutting down.
  case SERVICE_CONTROL_STOP:      // The service is stopping.
    instance->quit();

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);

    return;

  case SERVICE_CONTROL_PAUSE:
    break;

  case SERVICE_CONTROL_CONTINUE:
    break;

  default:
    break;
  }

  ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

} // End of namespace
