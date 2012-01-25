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

#include "frontend.h"

#if defined(Q_OS_LINUX)
const char Frontend::daemonName[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Frontend::daemonName[] = "LXiMediaCenter Backend";
#elif defined(Q_OS_MACX)
const char Frontend::daemonName[] = "net.sf.lximedia.lximediacenter.backend";
#endif

Frontend::Frontend()
  : QWebView(),
    ssdpClient(QString("uuid:" + QUuid::createUuid().toString()).replace("{", "").replace("}", "")),
    frontendPageShowing(false),
    waitingForWelcome(qApp->arguments().contains("--welcome"))
{
#if defined(Q_OS_MACX)
  if (!QFile::exists(QDir::homePath() + "/Library/LaunchAgents/" + daemonName + ".plist"))
  if (QProcess::startDetached(QDir(qApp->applicationDirPath()).absoluteFilePath("lximcbackend"), QStringList("--run")))
  {
    registerAgent();
    startingTimer.start();
    waitingForWelcome = true;
  }
#endif

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
  else
    QWebView::keyPressEvent(e);
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
  else if (url.path() == "/waiting")
  {
    setContent(makeWaitingPage(), "text/html", QUrl("qrc:/"));
  }
  else
  {
    if (url.path() == "/startbackend")
    {
#if !defined(Q_OS_MACX)
      SDaemon::start(daemonName);
#else
      if (QProcess::startDetached(QDir(qApp->applicationDirPath()).absoluteFilePath("lximcbackend"), QStringList("--run")))
        startingTimer.start();
#endif
    }
    else if ((url.path() == "/stopbackend") || (url.path() == "/disablebackend"))
    {
#if !defined(Q_OS_MACX)
      SDaemon::stop(daemonName);
#else
      startingTimer = QTime();

      foreach (const Server &server, servers)
      if (isLocalAddress(server.presentationURL.host()))
      {
        QUrl url = server.presentationURL;
        url.setPath(url.path() + "exit");

        networkAccessManager.get(QNetworkRequest(url));
      }

      if (url.path() == "/disablebackend")
      {
        QDir dir(QDir::homePath() + "/Library/LaunchAgents/");
        if (dir.exists())
          dir.remove(QString(daemonName) + ".plist");
      }
#endif
    }

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

          if (waitingForWelcome &&
              (server->modelName == qApp->applicationName()) &&
              isLocalAddress(server->presentationURL.host()))
          {
            waitingForWelcome = false;
            setUrl(server->presentationURL);
          }
          else
            frontendPageTimer.start(250);
        }
      }
    }
  }

  reply->deleteLater();
}

void Frontend::updateFrontendPage(void)
{
  if (waitingForWelcome)
    loadFrontendPage(QUrl("frontend:/waiting"));
  else if (frontendPageShowing)
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

#if defined(Q_OS_MACX)
void Frontend::registerAgent(void)
{
  QDir dir(QDir::homePath() + "/Library/LaunchAgents/");

  if (!dir.exists())
    QDir::home().mkpath("Library/LaunchAgents");

  if (dir.exists())
  {
    QDomDocument doc;
    QDomElement root = doc.createElement("plist");
    root.setAttribute("version", "1.0");
    QDomElement dict = doc.createElement("dict");

    QDomElement elm;
    elm = doc.createElement("key");
    elm.appendChild(doc.createTextNode("Label"));
    dict.appendChild(elm);
    elm = doc.createElement("string");
    elm.appendChild(doc.createTextNode(QString(daemonName) + ".agent"));
    dict.appendChild(elm);

    elm = doc.createElement("key");
    elm.appendChild(doc.createTextNode("RunAtLoad"));
    dict.appendChild(elm);
    dict.appendChild(doc.createElement("true"));

    elm = doc.createElement("key");
    elm.appendChild(doc.createTextNode("Program"));
    dict.appendChild(elm);
    elm = doc.createElement("string");
    elm.appendChild(doc.createTextNode(QDir(qApp->applicationDirPath()).absoluteFilePath("lximcbackend")));
    dict.appendChild(elm);

    root.appendChild(dict);
    doc.appendChild(root);

    QFile file(dir.absoluteFilePath(QString(daemonName) + ".plist"));
    if (file.open(QFile::WriteOnly))
      file.write(doc.toByteArray());
  }
}
#endif


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
