#include "backend.h"
#include "platform/messageloop.h"
#include <clocale>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

static int run(class messageloop &messageloop, const std::string &logfile)
{
    std::unique_ptr<class backend> backend;
    ::backend::recreate_backend = [&messageloop, &logfile, &backend]
    {
        backend = nullptr;
        backend.reset(new class backend(messageloop, logfile));
        if (!backend->initialize())
        {
            std::clog << "[" << backend.get() << "] failed to initialize backend; stopping." << std::endl;
            messageloop.stop(1);
        }
    };

    ::backend::recreate_backend();
    const int result = messageloop.run();
    ::backend::recreate_backend = nullptr;

    return result;
}

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    setlocale(LC_ALL, "");

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--daemon") == 0)
        {
            static const char pidfilename[] = "/var/run/lximcbackend.pid";
            auto pidfile = open(pidfilename, O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR);
            if (pidfile == -1)
            {
                std::cerr << "Failed to open PID file." << std::endl;
                return 1;
            }

            int result = 1;
            if (daemon(0, 0) == 0)
            {
                {
                    char buffer[64];
                    snprintf(buffer, sizeof(buffer), "%ld\n", long(getpid()));
                    write(pidfile, buffer, strlen(buffer));
                }

                static const char logfilename[] = "/var/log/lximcbackend.log";
                auto logfile = freopen(logfilename, "w", stderr);
                {
                    class messageloop messageloop;

                    result = run(messageloop, logfile ? logfilename : std::string());
                }
                if (logfile) fclose(logfile);
            }
            else
                std::cerr << "Failed to start daemon" << std::endl;

            close(pidfile);
            remove(pidfilename);

            return result;
        }

    class messageloop messageloop;
    return run(messageloop, std::string());
}

#elif defined(WIN32)
#include "platform/path.h"
#include <windows.h>

static const wchar_t service_name[] = L"LXiMediaCenter Backend";

static bool install_service()
{
    bool result = false;

    SC_HANDLE scm = ::OpenSCManagerW(0, 0, SC_MANAGER_CREATE_SERVICE);
    if (scm)
    {
        wchar_t path[MAX_PATH + 1];
        if (::GetModuleFileNameW(0, path, sizeof(path) / sizeof(*path)) > 0)
        {
            SC_HANDLE service = ::CreateServiceW(
                        scm,
                        service_name, service_name,
                        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                        SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
                        0, 0, 0,
                        L"NT AUTHORITY\\LocalService", 0);

            if (service)
            {
                result = ::StartServiceW(service, 0, NULL);
                ::CloseServiceHandle(service);
            }
        }

        ::CloseServiceHandle(scm);
    }

    return result;
}

static bool uninstall_service()
{
    bool result = false;

    SC_HANDLE scm = ::OpenSCManagerW(0, 0, SC_MANAGER_CONNECT);
    if (scm)
    {
        SC_HANDLE service = ::OpenServiceW(scm, service_name, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
        if (service)
        {
            SERVICE_STATUS status;
            ::ControlService(service, SERVICE_CONTROL_STOP, &status);

            for (unsigned i=0; i<30; i++)
            {
                if (::QueryServiceStatus(service, &status))
                    if (status.dwCurrentState == SERVICE_STOPPED)
                    {
                        result = ::DeleteService(service);
                        break;
                    }

                ::Sleep(1000);
            }

            ::CloseServiceHandle(service);
        }

        ::CloseServiceHandle(scm);
    }

    return result;
}

static SERVICE_STATUS service_status;
static SERVICE_STATUS_HANDLE service_status_handle = 0;
static std::unique_ptr<class messageloop> messageloop;

static void WINAPI service_control_handler(DWORD controlCode)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_SHUTDOWN:  // Windows is shutting down.
    case SERVICE_CONTROL_STOP:      // The service is stopping.
        if (messageloop)
        {
            messageloop->stop(0);
            service_status.dwCurrentState = SERVICE_STOP_PENDING;
        }

        break;

    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    default:
        break;
    }

    ::SetServiceStatus(service_status_handle, &service_status);
}

static void WINAPI service_main(DWORD argc, WCHAR *argv[])
{
    service_status.dwServiceType = SERVICE_WIN32;
    service_status.dwCurrentState = SERVICE_STOPPED;
    service_status.dwControlsAccepted = 0;
    service_status.dwWin32ExitCode = NO_ERROR;
    service_status.dwServiceSpecificExitCode = NO_ERROR;
    service_status.dwCheckPoint = 0;
    service_status.dwWaitHint = 0;

    service_status_handle = ::RegisterServiceCtrlHandlerW(service_name, &service_control_handler);
    if (service_status_handle)
    {
        service_status.dwCurrentState = SERVICE_START_PENDING;
        ::SetServiceStatus(service_status_handle, &service_status);

        messageloop.reset(new class messageloop());
        messageloop->post([]
        {
            service_status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
            service_status.dwCurrentState = SERVICE_RUNNING;
            ::SetServiceStatus(service_status_handle, &service_status);
        });

        const wchar_t *temp = _wgetenv(L"TEMP");
        if (temp == nullptr)
            temp = _wgetenv(L"TMP");

        FILE *logfile = nullptr;
        std::string logfilename;
        if (temp)
        {
            std::wstring filename = std::wstring(temp) + L"\\lximcbackend.log";
            logfile = _wfreopen(filename.c_str(), L"w", stderr);
            if (logfile)
                logfilename = from_windows_path(filename);
        }

        const int result = run(*messageloop, logfilename);

        messageloop = nullptr;

        if (logfile) fclose(logfile);

        service_status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
        service_status.dwCurrentState = SERVICE_STOPPED;
        service_status.dwWin32ExitCode = result;
        ::SetServiceStatus(service_status_handle, &service_status);
    }
}

static bool start_service_dispatcher()
{
    static const SERVICE_TABLE_ENTRYW service_table[] =
    {
        { const_cast<wchar_t *>(service_name), &service_main },
        { 0, 0 }
    };

    return ::StartServiceCtrlDispatcherW(service_table);
}

int main(int argc, const char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--install") == 0)
        {
            if (install_service())
                return 0;

            std::cerr << "Failed to install service." << std::endl;
            return 1;
        }
        else if (strcmp(argv[i], "--uninstall") == 0)
        {
            if (uninstall_service())
                return 0;

            std::cerr << "Failed to uninstall service." << std::endl;
            return 1;
        }
        else if (strcmp(argv[i], "--run") == 0)
        {
            class messageloop messageloop;
            return run(messageloop, std::string());
        }
        else
        {
            std::cerr << "Usage:" << std::endl
                      << "--install     Install service" << std::endl
                      << "--uninstall   Uninstall service" << std::endl
                      << "--run         Run directly" << std::endl;

            return 1;
        }
    }

    std::cout << "Starting service control dispatcher." << std::endl;
    return start_service_dispatcher() ? 1 : 0;
}

#endif
