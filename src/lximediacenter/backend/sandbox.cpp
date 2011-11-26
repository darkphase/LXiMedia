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

  sandboxServer.registerCallback("/", this);

  // Load plugins
  backendSandboxes = BackendSandbox::create(this);
  foreach (BackendSandbox *sandbox, backendSandboxes)
    sandbox->initialize(&sandboxServer);

  sandboxServer.initialize(mode);

  if (mode != "local")
  {
    connect(&stopTimer, SIGNAL(timeout()), SLOT(deleteLater()));
    connect(&sandboxServer, SIGNAL(busy()), &stopTimer, SLOT(stop()));
    connect(&sandboxServer, SIGNAL(idle()), &stopTimer, SLOT(start()));

    stopTimer.setSingleShot(true);
    stopTimer.setInterval(60000);
    stopTimer.start();

    qDebug() << "Finished initialization of sandbox process" << qApp->applicationPid();
  }
}

SSandboxServer::ResponseMessage Sandbox::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *)
{
  const MediaServer::File file(request);

  if (request.isGet() && (file.url().path() == "/"))
  {
    if (file.url().hasQueryItem("formats"))
    {
      QDomDocument doc("");
      QDomElement rootElm = doc.createElement("formats");
      doc.appendChild(rootElm);

      struct T
      {
        static QDomElement createElement(QDomDocument &doc, const QString &name, const QString &type, const QStringList &values)
        {
          QDomElement codecsElm = doc.createElement(name);
          foreach (const QString &value, values)
          {
            QDomElement elm = doc.createElement(type);
            elm.appendChild(doc.createTextNode(value));
            codecsElm.appendChild(elm);
          }

          return codecsElm;
        }
      };

      rootElm.appendChild(T::createElement(doc, "audiocodecs", "codec", SAudioEncoderNode::codecs()));
      rootElm.appendChild(T::createElement(doc, "videocodecs", "codec", SVideoEncoderNode::codecs()));
      rootElm.appendChild(T::createElement(doc, "formats", "format", SIOOutputNode::formats()));

      return SSandboxServer::ResponseMessage(
          request, SSandboxServer::Status_Ok,
          doc.toByteArray(-1), SHttpEngine::mimeTextXml);
    }
    else if (file.url().hasQueryItem("modules"))
    {
      QDomDocument doc("");
      QDomElement rootElm = doc.createElement("modules");
      doc.appendChild(rootElm);

      const QMap<QString, SModule *> modules = sApp->modules();
      for (QMap<QString, SModule *>::ConstIterator i=modules.begin(); i!=modules.end(); i++)
      {
        QDomElement moduleElm = doc.createElement("module");
        moduleElm.setAttribute("file", i.key());

        QDomElement aboutElm = doc.createElement("about");
        aboutElm.appendChild(doc.createTextNode((*i)->about()));
        moduleElm.appendChild(aboutElm);

        QDomElement licensesElm = doc.createElement("licenses");
        licensesElm.appendChild(doc.createTextNode((*i)->licenses()));
        moduleElm.appendChild(licensesElm);

        rootElm.appendChild(moduleElm);
      }

      return SSandboxServer::ResponseMessage(
          request, SSandboxServer::Status_Ok,
          doc.toByteArray(-1), SHttpEngine::mimeTextXml);
    }
    else if (file.url().hasQueryItem("exit"))
    {
      QTimer::singleShot(5000, this, SLOT(deleteLater()));

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok);
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}
