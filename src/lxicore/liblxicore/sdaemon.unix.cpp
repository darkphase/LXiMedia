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
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

namespace LXiCore {

struct SDaemon::Data
{
  static const char             pidDir[];
  static SDaemon              * instance;
  static char                   name[256];
  static pid_t                  sessionID;

  static bool                   start(int &argc, char *argv[]);
  static bool                   stop(void);
  static void                   termHandler(int);
};

const char                      SDaemon::Data::pidDir[] = "/var/run/";
SDaemon                       * SDaemon::Data::instance = NULL;
char                            SDaemon::Data::name[256] = { '\0' };
pid_t                           SDaemon::Data::sessionID = 0;

SDaemon::SDaemon(const QString &name)
{
  qstrncpy(Data::name, name.toAscii(), sizeof(Data::name));
  Data::sessionID = 0;
  Data::instance = this;
}

SDaemon::~SDaemon()
{
  Data::instance = NULL;
  Data::name[0] = '\0';
  Data::sessionID = 0;
}

int SDaemon::main(int &argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Specify --help for more information." << std::endl;
  }
  else if ((strcmp(argv[1], "--start") == 0) || (strcmp(argv[1], "-s") == 0))
  {
    if (Data::start(argc, argv))
    {
      std::cout << "Started successfully." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "Could not start daemon." << std::endl;
      return 1;
    }
  }
  else if ((strcmp(argv[1], "--stop") == 0) || (strcmp(argv[1], "-q") == 0))
  {
    if (Data::stop())
    {
      std::cout << "Terminated successfully." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "Could not stop daemon." << std::endl;
      return 1;
    }
  }
  else if ((strcmp(argv[1], "--run") == 0) || (strcmp(argv[1], "-r") == 0))
  {
    return run(argc, argv);
  }
  else
  {
    std::cerr << Data::name << " daemon" << std::endl
              << "Usage: " << argv[0] << " --start | -s      (Starts the daemon)"  << std::endl
              << "       " << argv[0] << " --stop | -q       (Stops the daemon)"  << std::endl
              << "       " << argv[0] << " --run | -r        (Runs the daemon directly)" << std::endl;

    return 1;
  }

  return 0;
}

bool SDaemon::Data::start(int &argc, char *argv[])
{
  const pid_t pid = fork();

  if (pid > 0) // Succeeded
  {
    QFile file(QString(Data::pidDir) + Data::name + ".pid");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
      file.write(QByteArray::number(pid) + '\n');
    else
      std::cout << pid << std::endl;

    return true;
  }
  else if (pid < 0)
    return false; // Failed

  /////////////////////////////////////////////////////////////////////////////
  // The code below is only executed in the forked process, this actually
  // starts the deamon code.

  // Get a session ID
  Data::sessionID = setsid();

  // Make sure file I/O works normally
  umask(0);
  chdir("/");

  // Close these to prevent writing to the wrong terminal
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Specify a signal action handler so we can cleanly exit
  struct sigaction act;
  act.sa_handler = &Data::termHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGTERM, &act, NULL);

  // This starts the deamon
  Data::instance->run(argc, argv);

  return true;
}

bool SDaemon::Data::stop(void)
{
  QFile file(QString(Data::pidDir) + Data::name + ".pid");
  if (file.open(QIODevice::ReadOnly))
  {
    const pid_t pid = file.readAll().trimmed().toInt();
    if (pid > 0)
    {
      kill(pid, SIGTERM);

      int exitCode = 0, exited = 0;
      for (int i=0; i<10; i++)
      if (waitpid(-1, &exitCode, WNOHANG) != 0)
      {
        exited = 1;
        break;
      }
      else
        sleep(1);

      if (exited == 0)
        return kill(pid, SIGKILL) == 0;
      else
        return true;
    }
  }

  return false;
}

void SDaemon::Data::termHandler(int)
{
  if (Data::sessionID > 0)
  {
    QFile::remove(QString(Data::pidDir) + Data::name + ".pid");

    Data::instance->quit();
    return; // The application will quit nicely.
  }

  exit(0);
}

} // End of namespace
