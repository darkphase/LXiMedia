/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include <LXiCore>
#include <QtWidgets>
#include <iostream>

#include "backend.h"

int sandbox();

void configApp(void)
{
  qApp->setOrganizationName("LeX-Interactive");
  qApp->setOrganizationDomain("lximedia.sf.net");
  qApp->setApplicationName("LXiMediaCenter");
  qApp->setApplicationVersion(
#include "_version.h"
      );

#if defined(Q_OS_MACX)
  QDir appDir(qApp->applicationDirPath());
  if (appDir.dirName() == "MacOS")
  {
    appDir.cdUp();
    appDir.cd("PlugIns");
    qApp->setLibraryPaths(QStringList() << appDir.absolutePath());
  }
#elif defined(Q_OS_WIN)
  // Do not use the registry on Windows.
  QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
}

#if (defined(Q_OS_LINUX) || defined(Q_OS_WIN))
class Daemon : public SDaemon
{
public:
  inline Daemon(void)
    : SDaemon(name)
  {
  }

  virtual int run(int &argc, char *argv[])
  {
    QCoreApplication app(argc, argv); configApp();
    SApplication mediaApp(true);

    Backend backend;
    backend.start();

    const int exitCode = qApp->exec();

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

#if defined(Q_OS_LINUX)
const char Daemon::name[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Daemon::name[] = "LXiMediaCenter Backend";
#endif

int main(int argc, char *argv[])
{
  if ((argc >= 2) && (strcmp(argv[1], "--sandbox") == 0))
  {
    QCoreApplication app(argc, argv); configApp();
    SApplication mediaApp(false);

    return sandbox();
  }

  Daemon daemon;
  return daemon.main(argc, argv);
}
#elif defined(Q_OS_MACX)
int main(int argc, char *argv[])
{
  QApplication app(argc, argv); configApp();
  SApplication mediaApp(true);

  if ((argc >= 2) && (strcmp(argv[1], "--sandbox") == 0))
    return sandbox();

  int exitCode = 0;
  do
  {
    Backend backend;
    backend.start();

    exitCode = qApp->exec();
  }
  while (exitCode == -1);

  return exitCode;
}
#endif
