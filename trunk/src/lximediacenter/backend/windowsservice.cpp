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


#include "windowsservice.h"


WindowsService                * WindowsService::instance = NULL;
TCHAR                           WindowsService::serviceName[512];
SERVICE_STATUS                  WindowsService::serviceStatus;
SERVICE_STATUS_HANDLE           WindowsService::serviceStatusHandle = 0;
HANDLE                          WindowsService::stopServiceEvent = 0;


WindowsService::WindowsService(const QString &name)
{
  serviceName[0] = 0;

  if (instance == NULL)
  {
    instance = this;
    memcpy(serviceName, name.unicode(), qMin((name.length() + 1) * sizeof(*serviceName), sizeof(serviceName)));
    serviceName[(sizeof(serviceName) / sizeof(*serviceName)) - 1] = 0;
    serviceStatusHandle = 0;
    stopServiceEvent = 0;
  }
}

WindowsService::~WindowsService()
{
  if (instance == this)
    instance = NULL;
}

void WindowsService::waitForStopSignal(void)
{
  while(waitForStopSignal(0xFFFFFFFF) == false)
    continue;
}

bool WindowsService::waitForStopSignal(unsigned time)
{
  return WaitForSingleObject(stopServiceEvent, time) != WAIT_TIMEOUT;
}

void WindowsService::startServiceDispatcher(void)
{
  SERVICE_TABLE_ENTRY serviceTable[] =
  {
    { serviceName, serviceMain },
    { 0, 0 }
  };

  ::StartServiceCtrlDispatcher(serviceTable);
}

void WINAPI WindowsService::serviceControlHandler(DWORD controlCode)
{
  ::OutputDebugStringA("WindowsService::serviceControlHandler");

  if (instance != NULL)
  switch (controlCode)
  {
  // Windows is interrogating
  case SERVICE_CONTROL_INTERROGATE:
    instance->interrogate();
    break;

  // Windows is shutting down
  case SERVICE_CONTROL_SHUTDOWN:
    instance->shutdown();

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
    ::SetEvent(stopServiceEvent);

    return;

  // The service is stopping
  case SERVICE_CONTROL_STOP:
    instance->quit();

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
    ::SetEvent(stopServiceEvent);

    return;

  // The service is pausing
  case SERVICE_CONTROL_PAUSE:
    instance->pause();
    break;

  // The service is continuing
  case SERVICE_CONTROL_CONTINUE:
    instance->resume();
    break;

  // Other case
  default:
    if ((controlCode >= 128) && (controlCode <= 255))
      instance->user(controlCode);
    else
      instance->unrecognised(controlCode);

    break;
  }

  ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI WindowsService::serviceMain(DWORD argc, TCHAR *argv[])
{
  ::OutputDebugStringA("WindowsService::serviceMain");

  // This will load the JIT core dumper when available.
  const QString dumper = QCoreApplication::applicationDirPath() + "/exchndl.dll";
  if (QFile::exists(dumper))
    ::LoadLibrary((const WCHAR *)QDir::toNativeSeparators(dumper).utf16());

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

    // Initialise the service
    stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);
    instance->init(argc, (const char **)argv);

    // Run the service
    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
    instance->run();

    // Wait for the stop service event to kill things
    instance->waitForStopSignal();

    // Stop the service
    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);

    // Close the service
    CloseHandle(stopServiceEvent);
    stopServiceEvent = 0;
    instance->close();

    // Set service status to stopped
    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    ::SetServiceStatus(serviceStatusHandle, &serviceStatus);
  }
}

bool WindowsService::install(bool startImmediately)
{
  SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);

  if (serviceControlManager)
  {
    TCHAR path[_MAX_PATH + 1];
    if (GetModuleFileName(0, path, sizeof(path)/sizeof(path[0]) ) > 0)
    {
      SC_HANDLE service = ::CreateService(serviceControlManager,
                                          serviceName, serviceName,
                                          SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                                          SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
                                          0, 0, 0, 0, 0);

      if (service)
      {
        if (startImmediately)
          StartService(service, 0, NULL);

        ::CloseServiceHandle(service);
      }
    }

    ::CloseServiceHandle(serviceControlManager);
  }

  return true;
}

bool WindowsService::uninstall(void)
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

void WindowsService::interrogate(void)
{
}

void WindowsService::shutdown(void)
{
}

void WindowsService::quit(void)
{
}

void WindowsService::pause(void)
{
}

void WindowsService::resume(void)
{
}

void WindowsService::user(quint32)
{
}

void WindowsService::unrecognised(quint32)
{
}

void WindowsService::init(int, const char *[])
{
}

void WindowsService::close(void)
{
}
