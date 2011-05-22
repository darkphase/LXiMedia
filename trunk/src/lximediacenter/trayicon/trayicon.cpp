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

#include "trayicon.h"
#if defined(QT_WEBKIT_LIB)
#include <QtWebKit>
#endif
#include <QtXml>
#include <liblximediacenter/globalsettings.h>
#if defined(Q_OS_WIN)
#include <windows.h>
#endif

using namespace LXiMediaCenter;

#if defined(Q_OS_WIN)
const int   TrayIcon::iconSize = 16;
#elif defined(Q_OS_UNIX)
const int   TrayIcon::iconSize = 32;
#endif


TrayIcon::TrayIcon()
    : QObject(),
      icon(":/lximedia.png"),
      trayIcon(),
      ssdpClient(QString("uuid:" + GlobalSettings::serverUuid().toString()).replace("{", "").replace("}", "")),
      aboutBox(NULL)
{
  connect(&updateStatusTimer, SIGNAL(timeout()), SLOT(updateStatus()));
  connect(&networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(requestFinished(QNetworkReply *)));
  connect(&trayIcon, SIGNAL(messageClicked()), SLOT(messageClicked()));
  connect(&menu, SIGNAL(triggered(QAction *)), SLOT(loadBrowser(QAction *)));
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::show(void)
{
  trayIcon.setIcon(icon.pixmap(iconSize, QIcon::Normal));
  trayIcon.setContextMenu(&menu);
  trayIcon.setToolTip(QCoreApplication::applicationName());
  trayIcon.show();

  rebuildMenu();

  QTimer::singleShot(5000, this, SLOT(startSSDP()));
}

void TrayIcon::startSSDP(void)
{
  ssdpClient.initialize(GlobalSettings::defaultBackendInterfaces());
  ssdpClient.sendSearch(qApp->applicationName() + QString(":server"));

  connect(&ssdpClient, SIGNAL(searchUpdated()), SLOT(updateMenu()));

  updateStatusTimer.start(60000);
}

void TrayIcon::showAbout(void)
{
  if (aboutBox == NULL)
  {
    aboutBox = new QMessageBox();
    aboutBox->setWindowTitle(tr("About") + " " + QByteArray(sApp->name()));
    aboutBox->setIconPixmap(icon.pixmap(64, QIcon::Normal));
    aboutBox->setText(sApp->about());

    aboutBox->setStandardButtons(QMessageBox::Close);
    aboutBox->setWindowModality(Qt::NonModal);
  }

  aboutBox->show();
}

void TrayIcon::updateMenu(void)
{
  QSet<QString> uuids;
  foreach (const SSsdpClient::Node &result, ssdpClient.searchResults(qApp->applicationName() + QString(":server")))
  {
    QMap<QString, Server>::Iterator i = servers.find(result.uuid);
    if (i == servers.end())
    {
      Server server;
      server.url = result.location;
      server.hostname = server.url.host();

      i = servers.insert(result.uuid, server);
    }
    else
    {
      i->url = result.location;
      i->hostname = i->url.host();
    }

    uuids += result.uuid;
    updateStatus(i);
  }

  // Remove old servers
  for (QMap<QString, Server>::Iterator i=servers.begin(); i!=servers.end(); )
  if (!uuids.contains(i.key()))
    i = servers.erase(i);
  else
    i++;

  rebuildMenu();
}

void TrayIcon::rebuildMenu(void)
{
  menu.clear();
  menu.addAction(tr("About") + " " + QByteArray(sApp->name()), this, SLOT(showAbout()));
  menu.addSeparator();

  bool found = false;
  foreach (const Server &server, servers)
  if (server.visible)
  {
    menu.addAction(icon, server.hostname)->setData(server.url.toString());
    found = true;
  }

  if (!found)
  {
    trayIcon.setIcon(icon.pixmap(iconSize, QIcon::Disabled));
    menu.addAction(tr("Looking for servers") + " ...")->setEnabled(false);
  }
  else
    trayIcon.setIcon(icon.pixmap(iconSize, QIcon::Normal));

  menu.addSeparator();
  menu.addAction(tr("Exit"), qApp, SLOT(quit()));
}

void TrayIcon::updateStatus(void)
{
  for (QMap<QString, Server>::Iterator i=servers.begin(); i!=servers.end(); i++)
    updateStatus(i);
}

void TrayIcon::updateStatus(QMap<QString, Server>::Iterator &i)
{
  QUrl url = i->url;
  url.setPath("/traystatus.xml");

  if (i->updateStatusReply)
    i->updateStatusReply->abort();

  QNetworkRequest request(url);
  i->updateStatusReply = networkAccessManager.get(request);
}

void TrayIcon::requestFinished(QNetworkReply *reply)
{
  for (QMap<QString, Server>::Iterator i=servers.begin(); i!=servers.end(); i++)
  if (i->updateStatusReply == reply)
  {
    if ((reply->error() == QNetworkReply::NoError) ||
        (reply->error() == QNetworkReply::RemoteHostClosedError))
    {
      const QByteArray data = i->updateStatusReply->readAll();

      QDomDocument doc("");
      if (doc.setContent(data))
      {
        QDomElement root = doc.documentElement();

        QDomElement hostInfo = root.firstChildElement("hostinfo");
        if (!hostInfo.isNull())
          i->hostname = hostInfo.attribute("hostname", i->url.host());

        for (QDomElement j=root.firstChildElement("dlnaclient"); !j.isNull(); j=j.nextSiblingElement("dlnaclient"))
        if (!i->detectedClients.contains(j.attribute("name")))
        {
          i->detectedClients += j.attribute("name");

          if (!i->firstUpdate)
          {
            messageUrl = i->url;
            messageUrl.setPath("/setup/");

            trayIcon.showMessage(
                tr("Detected a new media playback device"),
                j.attribute("useragent"),
                QSystemTrayIcon::Information);
          }
        }

        if (!root.firstChildElement("errorlogfile").isNull() && !i->notifiedErrorLog)
        {
          i->notifiedErrorLog = true;
          messageUrl = i->url;

          trayIcon.showMessage(
              tr("Error reports ready for submission"),
              tr("The last time the program was run some unexpected errors "
                 "occurred. Detailed information on these errors have been "
                 "written to one or more error reports, click here for more info."),
              QSystemTrayIcon::Warning);
        }

        i->firstUpdate = false;
      }

      i->visible = true;
    }
    else if (reply->error() != QNetworkReply::OperationCanceledError)
    { // Failed, hide item
      i->visible = false;
    }

    rebuildMenu();
    i->updateStatusReply = NULL;

    break;
  }

  reply->deleteLater();
}

void TrayIcon::messageClicked(void)
{
  loadBrowser(messageUrl);
}

void TrayIcon::loadBrowser(QAction *action)
{
  const QString url = action->data().toString();
  if (!url.isEmpty())
    loadBrowser(url);
}

void TrayIcon::loadBrowser(const QUrl &url)
{
#if defined(QT_WEBKIT_LIB)
  QWebView *view = new QWebView();
  view->setAttribute(Qt::WA_DeleteOnClose);
  view->load(url);
  view->show();
#elif defined(Q_OS_UNIX)
  QProcess::startDetached("xdg-open", QStringList() << url.toString());
#elif defined(Q_OS_WIN)
  if (int(::ShellExecute(NULL, L"open", reinterpret_cast<const WCHAR *>(url.toString().utf16()), NULL, NULL, SW_SHOWNORMAL)) <= 32)
  {
    QProcess::startDetached(
        QProcessEnvironment::systemEnvironment().value("ComSpec", "cmd.exe"),
        QStringList() << "/c" << "start" << url.toString());
  }
#endif
}
