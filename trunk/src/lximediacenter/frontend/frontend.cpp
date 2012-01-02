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

#include "frontend.h"

#if defined(Q_OS_LINUX)
const char Frontend::backendName[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Frontend::backendName[] = "LXiMediaCenter Backend";
#endif

Frontend::Frontend()
  : QWebView(),
    ssdpClient(QString("uuid:" + QUuid::createUuid().toString()).replace("{", "").replace("}", "")),
    frontendPageShowing(false)
{
  WebPage * const webPage = new WebPage(this);
  webPage->setNetworkAccessManager(new NetworkAccessManager(this));
  setPage(webPage);

  connect(&ssdpClient, SIGNAL(searchUpdated()), SLOT(updateServers()));
  ssdpClient.initialize(SSsdpClient::localInterfaces());
  ssdpClient.sendSearch("urn:schemas-upnp-org:device:MediaServer:1");

  connect(&networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(requestFinished(QNetworkReply *)));

  connect(&frontendPageTimer, SIGNAL(timeout()), SLOT(updateFrontendPage()));
  frontendPageTimer.setSingleShot(true);

  connect(this, SIGNAL(titleChanged(QString)), SLOT(titleChanged(QString)));
}

Frontend::~Frontend()
{
}

void Frontend::show(void)
{
  setWindowIcon(QIcon(":/lximedia.png"));

  frontendPageShowing = true;
  updateFrontendPage();

  QWebView::show();
}

void Frontend::contextMenuEvent(QContextMenuEvent *)
{
  // Disable context menu
}

void Frontend::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape)
  {
    if (!isFullScreen())
    {
      frontendPageShowing = true;
      updateFrontendPage();
    }
    else
      showNormal();
  }
  else if (e->key() == Qt::Key_F11)
  {
    if (isFullScreen())
      showNormal();
    else
      showFullScreen();
  }

  // Disable keyboard navigation
}

void Frontend::keyReleaseEvent(QKeyEvent *)
{
  // Disable keyboard navigation
}

void Frontend::loadFrontendPage(const QUrl &url)
{
  frontendPageShowing = true;

  qApp->setOverrideCursor(Qt::WaitCursor);

  if (url.path().startsWith("/iframe/"))
  {
    frontendPageShowing = false;
    setContent(makeIFrame(QByteArray::fromHex(url.path().mid(8).toAscii())), "text/html", QUrl("qrc:/"));
  }
  else
  {
    if (url.path() == "/startbackend")
      SDaemon::start(backendName);
    else if (url.path() == "/stopbackend")
      SDaemon::stop(backendName);

    setContent(makeFrontendPage(), "text/html", QUrl("qrc:/"));
  }

  qApp->restoreOverrideCursor();
}

void Frontend::updateServers(void)
{
  QSet<QString> uuids;
  foreach (const SSsdpClient::Node &result, ssdpClient.searchResults("urn:schemas-upnp-org:device:MediaServer:1"))
  {
    servers.insert(result.uuid, result);
    uuids += result.uuid;
    networkAccessManager.get(QNetworkRequest(result.location));
  }

  // Remove old servers
  for (QMap<QString, Server>::Iterator i=servers.begin(); i!=servers.end(); )
  if (!uuids.contains(i.key()))
    i = servers.erase(i);
  else
    i++;

  frontendPageTimer.start(250);
}

void Frontend::requestFinished(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError)
  {
    QDomDocument doc;
    if (doc.setContent(reply))
    {
      QDomElement deviceElm = doc.documentElement().firstChildElement("device");
      if (!deviceElm.isNull())
      {
        QMap<QString, Server>::Iterator server =
            servers.find(deviceElm.firstChildElement("UDN").text());

        if (server != servers.end())
        {
          server->friendlyName = deviceElm.firstChildElement("friendlyName").text();
          server->modelName = deviceElm.firstChildElement("modelName").text();

          const QUrl presentationURL = deviceElm.firstChildElement("presentationURL").text();
          server->presentationURL = reply->request().url();
          server->presentationURL.setPath(presentationURL.path());
          server->presentationURL.setQueryItems(presentationURL.queryItems());

          QDomElement iconListElm = deviceElm.firstChildElement("iconList");
          if (!iconListElm.isNull())
          {
            QDomElement urlElm = iconListElm.firstChildElement("icon").firstChildElement("url");
            if (!urlElm.isNull())
            {
              const QUrl iconURL = urlElm.text();
              server->iconURL = reply->request().url();
              server->iconURL.setPath(iconURL.path());
              server->iconURL.setQueryItems(iconURL.queryItems());
            }
          }

          frontendPageTimer.start(250);
        }
      }
    }
  }

  reply->deleteLater();
}

void Frontend::updateFrontendPage(void)
{
  if (frontendPageShowing)
    loadFrontendPage(QUrl("frontend:/"));
}

void Frontend::titleChanged(const QString &title)
{
  setWindowTitle(title);
}

bool Frontend::isLocalAddress(const QString &host)
{
  const QHostAddress address(host);

  foreach (const QHostAddress &local, QNetworkInterface::allAddresses())
  if (local == address)
    return true;

  return false;
}

Frontend::WebPage::WebPage(Frontend *parent)
  : QWebPage(parent),
    parent(parent)
{
}

bool Frontend::WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType)
{
  if (frame == mainFrame())
  {
    if (request.url().scheme() == "frontend")
    {
      parent->loadFrontendPage(request.url());
      return false;
    }
    else if (request.url().scheme() != "qrc")
      parent->frontendPageShowing = false;
  }

  return true;
}

Frontend::NetworkAccessManager::NetworkAccessManager(Frontend *parent)
  : QNetworkAccessManager(parent)
{
}

QNetworkReply * Frontend::NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
  if (op == GetOperation)
  {
    QNetworkRequest req = request;
    req.setRawHeader(qApp->applicationName().toAscii() + ".HomeAddress", "frontend:/");

    return QNetworkAccessManager::createRequest(op, req, outgoingData);
  }
  else
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
