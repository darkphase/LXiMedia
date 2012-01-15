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
#include "internetsandbox.h"
#include "scriptengine.h"
#include "module.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char InternetServer::dirSplit =
#if defined(Q_OS_UNIX)
    ':';
#elif  defined(Q_OS_WIN)
    ';';
#else
#error Not implemented.
#endif

InternetServer::InternetServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL),
    siteDatabase(NULL)
{
}

void InternetServer::initialize(MasterServer *masterServer)
{
  foreach (ScriptEngine *engine, scriptEngines)
    delete engine;

  this->masterServer = masterServer;
  this->siteDatabase = SiteDatabase::createInstance();

  MediaServer::initialize(masterServer);
}

void InternetServer::close(void)
{
  MediaServer::close();

  for (QMap<QString, ScriptEngine *>::Iterator i=scriptEngines.begin(); i!=scriptEngines.end(); i++)
    delete *i;

  scriptEngines.clear();
}

QString InternetServer::serverName(void) const
{
  return Module::pluginName;
}

QString InternetServer::serverIconPath(void) const
{
  return "/img/homepage.png";
}

InternetServer::Stream * InternetServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  SSandboxClient::Priority priority = SSandboxClient::Priority_Normal;
  if (request.url().queryItemValue("priority") == "low")
    priority = SSandboxClient::Priority_Low;
  else if (request.url().queryItemValue("priority") == "high")
    priority = SSandboxClient::Priority_High;

  SSandboxClient * const sandbox = masterServer->createSandbox(priority);
  sandbox->ensureStarted();

  const QString localPath = sitePath(request.file());
  const QString name = localPath.left(localPath.indexOf('/'));

  ScriptEngine * const engine = getScriptEngine(name);
  if (engine)
  {
    const QString location = engine->streamLocation(localPath.mid(name.length()));
    if (!location.isEmpty())
    {
      QUrl rurl;
      rurl.setPath(InternetSandbox::path + request.fileName());
      rurl.addQueryItem("playstream", QString::null);
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, request.url().queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      Stream *stream = new Stream(this, sandbox, request.path());
      if (stream->setup(rurl, location.toUtf8()))
        return stream; // The graph owns the socket now.

      delete stream;
    }
  }

  delete sandbox;
  return NULL;
}

SHttpServer::ResponseMessage InternetServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

QList<InternetServer::Item> InternetServer::listItems(const QString &path, int start, int &count)
{
  QList<Item> result;

  if (path == serverPath())
  {
    QSettings settings;
    settings.beginGroup(Module::pluginName);

    foreach (const QString &host, siteDatabase->listSites(settings.value("Audiences").toStringList(), start, count))
    {
      Item item;
      item.isDir = true;
      item.type = Item::Type_None;
      item.title = siteDatabase->getName(host);
      item.path = serverPath() + item.title + '/';
      item.url = item.path;
      item.iconUrl = serverPath() + item.title + '/';
      item.iconUrl.addQueryItem("thumbnail", QString::null);

      result += item;
    }
  }
  else
  {
    const QString localPath = sitePath(path);
    const QString name = localPath.left(localPath.indexOf('/'));
    const QString host = siteDatabase->getHost(name);

    ScriptEngine * const engine = getScriptEngine(host);
    if (engine)
    foreach (const ScriptEngine::Item &sitem, engine->listItems(localPath.mid(name.length()), start, count))
    {
      Item item;
      item.isDir = false;
      item.type = sitem.type;
      item.path = serverPath() + localPath + sitem.name;
      item.url = item.path;
      item.iconUrl = item.url;
      item.iconUrl.addQueryItem("thumbnail", QString::null);

      item.title = sitem.name;

      result += item;
    }
  }

  return result;
}

InternetServer::Item InternetServer::getItem(const QString &path)
{
  Item item;

  if (path.endsWith('/'))
  {
    item.isDir = true;
    item.type = Item::Type_None;
    item.path = path;
    item.url = item.path;
    item.iconUrl = item.url;
    item.iconUrl.addQueryItem("thumbnail", QString::null);

    item.title = sitePath(path);
    item.title = item.title.left(item.title.indexOf('/'));
  }

  return item;
}

QString InternetServer::sitePath(const QString &path) const
{
  return path.mid(serverPath().length());
}

ScriptEngine * InternetServer::getScriptEngine(const QString &host)
{
  if (!host.isEmpty())
  {
    QMap<QString, ScriptEngine *>::Iterator i = scriptEngines.find(host);
    if (i == scriptEngines.end())
    {
      ScriptEngine * const engine =
          new ScriptEngine(siteDatabase->getScript(host));

      if (engine->isValid())
        i = scriptEngines.insert(host, engine);
      else
        delete engine;
    }

    if (i != scriptEngines.end())
      return *i;
  }

  return NULL;
}

void InternetServer::deleteScriptEngine(const QString &host)
{
  if (!host.isEmpty())
  {
    QMap<QString, ScriptEngine *>::Iterator i = scriptEngines.find(host);
    if (i != scriptEngines.end())
    {
      delete *i;
      scriptEngines.erase(i);
    }
  }
}


InternetServer::Stream::Stream(InternetServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

InternetServer::Stream::~Stream()
{
  delete sandbox;
}

bool InternetServer::Stream::setup(const QUrl &url, const QByteArray &content)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
  message.setContent(content);

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *, SHttpEngine *)), Qt::DirectConnection);

  return true;
}

} } // End of namespaces
