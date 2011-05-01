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

#include "sandbox.h"

Sandbox::Sandbox()
  : QObject(),
    mediaApp(),
    sandboxServer()
{
  mediaApp.installExcpetionHandler();
  //mediaApp.enableProfiling(QDir::temp().absoluteFilePath(QString::number(qApp->applicationPid()) + ".svg"));

  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  stopTimer.setSingleShot(true);
  stopTimer.setInterval(60000);
  stopTimer.start();

  connect(&stopTimer, SIGNAL(timeout()), SLOT(stop()));
  connect(&sandboxServer, SIGNAL(busy()), &stopTimer, SLOT(stop()));
  connect(&sandboxServer, SIGNAL(idle()), &stopTimer, SLOT(start()));
}

Sandbox::~Sandbox()
{
  QThreadPool::globalInstance()->waitForDone();

  sandboxServer.close();
}

void Sandbox::start(const QString &mode)
{
  stopTimer.start();

  // Load plugins
  backendSandboxes = BackendSandbox::create(this);
  foreach (BackendSandbox *sandbox, backendSandboxes)
    sandbox->initialize(&sandboxServer);

  sandboxServer.initialize(mode);

  qDebug() << "Finished initialization of sandbox process" << qApp->applicationPid();
}

void Sandbox::stop(void)
{
  foreach (BackendSandbox *sandbox, backendSandboxes)
  {
    sandbox->close();
    delete sandbox;
  }

  backendSandboxes.clear();

  qDebug() << "Stopping sandbox process" << qApp->applicationPid();

  qApp->exit(0);
}
