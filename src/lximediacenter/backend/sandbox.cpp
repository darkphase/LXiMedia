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

const QEvent::Type  Sandbox::exitEventType = QEvent::Type(QEvent::registerEventType());

Sandbox::Sandbox()
  : QObject(),
    sandboxServer(),
    streamApp(NULL)
{
  // Initialize LXiStream
  QDir logDir(GlobalSettings::applicationDataDir() + "/log");
  if (!logDir.exists())
    logDir.mkpath(logDir.absolutePath());

  streamApp =
      new SApplication(
          SApplication::Initialize_Default | SApplication::Initialize_LogToConsole,
          logDir.absolutePath());

  //SThreadPool::globalInstance()->enableTrace("/tmp/threadpool.svg");

  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  qDebug() << "Starting sandbox process" << qApp->applicationPid();

  stopTimer.setSingleShot(true);
  stopTimer.setInterval(30000);
  connect(&stopTimer, SIGNAL(timeout()), SLOT(stop()));
  connect(&sandboxServer, SIGNAL(busy()), &stopTimer, SLOT(stop()));
  connect(&sandboxServer, SIGNAL(idle()), &stopTimer, SLOT(start()));
}

Sandbox::~Sandbox()
{
  qDebug() << "Stopping sandbox process" << qApp->applicationPid();

  foreach (BackendPlugin *plugin, backendPlugins)
    delete plugin;

  QThreadPool::globalInstance()->waitForDone();

  qDebug() << "Stopped sandbox process" << qApp->applicationPid();

  sandboxServer.close();

  // Shutdown LXiStream
  delete streamApp;
}

void Sandbox::start(const QString &name, const QString &mode)
{
  sandboxServer.initialize(name, mode);

  // Load plugins
  backendPlugins = BackendPlugin::loadPlugins();
  foreach (BackendPlugin *backendPlugin, backendPlugins)
  if (backendPlugin)
    backendPlugin->registerSandbox(&sandboxServer);

  stopTimer.start();

  qDebug() << "Finished initialization of sandbox process" << qApp->applicationPid();
}

void Sandbox::stop(void)
{
  qApp->exit(0);
}

void Sandbox::customEvent(QEvent *e)
{
  if (e->type() == exitEventType)
    stop();
  else
    QObject::customEvent(e);
}
