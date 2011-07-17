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
    sandboxServer()
{
  const QDir tempDir = QDir::temp();
  if (tempDir.exists("sandbox_profile.svg"))
    sApp->enableProfiling(tempDir.absoluteFilePath("sandbox_profile.svg"));

  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  connect(&stopTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  connect(&sandboxServer, SIGNAL(busy()), &stopTimer, SLOT(stop()));
  connect(&sandboxServer, SIGNAL(idle()), &stopTimer, SLOT(start()));
}

Sandbox::~Sandbox()
{
  if (mode != "local")
    qDebug() << "Stopping sandbox process" << qApp->applicationPid();

  sandboxServer.close();

  QThreadPool::globalInstance()->waitForDone();

  foreach (BackendSandbox *sandbox, backendSandboxes)
  {
    sandbox->close();
    delete sandbox;
  }

  sApp->disableProfiling();

  if (mode != "local")
    qApp->exit(0);
}

void Sandbox::start(const QString &mode)
{
  this->mode = mode;
  stopTimer.start();

  sandboxServer.registerCallback("/", this);

  // Load plugins
  backendSandboxes = BackendSandbox::create(this);
  foreach (BackendSandbox *sandbox, backendSandboxes)
    sandbox->initialize(&sandboxServer);

  sandboxServer.initialize(mode);

  if (mode != "local")
  {
    stopTimer.setSingleShot(true);
    stopTimer.setInterval(60000);
    stopTimer.start();

    qDebug() << "Finished initialization of sandbox process" << qApp->applicationPid();
  }
}

SSandboxServer::SocketOp Sandbox::handleHttpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if ((request.method() == "GET") && (file.url().path() == "/"))
  {
    if (file.url().hasQueryItem("formats"))
    {
      QByteArray content;
      content += "AudioCodecs:" + SAudioEncoderNode::codecs().join("\t").toUtf8() + '\n';
      content += "VideoCodecs:" + SVideoEncoderNode::codecs().join("\t").toUtf8() + '\n';
      content += "Formats:" + SIOOutputNode::formats().join("\t").toUtf8() + '\n';

      return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_Ok, content, this);
    }
    else if (file.url().hasQueryItem("exit"))
    {
      QTimer::singleShot(5000, this, SLOT(deleteLater()));

      return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_Ok, this);
    }
  }

  return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_NotFound, this);
}

void Sandbox::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET");
}
