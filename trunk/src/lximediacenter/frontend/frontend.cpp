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
#include <LXiCore>
#include <upnp/upnp.h>

#if defined(Q_OS_LINUX)
const char Frontend::daemonName[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Frontend::daemonName[] = "LXiMediaCenter Backend";
#elif defined(Q_OS_MACX)
const char Frontend::daemonName[] = "net.sf.lximedia.lximediacenter.backend";
#endif

const QEvent::Type Frontend::DiscoveryEvent::myType = QEvent::Type(QEvent::registerEventType());


Frontend::Frontend()
  : QWebView(),
    initialized(false),
    clientHandle(0),
    checkNetworkInterfacesTimer(this),
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

  int result = ::UpnpInit(NULL, 0);
  initialized = result == UPNP_E_SUCCESS;
  if (initialized)
  {
    struct T
    {
      static int callback(Upnp_EventType eventType, void *event, void *cookie)
      {
        Upnp_Discovery * const discovery = reinterpret_cast<Upnp_Discovery *>(event);
        Frontend * const me = reinterpret_cast<Frontend *>(cookie);
        if (eventType == UPNP_DISCOVERY_ADVERTISEMENT_ALIVE)
          qApp->postEvent(me, new DiscoveryEvent(DiscoveryEvent::Alive, discovery->DeviceId, discovery->Location));
        else if (eventType == UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE)
          qApp->postEvent(me, new DiscoveryEvent(DiscoveryEvent::ByeBye, discovery->DeviceId, discovery->Location));
        else if (eventType == UPNP_DISCOVERY_SEARCH_RESULT)
          qApp->postEvent(me, new DiscoveryEvent(DiscoveryEvent::Found, discovery->DeviceId, discovery->Location));
      }
    };

    if (::UpnpRegisterClient(&T::callback, this, &clientHandle) == UPNP_E_SUCCESS)
      ::UpnpSearchAsync(clientHandle, 10, "urn:schemas-upnp-org:device:MediaServer:1", this);
  }

  connect(&networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(requestFinished(QNetworkReply *)));

  connect(&frontendPageTimer, SIGNAL(timeout()), SLOT(updateFrontendPage()));
  frontendPageTimer.setSingleShot(true);

  connect(this, SIGNAL(titleChanged(QString)), SLOT(titleChanged(QString)));

  checkNetworkInterfaces();
  connect(&checkNetworkInterfacesTimer, SIGNAL(timeout()), SLOT(checkNetworkInterfaces()));
}

