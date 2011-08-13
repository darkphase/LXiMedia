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

InternetServer::InternetServer(const QString &category, QObject *parent)
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

  cacheTimer.start();
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
  const MediaServer::File file(request);

  SSandboxClient::Priority priority = SSandboxClient::Priority_Normal;
  if (file.url().queryItemValue("priority") == "low")
    priority = SSandboxClient::Priority_Low;
  else if (file.url().queryItemValue("priority") == "high")
    priority = SSandboxClient::Priority_High;

  SSandboxClient * const sandbox = masterServer->createSandbox(priority);
  sandbox->ensureStarted();

  const QString item = file.url().hasQueryItem("item")
    ? file.url().queryItemValue("item")
    : file.baseName();

  if (!item.isEmpty())
  {
    const QString script = siteDatabase->script(siteDatabase->reverseDomain(file.parentDir()));
    if (!script.isEmpty())
    {
      ScriptEngine engine(script);
      const QString location = engine.streamLocation(item);
      if (!location.isEmpty())
      {
        QUrl rurl;
        rurl.setPath(InternetSandbox::path + file.fullName());
        rurl.addQueryItem("playstream", QString::null);
        typedef QPair<QString, QString> QStringPair;
        foreach (const QStringPair &queryItem, file.url().queryItems())
          rurl.addQueryItem(queryItem.first, queryItem.second);

        Stream *stream = new Stream(this, sandbox, request.path());
        if (stream->setup(rurl, location.toUtf8()))
          return stream; // The graph owns the socket now.

        delete stream;
      }
    }
  }

  masterServer->recycleSandbox(sandbox);

  return NULL;
}

int InternetServer::countItems(const QString &path)
{
  if (path == "/")
  {
    PluginSettings settings(pluginName());

    return siteDatabase->countSites(category, settings.value("Audiences").toStringList());
  }
  else
    return cachedItems(path).count();
}

QList<InternetServer::Item> InternetServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> result;

  if (path == "/")
  {
    PluginSettings settings(pluginName());

    foreach (const QString &identifier, siteDatabase->getSites(category, settings.value("Audiences").toStringList(), start, count))
    {
      Item item;
      item.isDir = true;
      item.type = Item::Type_None;
      item.title = siteDatabase->reverseDomain(identifier);
      item.iconUrl = siteDatabase->reverseDomain(identifier) + "/-thumb.png";

      result += item;
    }
  }
  else
  {
    const QList<Item> &items = cachedItems(path);

    const bool returnAll = count == 0;
    for (int i=start, n=0; (i<items.count()) && (returnAll || (n<int(count))); i++, n++)
      result.append(items[i]);
  }

  return result;
}

SHttpServer::ResponseMessage InternetServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);
    if (file.fullName().endsWith("-thumb.png"))
    {
      QSize size(128, 128);
      if (file.url().hasQueryItem("resolution"))
      {
        const QStringList sizeTxt = file.url().queryItemValue("resolution").split('x');
        if (sizeTxt.count() >= 2)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
        else if (sizeTxt.count() >= 1)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());
      }

      const QString script = siteDatabase->script(siteDatabase->reverseDomain(file.parentDir()));
      if (!script.isEmpty())
      {
        ScriptEngine engine(script);
        QImage image = engine.icon(file.baseName().left(file.baseName().length() - 6));
        if (!image.isNull())
          image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QImage result(size, QImage::Format_ARGB32);
        QPainter p;
        p.begin(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
        p.fillRect(result.rect(), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
        p.drawImage(
            (result.width() / 2) - (image.width() / 2),
            (result.height() / 2) - (image.height() / 2),
            image);

        if (file.url().hasQueryItem("overlay"))
        {
          QImage overlayImage(":/lximediacenter/images/" + file.url().queryItemValue("overlay") + ".png");
          if (!overlayImage.isNull())
          {
            overlayImage = overlayImage.scaled(size / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            p.drawImage(
                (result.width() / 2) - (overlayImage.width() / 2),
                (result.height() / 2) - (overlayImage.height() / 2),
                overlayImage);
          }
        }

        p.end();

        QBuffer b;
        result.save(&b, "PNG");

        return makeResponse(request, b.data(), "image/png", true);
      }

      QImage image(":/lximediacenter/images/video-template.png");
      if (!image.isNull())
      {
        if (file.url().hasQueryItem("resolution"))
          image = image.scaled(size / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QImage sum(size, QImage::Format_ARGB32);
        QPainter p;
        p.begin(&sum);
        p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
        p.fillRect(sum.rect(), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
        p.drawImage(
            (sum.width() / 2) - (image.width() / 2),
            (sum.height() / 2) - (image.height() / 2),
            image);
        p.end();

        QBuffer b;
        sum.save(&b, "PNG");

        return makeResponse(request, b.data(), "image/png", true);
      }

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_MovedPermanently);
      response.setField("Location", "http://" + request.host() + "/img/null.png");
      return response;
    }
    else if (file.suffix() == "html") // Show player
      return makeHtmlContent(request, file.url(), buildVideoPlayer(file.baseName().toUtf8(), file.baseName(), file.url()), headPlayer);
  }

  return MediaServer::httpRequest(request, socket);
}

const QList<InternetServer::Item> & InternetServer::cachedItems(const QString &path)
{
  static const QList<Item> null;

  if (qAbs(cacheTimer.restart()) >= 5 * 60000)
    itemCache.clear();

  const int sep = path.indexOf('/', 1);
  const QString identifier = siteDatabase->reverseDomain(path.mid(1, sep - 1));

  QMap<QString, QList<Item> >::Iterator i = itemCache.find(path);
  if (i == itemCache.end())
  {
    const QString script = siteDatabase->script(identifier);
    if (!script.isEmpty())
    {
      ScriptEngine engine(script);

      i = itemCache.insert(path, engine.listItems(path.mid(sep)));
    }
    else
      return null;
  }

  return *i;
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
