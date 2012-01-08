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

const char Frontend::htmlIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{_PRODUCT}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"icon\" type=\"image/png\" href=\"/lximedia.png\" />"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/frontend.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    "{NAVIGATOR}"
    "{CONTENT}"
    "</body>\n"
    "</html>\n";

const char Frontend::htmlWaiting[] =
    " <div style=\"font-size:3em;text-align:center;padding-top:4em;\">\n"
    "  {TR_INITIALIZING_PLEASE_WAIT}\n"
    " </div>\n";

const char Frontend::htmlNavigator[] =
    " <div class=\"main_navigator\" id=\"navigator\">\n"
    "  <div class=\"root\">{_PRODUCT}</div>\n"
    "  <div><a href=\"frontend:/\"><img src=\"/lximedia.png\" alt=\"..\" /></a></div>\n"
    " </div>\n";

const char Frontend::htmlServers[] =
    " <p>{TR_FRONTEND_EXPLAIN}</p>\n"
    " <p class=\"title\">{TR_LXIMEDIACENTER_SERVER_ON_THIS_COMPUTER}</p>\n"
    "{LOCAL_SERVER}"
    " <p class=\"title\">{TR_LXIMEDIACENTER_SERVERS}</p>\n"
    " <div class=\"servers\">\n"
    " <p>{TR_LXIMEDIACENTER_SERVERS_EXPLAIN}</p>\n"
    "{LXIMEDIACENTER_SERVERS}"
    " </div>\n"
    " <p class=\"title\">{TR_OTHER_SERVERS}</p>\n"
    " <p>{TR_OTHER_SERVERS_EXPLAIN}</p>\n"
    " <div class=\"servers\">\n"
    "{OTHER_SERVERS}"
    " </div>\n";

const char Frontend::htmlLocalServer[] =
    " <p>{LOCAL_SERVER_STATUS}</p>\n";

const char Frontend::htmlNoLocalServer[] =
    " <p class=\"idle\">{TR_NOT_INSTALLED}</p>\n";

const char Frontend::htmlStartLocalServer[] =
#if defined(Q_OS_LINUX)
    " <span class=\"idle\">{TR_START_LINUX}</span>";
#else
    " <a href=\"frontend:/startbackend\">{TR_START}</a>";
#endif

const char Frontend::htmlStopLocalServer[] =
#if defined(Q_OS_LINUX)
    " <span class=\"idle\">{TR_STOP_LINUX}</span>";
#else
    " <a href=\"frontend:/stopbackend\">{TR_STOP}</a>";
#endif

const char Frontend::htmlDisableLocalServer[] =
#if !defined(Q_OS_MACX)
    "";
#else
    " <a href=\"frontend:/disablebackend\">{TR_DISABLE}</a>";
#endif

const char Frontend::htmlConfigureLocalServer[] =
    " <a href=\"{ITEM_LOCATION}settings\">{TR_CONFIGURE}</a>";

const char Frontend::htmlServer[] =
    " <a class=\"hidden\" href=\"{ITEM_LOCATION}\">"
    "  <div class=\"server\">\n"
    "   <span><img src=\"{ITEM_ICON}\" /></span>\n"
    "   <span class=\"title\">\n"
    "    <p class=\"name\">{ITEM_NAME}</p>\n"
    "    <p>{TR_MODEL}: {ITEM_MODEL}</p>\n"
    "    <p>{TR_ADDRESS}: http://{ITEM_HOST}/ {THIS_COMPUTER}</p>\n"
    "   </span>\n"
    "  </div>\n"
    " </a>\n";

const char Frontend::htmlNoServers[] =
    " <p class=\"idle\">{TR_NO_SERVERS}</p>\n";