Frontend::~Frontend()
{
  if (initialized)
  {
    ::UpnpUnRegisterClient(clientHandle);
    ::UpnpFinish();
  }
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

void Frontend::customEvent(QEvent *e)
{
  if (e->type() == DiscoveryEvent::myType)
  {
    DiscoveryEvent * const event = static_cast<DiscoveryEvent *>(e);
    switch (event->kind)
    {
    case DiscoveryEvent::Alive:
    case DiscoveryEvent::Found:
      networkAccessManager.get(QNetworkRequest(QUrl::fromEncoded(event->location)));
      break;

    case DiscoveryEvent::ByeBye:
      {
        QMap<QByteArray, Server>::Iterator server = servers.find(event->deviceId);
        if (server != servers.end())
        {
          servers.erase(server);
          frontendPageTimer.start(250);
        }
      }
      break;
    }
  }
  else
    QWebView::customEvent(e);
}

void Frontend::loadFrontendPage(const QUrl &url)
{
  frontendPageShowing = true;

  qApp->setOverrideCursor(Qt::WaitCursor);

  if (url.path().startsWith("/iframe/"))
  {
    frontendPageShowing = false;
    setContent(makeIFrame(QByteArray::fromHex(url.path().mid(8).toLatin1())), "text/html", QUrl("qrc:/"));
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

static IXML_Node * getElement(_IXML_Node *from, const char *name)
{
  IXML_Node *result = NULL;

  IXML_NodeList * const children = ixmlNode_getChildNodes(from);
  for (IXML_NodeList *i = children; i && !result; i = i->next)
  {
    const char *n = strchr(i->nodeItem->nodeName, ':');
    if (n == NULL)
      n = i->nodeItem->nodeName;
    else
      n++;

    if (strcmp(n, name) == 0)
      result = i->nodeItem;
  }

  ixmlNodeList_free(children);

  return result;
}

static QByteArray getText(_IXML_Node *from)
{
  QByteArray result;

  if (from)
  {
    IXML_NodeList * const children = ixmlNode_getChildNodes(from);
    for (IXML_NodeList *i = children; i; i = i->next)
    if (i->nodeItem->nodeValue)
      result += i->nodeItem->nodeValue;

    ixmlNodeList_free(children);
  }

  return result;
}

void Frontend::requestFinished(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError)
  {
    const QByteArray data = reply->readAll();
    qDebug() << data;
    IXML_Document * const doc = ixmlParseBuffer(data);
    if (doc)
    {
      IXML_Node * const rootElm = getElement(&doc->n, "root");
      if (rootElm)
      {
        IXML_Node * const deviceElm = getElement(rootElm, "device");
        if (deviceElm)
        {
          const QByteArray udn = getText(getElement(deviceElm, "UDN"));
          QMap<QByteArray, Server>::Iterator server = servers.find(udn);
          if (server == servers.end())
            server = servers.insert(udn, Server());

          server->friendlyName = getText(getElement(deviceElm, "friendlyName"));
          server->modelName = getText(getElement(deviceElm, "modelName"));

          const QUrl presentationURL = QUrl::fromEncoded(getText(getElement(deviceElm, "presentationURL")));
          server->presentationURL = reply->request().url();
          server->presentationURL.setPath(presentationURL.path());
          server->presentationURL.setQuery(presentationURL.query());

          IXML_Node * const iconListElm = getElement(deviceElm, "iconList");
          if (iconListElm)
          {
            IXML_Node * const iconElm = getElement(iconListElm, "icon");
            if (iconElm)
            {
              const QUrl iconURL = QUrl::fromEncoded(getText(getElement(iconElm, "url")));
              server->iconURL = reply->request().url();
              server->iconURL.setPath(iconURL.path());
              server->iconURL.setQuery(iconURL.query());
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

          ixmlDocument_free(doc);
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

void Frontend::checkNetworkInterfaces(void)
{
//  QSettings settings;

//  const QList<QHostAddress> interfaces =
//      settings.value("BindAllNetworks", false).toBool()
//          ? QNetworkInterface::allAddresses()
//          : SSsdpClient::localAddresses();

//  // Bind new interfaces
//  bool boundNew = false;
//  foreach (const QHostAddress &interface, interfaces)
//  {
//    bool found = false;
//    foreach (const QHostAddress &bound, boundNetworkInterfaces)
//    if (bound == interface)
//    {
//      found = true;
//      break;
//    }

//    if (!found)
//    {
//      ssdpClient.bind(interface);

//      boundNetworkInterfaces += interface;
//      boundNew = true;
//    }
//  }

//  // Release old interfaces
//  for (QList<QHostAddress>::Iterator bound = boundNetworkInterfaces.begin();
//       bound != boundNetworkInterfaces.end();
//       )
//  {
//    bool found = false;
//    foreach (const QHostAddress &interface, interfaces)
//    if (interface == *bound)
//    {
//      found = true;
//      break;
//    }

//    if (!found)
//    {
//      ssdpClient.release(*bound);

//      bound = boundNetworkInterfaces.erase(bound);
//    }
//    else
//      bound++;
//  }

//  if (boundNew)
//    ssdpClient.sendSearch("urn:schemas-upnp-org:device:MediaServer:1");
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
    req.setRawHeader(qApp->applicationName().toLatin1() + ".HomeAddress", "frontend:/");

    return QNetworkAccessManager::createRequest(op, req, outgoingData);
  }
  else
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
