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
#include "sandbox.h"
#if defined(Q_OS_UNIX)
#include "unixdaemon.h"
#elif defined(Q_OS_WIN)
#include "windowsservice.h"
#endif

class Daemon
#if defined(Q_OS_UNIX)
  : public UnixDaemon
#elif defined(Q_OS_WIN)
  : public WindowsService
#endif
{
public:
  inline Daemon(void)
#if defined(Q_OS_UNIX)
    : UnixDaemon("lximcbackend", QString::null)
#elif defined(Q_OS_WIN)
    : WindowsService("LXiMediaCenter Backend")
#endif
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

  virtual bool quit(void)
  {
    qApp->quit();
    return true;
  }

  int runSandbox(const char *name, const char *mode)
  {
    Sandbox sandbox;
    sandbox.start(name, mode);

    return qApp->exec();
  }
};

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  app.setOrganizationName("LeX-Interactive");
  app.setOrganizationDomain("lximedia.sf.net");
  app.setApplicationName("LXiMediaCenter");
  app.setApplicationVersion(
#include "_version.h"
      );

  Daemon daemon;

  if (argc < 2)
  {
    std::cout << "Specify --help for more information." << std::endl;
  }
#if defined(Q_OS_UNIX)
  else if ((strcmp(argv[1], "--start") == 0) || (strcmp(argv[1], "-s") == 0))
  {
    daemon.start();
  }
  else if ((strcmp(argv[1], "--stop") == 0) || (strcmp(argv[1], "-q") == 0))
  {
    daemon.stop();
  }
#elif defined(Q_OS_WIN)
  else if ((strcmp(argv[1], "--install") == 0) || (strcmp(argv[1], "-i") == 0))
  {
    daemon.install();
  }
  else if ((strcmp(argv[1], "--uninstall") == 0) || (strcmp(argv[1], "-u") == 0))
  {
    daemon.uninstall();
  }
#endif
  else if ((strcmp(argv[1], "--run") == 0) || (strcmp(argv[1], "-r") == 0))
  {
    return daemon.run();
  }
  else if ((argc >= 4) && (strcmp(argv[1], "--sandbox") == 0))
  {
    return daemon.runSandbox(argv[2], argv[3]);
  }
  else
  {
#if defined(Q_OS_UNIX)
    std::cerr << "LXiMediaCenter Backend daemon" << std::endl
              << "Usage: " << argv[0] << " --start | -s      (Starts the daemon)"  << std::endl
              << "       " << argv[0] << " --stop | -q       (Stops the daemon)"  << std::endl
#elif defined(Q_OS_WIN)
    std::cerr << "LXiMediaCenter Backend service" << std::endl
              << "Usage: " << argv[0] << " --install | -i    (Installs the service)"  << std::endl
              << "       " << argv[0] << " --uninstall | -u  (Uninstalls the service)"  << std::endl
#else
    std::cerr << "LXiMediaCenter Backend" << std::endl
#endif
              << "       " << argv[0] << " --run | -r        (Starts the backend from this executable)" << std::endl;

    return 1;
  }

  return 0;
}
