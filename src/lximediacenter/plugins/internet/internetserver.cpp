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

#include "internetserver.h"
#include <QtScript>
//#include "internetsandbox.h"
#include "module.h"

namespace LXiMediaCenter {
namespace InternetBackend {

InternetServer::InternetServer(SiteDatabase::Category category, QObject *parent)
  : MediaServer(parent),
    category(category),
    masterServer(NULL),
    siteDatabase(NULL)
{
}

void InternetServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  this->siteDatabase = SiteDatabase::createInstance();

  MediaServer::initialize(masterServer);
}

void InternetServer::close(void)
{
  MediaServer::close();
}

QString InternetServer::pluginName(void) const
{
  return Module::pluginName;
}

InternetServer::SearchResultList InternetServer::search(const QStringList &rawQuery) const
{
  SearchResultList list;

  return list;
}

InternetServer::Stream * InternetServer::streamVideo(const SHttpServer::RequestMessage &request)
{

  return NULL;
}

int InternetServer::countItems(const QString &)
{
  return siteDatabase->countSites(category);
}

QList<InternetServer::Item> InternetServer::listItems(const QString &, unsigned start, unsigned count)
{
  QList<Item> result;

  foreach (const QString &hostname, siteDatabase->getSites(category, start, count))
  {
    QString countries, script;
    SiteDatabase::Category category = SiteDatabase::Category_None;

    siteDatabase->getSite(hostname, countries, category, script);

    QScriptEngine engine;
    engine.evaluate(script);

    QScriptValue global = engine.globalObject();
    QScriptValue streamLocationFunc = global.property("streamLocation");
    QScriptValue iconLocationFunc = global.property("iconLocation");

    const QString streamLocation = streamLocationFunc.call().toString();
    const QString iconLocation = iconLocationFunc.call().toString();

    Item item;
    item.title = hostname;
    item.type = defaultItemType();
    item.url = streamLocation.toUtf8().toHex();
    item.iconUrl = iconLocation.toUtf8().toHex() + "-thumb.png";

    result += item;
  }

  return result;
}

SHttpServer::SocketOp InternetServer::handleHttpRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  return MediaServer::handleHttpRequest(request, socket);
}

InternetServer::Item::Type InternetServer::defaultItemType(void) const
{
  switch (category)
  {
  default:
  case SiteDatabase::Category_None:
    return Item::Type_None;

  case SiteDatabase::Category_Radio:
    return Item::Type_Music;

  case SiteDatabase::Category_Television:
    return Item::Type_Video;
  }
}

InternetServer::Stream::Stream(InternetServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

InternetServer::Stream::~Stream()
{
  static_cast<InternetServer *>(parent)->masterServer->recycleSandbox(sandbox);
}

bool InternetServer::Stream::setup(const QUrl &url, const QByteArray &content)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
  message.setContent(content);

  sandbox->openRequest(message, &proxy, SLOT(setSource(QAbstractSocket *)));

  return true;
}

} } // End of namespaces