const char Frontend::htmlIFrame[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{_PRODUCT}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"icon\" type=\"image/png\" href=\"/lximedia.png\" />"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/frontend.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/frontend.js\"></script>\n" // Open and close tag due to IE bug
    "</head>\n"
    "<body class=\"browser\" id=\"body\" onresize=\"resizeWindow()\">\n"
    "{NAVIGATOR}"
    " <iframe class=\"browser\" id=\"browser\" src=\"{PATH}\">\n"
    " </iframe>\n"
    " <script type=\"text/javascript\">resizeWindow();</script>\n"
    "</body>\n"
    "</html>\n";

QByteArray Frontend::makeWaitingPage(void) const
{
  SStringParser htmlParser;
  htmlParser.setField("NAVIGATOR", "");

  htmlParser.setField("TR_INITIALIZING_PLEASE_WAIT", tr("Initializing, please wait ..."));

  htmlParser.setField("CONTENT", htmlParser.parse(htmlWaiting));
  return htmlParser.parse(htmlIndex);
}

QByteArray Frontend::makeFrontendPage(void) const
{
  SStringParser htmlParser;
  htmlParser.setField("NAVIGATOR", htmlParser.parse(htmlNavigator));

  htmlParser.setField("TR_ADDRESS", tr("Address"));
  htmlParser.setField("TR_CONFIGURE", tr("Configure"));
  htmlParser.setField("TR_DISABLE", tr("Disable"));
  htmlParser.setField("TR_LXIMEDIACENTER_SERVER_ON_THIS_COMPUTER", qApp->applicationName() + ' ' + tr("server on this computer"));
  htmlParser.setField("TR_LXIMEDIACENTER_SERVERS", qApp->applicationName() + ' ' + tr("servers"));
  htmlParser.setField("TR_MODEL", tr("Model"));
  htmlParser.setField("TR_NO_SERVERS", tr("No servers"));
  htmlParser.setField("TR_NOT_INSTALLED", tr("Not installed"));
  htmlParser.setField("TR_OTHER_SERVERS", tr("Other servers"));
  htmlParser.setField("TR_START", tr("Start"));
  htmlParser.setField("TR_STOP", tr("Stop"));

#ifdef Q_OS_LINUX
  htmlParser.setField("TR_START_LINUX",
      tr("To start the server type \"sudo /etc/init.d/lximediacenter start\" "
         "in a terminal."));
  htmlParser.setField("TR_STOP_LINUX",
      tr("To stop the server type \"sudo /etc/init.d/lximediacenter stop\" "
         "in a terminal."));
#endif

  htmlParser.setField("TR_FRONTEND_EXPLAIN",
      tr("This application can be used to configure all media servers in the "
         "local network. It will scan the local network for servers and lists "
         "them below, click on one of the servers to manage it. It is also "
         "possible to manage the server by typing the address displayed in the "
         "box in the address bar of a web browser on a computer, tablet, or "
         "smart-phone connected to the local network."));

  htmlParser.setField("TR_LXIMEDIACENTER_SERVERS_EXPLAIN",
      tr("Below are all LXiMediaCenter servers found in the local network, "
         "including this computer."));

  htmlParser.setField("TR_OTHER_SERVERS_EXPLAIN",
      tr("Below are all other servers found in the local network, including "
         "this computer. These servers are from other manufacturers and may "
         "not all provide a web-based configuration utility. Please refer to "
         "the manual of the respective manufacturer for more information."));

  int localConfigAdded = 0;
  QMultiMap<QString, QByteArray> lximediaServers, otherServers;
  foreach (const Server &server, servers)
  {
    htmlParser.setField("ITEM_NAME", server.friendlyName);
    htmlParser.setField("ITEM_MODEL", server.modelName);
    htmlParser.setField("ITEM_ICON", server.iconURL.isEmpty() ? QUrl(":/img/null.png") : server.iconURL);
    htmlParser.appendField("THIS_COMPUTER", "");

    QString host = server.presentationURL.host();

    if (server.modelName == qApp->applicationName())
    {
      htmlParser.setField("ITEM_LOCATION", server.presentationURL);
      if (isLocalAddress(host) && (localConfigAdded++ == 0))
        htmlParser.appendField("THIS_COMPUTER", '(' + tr("This computer") + ')');
    }
    else
      htmlParser.setField("ITEM_LOCATION", "frontend:/iframe/" + server.presentationURL.toEncoded().toHex());

    if ((QHostAddress(host) == QHostAddress::LocalHost) ||
        (QHostAddress(host) == QHostAddress::LocalHostIPv6))
    {
      foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
      if ((address.protocol() == QAbstractSocket::IPv4Protocol) &&
          (address != QHostAddress::LocalHost))
      {
        host = address.toString();
        break;
      }
    }

    host += ':' + QString::number(server.presentationURL.port());
    htmlParser.setField("ITEM_HOST", host);

    if (server.modelName == qApp->applicationName())
      lximediaServers.insert(server.friendlyName, htmlParser.parse(htmlServer));
    else
      otherServers.insert(server.friendlyName, htmlParser.parse(htmlServer));
  }

  if (localConfigAdded)
  {
    htmlParser.setField("LOCAL_SERVER_STATUS", tr("The local server is running"));
    htmlParser.appendField("LOCAL_SERVER_STATUS", htmlParser.parse(htmlStopLocalServer));
    htmlParser.appendField("LOCAL_SERVER_STATUS", htmlParser.parse(htmlConfigureLocalServer));
  }
#if !defined(Q_OS_MACX)
  else if (SDaemon::isRunning(daemonName))
#else
  else if (qAbs(startingTimer.elapsed()) < 30000)
#endif
  {
    htmlParser.setField("LOCAL_SERVER_STATUS", tr("The local server is starting"));
  }
  else
  {
    htmlParser.setField("LOCAL_SERVER_STATUS", tr("The local server is not running"));
    htmlParser.appendField("LOCAL_SERVER_STATUS", htmlParser.parse(htmlStartLocalServer));
  }

#if defined(Q_OS_MACX)
  if (QFile::exists(QDir::homePath() + "/Library/LaunchAgents/" + daemonName + ".plist"))
  {
    htmlParser.appendField("LOCAL_SERVER_STATUS", htmlParser.parse(htmlDisableLocalServer));
    htmlParser.setField("LOCAL_SERVER", htmlParser.parse(htmlLocalServer));
  }
#else
  if (SDaemon::isInstalled(daemonName))
    htmlParser.setField("LOCAL_SERVER", htmlParser.parse(htmlLocalServer));
#endif
  else
    htmlParser.setField("LOCAL_SERVER", htmlParser.parse(htmlNoLocalServer));

  if (!lximediaServers.isEmpty())
  {
    htmlParser.setField("LXIMEDIACENTER_SERVERS", "");
    foreach (const QByteArray &server, lximediaServers)
      htmlParser.appendField("LXIMEDIACENTER_SERVERS", server);
  }
  else
    htmlParser.setField("LXIMEDIACENTER_SERVERS", htmlParser.parse(htmlNoServers));

  if (!otherServers.isEmpty())
  {
    htmlParser.setField("OTHER_SERVERS", "");
    foreach (const QByteArray &server, otherServers)
      htmlParser.appendField("OTHER_SERVERS", server);
  }
  else
    htmlParser.setField("OTHER_SERVERS", htmlParser.parse(htmlNoServers));

  htmlParser.setField("CONTENT", htmlParser.parse(htmlServers));
  return htmlParser.parse(htmlIndex);
}

QByteArray Frontend::makeIFrame(const QByteArray &url) const
{
  SStringParser htmlParser;
  htmlParser.setField("NAVIGATOR", htmlParser.parse(htmlNavigator));
  htmlParser.setField("PATH", htmlParser.escapeXml(url));

  return htmlParser.parse(htmlIFrame);
}
