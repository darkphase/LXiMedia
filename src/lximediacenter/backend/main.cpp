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

#include <LXiCore>
#include <iostream>

#include "backend.h"
#include "sandbox.h"

void configApp(void)
{
  qApp->setOrganizationName("LeX-Interactive");
  qApp->setOrganizationDomain("lximedia.sf.net");
  qApp->setApplicationName("LXiMediaCenter");
  qApp->setApplicationVersion(
#include "_version.h"
      );
}

class Daemon : public SDaemon
{
public:
  inline Daemon(void)
    : SDaemon(name)
  {
  }

  virtual int run(int &argc, char *argv[])
  {
    QCoreApplication app(argc, argv);
    configApp();

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

  virtual void quit(void)
  {
    qApp->quit();
  }

public:
  static const char name[];
};

#if defined(Q_OS_UNIX)
const char Daemon::name[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Daemon::name[] = "LXiMediaCenter Backend";
#endif

int main(int argc, char *argv[])
{
  if ((argc >= 3) && (strcmp(argv[1], "--sandbox") == 0))
  {
    QCoreApplication app(argc, argv);
    configApp();

    Sandbox sandbox;
    sandbox.start(argv[2]);

    return qApp->exec();
  }
  else
  {
    Daemon daemon;
    return daemon.main(argc, argv);
  }
}
