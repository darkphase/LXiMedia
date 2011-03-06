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

#include <iostream>

#include "backend.h"
#if defined(Q_OS_UNIX)
  #include "unixdaemon.h"
#elif defined(Q_OS_WIN)
  #include "windowsservice.h"
#endif


#if defined(Q_OS_UNIX)
  class BackendDeamon : public UnixDaemon
  {
  public:
    inline BackendDeamon(void)
      : UnixDaemon("lximcbackend", QString::null, true)
    {
    }

    virtual int run(void)
    {
      int exitCode = 0;
      do
      {
        Backend backend;
        backend.start();

        exitCode = qApp->exec();
      }
      while (exitCode == -1);

      qDebug() << "Daemon has finished with exit code" << exitCode;
      return exitCode;
    }

    virtual bool terminate(void)
    {
      qApp->quit();
      return true;
    }

  public:
    static BackendDeamon *instance;
  };

  BackendDeamon *BackendDeamon::instance = NULL;


  int main(int argc, char *argv[])
  {
    QCoreApplication app(argc, argv);
    app.setOrganizationName("LeX-Interactive");
    app.setOrganizationDomain("lximedia.sf.net");
    app.setApplicationName("LXiMediaCenter");
    app.setApplicationVersion(
#include "_version.h"
        );

    BackendDeamon *deamon = new BackendDeamon();

    if (argc < 2)
      std::cout << "Specify --help for more information." << std::endl;

    else if ((strcmp(argv[1], "--start") == 0) || (strcmp(argv[1], "-s") == 0))
      deamon->start();

    else if ((strcmp(argv[1], "--stop") == 0) || (strcmp(argv[1], "-q") == 0))
      deamon->stop();

    else if ((strcmp(argv[1], "--run") == 0) || (strcmp(argv[1], "-r") == 0))
      return deamon->run();

    else
    {
      std::cout << "LXiMediaCenter Backend deamon" << std::endl
                << "Usage: " << argv[0] << " --start | -s      (Starts the deamon)"  << std::endl
                << "       " << argv[0] << " --stop | -q       (Stops the deamon)"  << std::endl
                << "       " << argv[0] << " --run | -r        (Starts the backend from this executable)" << std::endl;

      return 1;
    }

    return 0;
  }
#elif defined(Q_OS_WIN)
  class BackendService : public WindowsService
  {
  public:
    inline BackendService(void)
        : WindowsService("LXiMediaCenter Backend"),
          running(true)
    {
    }

    virtual void run(void)
    {
      // Hack to get access to msleep()
      struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

      int xargc = 0;
      char **xargv = NULL;
      QCoreApplication app(xargc, xargv);

      while(running)
      {
        childProcess.start(app.applicationFilePath(), QStringList() << "--run");
        if (childProcess.waitForStarted())
        {
          while (!childProcess.waitForFinished(timeout))
          if (!running)
          if (!childProcess.waitForFinished(timeout))
            childProcess.kill();

          if (running && (childProcess.exitCode() == Backend::haltExitCode))
          {
            HANDLE token;
            TOKEN_PRIVILEGES privileges;
            if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
            {
              ::LookupPrivilegeValueA(NULL, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid);
              privileges.PrivilegeCount = 1;
              privileges.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

              ::AdjustTokenPrivileges(token, FALSE, &privileges, 0, (PTOKEN_PRIVILEGES)NULL, 0);

              if (::InitiateSystemShutdown(NULL, NULL, 30, FALSE, FALSE) == FALSE)
                qDebug() << "Shutdown failed:" << ::GetLastError();

              break;
            }
            else if (childProcess.exitCode() == 0)
              break;
          }
        }

        if (running)
          T::msleep(10000);
      }
    }

    virtual void stop(void)
    {
      running = false;

      // Find the local port for the child server.
      quint16 localPort = GlobalSettings::defaultBackendHttpPort();
      QFile portFile(QDir(GlobalSettings::applicationDataDir()).absoluteFilePath("server.port"));
      if (portFile.open(QFile::ReadOnly))
        localPort = portFile.readAll().toUShort();

      QTime timer; timer.start();
      QTcpSocket socket;
      socket.connectToHost(QHostAddress::LocalHost, localPort);
      if (socket.waitForConnected(qMax(timeout - timer.elapsed(), 10)))
      {
        QHttpRequestHeader header("GET", "/?exit=exit");
        socket.write(header.toString().toUtf8());

        while (socket.bytesToWrite() > 0)
        if (!socket.waitForBytesWritten(qMax(timeout - timer.elapsed(), 10)))
          break;

        if (socket.waitForReadyRead(qMax(timeout - timer.elapsed(), 10)))
          socket.readAll();

        socket.disconnectFromHost();
        socket.waitForDisconnected(qMax(timeout - timer.elapsed(), 10));
      }
    }

  public:
    static const int timeout = 10000;
    static BackendService *instance;

  private:
    QProcess childProcess;
    volatile bool running;
  };

  BackendService *BackendService::instance = NULL;


  int main(int argc, char *argv[])
  {
    BackendService::instance = new BackendService();

    if (argc < 2)
    {
      std::cout << "Starting service dispatcher ..." << std::endl
                << "Specify --help for more information." << std::endl;

      BackendService::instance->startServiceDispatcher();
    }
    else if ((strcmp(argv[1], "--install") == 0) || (strcmp(argv[1], "-i") == 0))
      BackendService::instance->install();

    else if ((strcmp(argv[1], "--uninstall") == 0) || (strcmp(argv[1], "-u") == 0))
      BackendService::instance->uninstall();

    else if ((strcmp(argv[1], "--run") == 0) || (strcmp(argv[1], "-r") == 0))
    {
        QCoreApplication app(argc, argv);
        app.setOrganizationName("LeX-Interactive");
        app.setOrganizationDomain("lximedia.sf.net");
        app.setApplicationName("LXiMediaCenter");
        app.setApplicationVersion(
#include "_version.h"
            );

        int exitCode = 0;
        do
        {
          Backend backend;
          backend.start();

          exitCode = app.exec();
        }
        while (exitCode == -1);

        return exitCode;
    }
    else
    {
      std::cout << "LXiMediaCenter Backend service" << std::endl
                << "Usage: " << argv[0] << " --install | -i    (Installs the service)"  << std::endl
                << "       " << argv[0] << " --uninstall | -u  (Uninstalls the service)"  << std::endl
                << "       " << argv[0] << " --run | -r        (Starts the backend from this executable)" << std::endl;

      return 1;
    }

    return 0;
  }

#else // Standard main function
  int main(int argc, char *argv[])
  {
    QCoreApplication app(argc, argv);
    app.setOrganizationName("LeX-Interactive");
    app.setOrganizationDomain("lximedia.sf.net");
    app.setApplicationName("LXiMediaCenter");
    app.setApplicationVersion(
#include "_version.h"
        );

    int exitCode = 0;
    do
    {
      Backend backend;
      backend.start();

      exitCode = app.exec();
    }
    while (exitCode  == -1);

    return exitCode;
  }
#endif
