/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "screengrabber.h"

ScreenGrabber::ScreenGrabber()
  : QObject(),
    screenIcon(":/img/video-display.png"),
    eyesIcon(":/img/eyes.png"),
    menu(),
    trayIcon(screenIcon, this),
    sandboxServer(this)
{
  menu.addAction(tr("Quit"), qApp, SLOT(quit()));
  trayIcon.setContextMenu(&menu);
}

ScreenGrabber::~ScreenGrabber()
{
}

void ScreenGrabber::show()
{
  sandboxServer.registerCallback("/", this);
  sandboxServer.initialize("", "lximc-screengrabber");

  trayIcon.show();
  trayIcon.setToolTip(tr("LXiMediaCenter screen grabber"));

  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

SSandboxServer::ResponseMessage ScreenGrabber::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const QUrl url(request.path());

  if (request.method() == "GET")
  {
    if (url.hasQueryItem("listdesktops"))
    {
      QByteArray content;
      {
        QXmlStreamWriter writer(&content);
        writer.setAutoFormatting(false);
        writer.writeStartElement("desktops");

        foreach (const QString &camera, SAudioVideoInputNode::devices())
        if (camera.startsWith("Desktop ", Qt::CaseInsensitive))
        {
          writer.writeStartElement("desktop");
          writer.writeCharacters(camera);
          writer.writeEndElement();
        }

        writer.writeEndElement();
      }

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok, content, SHttpEngine::mimeTextXml);
    }
    else if (url.hasQueryItem("opendesktop"))
    {
      const QString device = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("desktop").toAscii()));
      if (!device.isEmpty() && device.startsWith("Desktop ", Qt::CaseInsensitive))
      {
        DesktopStream * const desktopStream = new DesktopStream(device);
        if (desktopStream->setup(request, socket))
        if (desktopStream->start())
        {
          trayIcon.showMessage(
                tr("LXiMediaCenter screen grabber"),
                tr("Grabber stream started, your desktop is being watched."));

          if (streams.isEmpty())
            trayIcon.setIcon(eyesIcon);

          streams.append(desktopStream);
          return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
        }

        delete desktopStream;
      }
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void ScreenGrabber::cleanStreams(void)
{
  const bool wasEmpty = streams.isEmpty();

  for (QList<SGraph *>::Iterator i=streams.begin(); i!=streams.end(); )
  if (!(*i)->isRunning())
  {
    delete *i;
    i = streams.erase(i);
  }
  else
    i++;

  if (!wasEmpty && streams.isEmpty())
    trayIcon.setIcon(screenIcon);
}


DesktopStream::DesktopStream(const QString &device)
  : MediaStream(),
    input(this, device)
{
}

DesktopStream::~DesktopStream()
{
}

bool DesktopStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (MediaStream::setup(
        request, socket, STime::null, STime(),
        input.audioFormat(), input.videoFormat()))
  {
    connect(&input, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
    connect(&input, SIGNAL(output(SVideoBuffer)), &video->letterboxDetectNode, SLOT(input(SVideoBuffer)));

    return true;
  }

  return false;
}
