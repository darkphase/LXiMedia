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

  masterServer->recycleSandbox(sandbox);
  return NULL;
}

SHttpServer::ResponseMessage InternetServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

int InternetServer::countItems(const QString &path)
{
  if (path == serverPath())
  {
    PluginSettings settings(Module::pluginName);

    return siteDatabase->countSites(settings.value("Audiences").toStringList());
  }
  else
  {
    const QString localPath = sitePath(path);
    const QString name = localPath.left(localPath.indexOf('/'));

    ScriptEngine * const engine = getScriptEngine(name);
    if (engine)
      return engine->listItems(localPath.mid(name.length())).count();
  }

  return 0;
}

QList<InternetServer::Item> InternetServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> result;

  if (path == serverPath())
  {
    PluginSettings settings(Module::pluginName);

    foreach (const QString &name, siteDatabase->getSites(settings.value("Audiences").toStringList(), start, count))
    {
      Item item;
      item.isDir = true;
      item.type = Item::Type_None;
      item.path = serverPath() + name + '/';
      item.url = item.path;
      item.iconUrl = item.url;
      item.iconUrl.addQueryItem("thumbnail", QString::null);

      item.title = name;

      result += item;
    }
  }
  else
  {
    const QString localPath = sitePath(path);
    const QString name = localPath.left(localPath.indexOf('/'));

    ScriptEngine * const engine = getScriptEngine(name);
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

SHttpServer::ResponseMessage InternetServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    if (request.url().hasQueryItem("thumbnail"))
    {
      const QSize size = SSize::fromString(request.url().queryItemValue("thumbnail")).size();
      QString defaultIcon = ":/img/null.png";
      QByteArray content;

      QString name = sitePath(request.file());
      name = name.left(name.indexOf('/'));

      ScriptEngine * const engine = getScriptEngine(name);
      if (engine)
        content = makeThumbnail(size, engine->icon(request.fileName()), request.url().queryItemValue("overlay"));

      if (content.isEmpty())
        content = makeThumbnail(size, QImage(defaultIcon));

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok, content, SHttpEngine::mimeImagePng);
    }
    else if (request.url().hasQueryItem("site_tree"))
    {
      PluginSettings settings(Module::pluginName);

      QStringList selectedAudiences = settings.value("Audiences").toStringList();

      const QString checkon = QString::fromUtf8(QByteArray::fromHex(request.url().queryItemValue("checkon").toAscii()));
      const QString checkoff = QString::fromUtf8(QByteArray::fromHex(request.url().queryItemValue("checkoff").toAscii()));
      if (!checkon.isEmpty() || !checkoff.isEmpty())
      {
        if (!checkon.isEmpty())
          selectedAudiences.append(checkon);

        if (!checkoff.isEmpty())
          selectedAudiences.removeAll(checkoff);

        settings.setValue("Audiences", selectedAudiences);
      }

      const QString open = request.url().queryItemValue("open");
      const QSet<QString> allopen = !open.isEmpty()
                                    ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                    : QSet<QString>();

      HtmlParser htmlParser;
      htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());
      htmlParser.setField("DIRS", QByteArray(""));
      foreach (const QString &targetAudience, siteDatabase->allAudiences())
      {
        htmlParser.setField("DIR_FULLPATH", targetAudience.toUtf8().toHex());
        htmlParser.setField("DIR_INDENT", QByteArray(""));

        // Expand
        bool addChildren = false;
        QSet<QString> all = allopen;
        if (all.contains(targetAudience))
        {
          all.remove(targetAudience);
          htmlParser.setField("DIR_OPEN", QByteArray("open"));
          addChildren = true;
        }
        else
        {
          all.insert(targetAudience);
          htmlParser.setField("DIR_OPEN", QByteArray("close"));
        }

        htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(all.toList()).join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSiteTreeExpand));

        if (selectedAudiences.contains(targetAudience))
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("full"));
          htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkoff"));
        }
        else
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("none"));
          htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkon"));
        }

        htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSiteTreeCheckLink));

        htmlParser.setField("DIR_NAME", targetAudience);

        htmlParser.appendField("DIRS", htmlParser.parse(htmlSiteTreeDir));

        if (addChildren)
        foreach (const QString &name, siteDatabase->getSites(targetAudience))
        {
          htmlParser.setField("DIR_FULLPATH", name.toUtf8().toHex());
          htmlParser.setField("DIR_INDENT", htmlParser.parse(htmlSiteTreeIndent) + htmlParser.parse(htmlSiteTreeIndent));
          htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
          htmlParser.setField("DIR_EXPAND", QByteArray(""));

          htmlParser.setField("ITEM_ICON", serverPath() + name + "/?thumbnail=16");
          htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSiteTreeCheckIcon));

          htmlParser.setField("ITEM_NAME", name);
          htmlParser.setField("DIR_NAME", htmlParser.parse(htmlSiteTreeScriptLink));

          htmlParser.appendField("DIRS", htmlParser.parse(htmlSiteTreeDir));
        }
      }

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setField("Cache-Control", "no-cache");
      response.setContentType("text/html;charset=utf-8");
      response.setContent(htmlParser.parse(htmlSiteTreeIndex));
      return response;
    }
    else if (request.url().hasQueryItem("edit"))
    {
      QString name = QString::fromUtf8(QByteArray::fromHex(request.url().queryItemValue("edit").toAscii()));
      name = name.left(name.indexOf('/'));

      const QString script = siteDatabase->getScript(name);
      if (!script.isEmpty())
      {
        HtmlParser htmlParser;
        htmlParser.setField("TR_SCRIPT_EDITOR", tr("Script editor"));
        htmlParser.setField("TR_SAVE", tr("Save"));

        htmlParser.setField("SCRIPT_NAME", name);
        htmlParser.setField("ENCODED_SCRIPT_NAME", name.toUtf8().toHex());
        htmlParser.setField("SCRIPT", script);

        SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
        response.setField("Cache-Control", "no-cache");
        response.setContentType("text/html;charset=utf-8");
        response.setContent(htmlParser.parse(htmlSiteEditIndex));
        return response;
      }
    }
  }

  return MediaServer::httpRequest(request, socket);
}

QString InternetServer::sitePath(const QString &path) const
{
  return path.mid(serverPath().length());
}

ScriptEngine * InternetServer::getScriptEngine(const QString &name)
{
  if (!name.isEmpty())
  {
    QMap<QString, ScriptEngine *>::Iterator i = scriptEngines.find(name);
    if (i == scriptEngines.end())
    {
      ScriptEngine * const engine =
          new ScriptEngine(siteDatabase->getScript(name));

      if (engine->isValid())
        i = scriptEngines.insert(name, engine);
      else
        delete engine;
    }

    if (i != scriptEngines.end())
      return *i;
  }

  return NULL;
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

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *)));

  return true;
}

} } // End of namespaces
