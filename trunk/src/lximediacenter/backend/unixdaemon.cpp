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

#include "unixdaemon.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "backend.h"


UnixDaemon * UnixDaemon::instance = NULL;

UnixDaemon::UnixDaemon(const QString &name, const QString &pidFile)
           :name(name),
            pidFile(pidFile),
            sessionID(0)
{
  instance = this;
}

UnixDaemon::~UnixDaemon()
{
  instance = NULL;
}

pid_t UnixDaemon::start(void)
{
  const pid_t pid = fork();

  if (pid > 0) // Succeeded
  {
    if (!pidFile.isEmpty())
    {
      QFile file(pidFile);
      if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QByteArray::number(pid) + '\n');
    }
    else
      std::cout << QByteArray::number(pid).data() << std::endl;

    return pid;
  }
  else if (pid < 0)
    return pid; // Failed

  /////////////////////////////////////////////////////////////////////////////
  // The code below is only executed in the forked process, this actually
  // starts the deamon code.
  startChild();

  return 0;
}

bool UnixDaemon::stop(void)
{
  if (!pidFile.isEmpty())
  {
    QFile file(pidFile);
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
  }

  return false;
}

bool UnixDaemon::quit(void)
{
  return false;
}

int UnixDaemon::startChild(void)
{
  // Get a session ID
  sessionID = setsid();

  // Make sure file I/O works normally
  umask(0);
  chdir("/");

  // Close these to prevent writing to the wrong terminal
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Specify a signal action handler so we can cleanly exit
  struct sigaction act;
  act.sa_handler = &termHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGTERM, &act, NULL);

  // This starts the deamon
  return run();
}

void UnixDaemon::termHandler(int)
{
  if (instance)
  if (instance->sessionID > 0)
  {
    if (!instance->pidFile.isEmpty())
      QFile(instance->pidFile).remove();

    if (instance->quit())
      return; // The application will quit nicely.
  }

  exit(0);
}
