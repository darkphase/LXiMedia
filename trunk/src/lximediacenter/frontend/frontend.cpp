/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
    upnp(this),
    upnpClient(&upnp),
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

  if (upnp.initialize(0))
  {
    connect(&upnpClient, SIGNAL(deviceDiscovered(QByteArray,QByteArray)), SLOT(deviceDiscovered(QByteArray,QByteArray)));
    connect(&upnpClient, SIGNAL(deviceClosed(QByteArray)), SLOT(deviceClosed(QByteArray)));

    upnpClient.startSearch("urn:schemas-upnp-org:device:MediaServer:1");
  }

  connect(&frontendPageTimer, SIGNAL(timeout()), SLOT(updateFrontendPage()));
  frontendPageTimer.setSingleShot(true);

  connect(this, SIGNAL(titleChanged(QString)), SLOT(titleChanged(QString)));
}

Frontend::~Frontend()
{
  upnp.close();
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

void Frontend::deviceDiscovered(const QByteArray &, const QByteArray &location)
{
  Client::DeviceDescription description;
  if (upnpClient.getDeviceDescription(location, description))
  {
    QMap<QByteArray, Client::DeviceDescription>::Iterator device = devices.find(description.udn);
    if (device == devices.end())
      device = devices.insert(description.udn, description);
    else
      *device = description;

    if (waitingForWelcome &&
        (description.modelName == qApp->applicationName()) &&
        upnp.isMyAddress(description.presentationURL.host().toLatin1()))
    {
      waitingForWelcome = false;
      setUrl(device->presentationURL);
    }
    else
      frontendPageTimer.start(250);
  }
}

void Frontend::deviceClosed(const QByteArray &deviceId)
{
  QMap<QByteArray, Client::DeviceDescription>::Iterator device = devices.find(deviceId);
  if (device != devices.end())
  {
    devices.erase(device);
    frontendPageTimer.start(250);
  }
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
