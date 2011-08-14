/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
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

#include "scriptengine.h"

namespace LXiMediaCenter {
namespace InternetBackend {

ScriptEngine::ScriptEngine(const QString &script, QObject *parent)
  : QObject(parent),
    engine(this)
{
  connect(&engine, SIGNAL(signalHandlerException(QScriptValue)), SLOT(exceptionHandler(QScriptValue)));

  engine.evaluate(script);
  global = engine.globalObject();

  versionFunc = global.property("version");
  targetAudienceFunc = global.property("targetAudience");

  iconFunc = global.property("icon");
  listItemsFunc = global.property("listItems");
  streamLocationFunc = global.property("streamLocation");
}

QString ScriptEngine::version(void)
{
  if (!versionFunc.isNull())
    return versionFunc.call(me).toString();

  return QString::null;
}

bool ScriptEngine::isCompatible(void)
{
  const QStringList myVersion = qApp->applicationVersion().split('-').first().split('.');
  const QStringList scriptVersion = version().split('.');

  if (myVersion.count() == scriptVersion.count())
  {
    for (int i=0; i<myVersion.count(); i++)
    if (scriptVersion[i].toInt() > myVersion[i].toInt())
      return false;

    return true;
  }

  return false;
}

QString ScriptEngine::targetAudience(void)
{
  if (!targetAudienceFunc.isNull())
    return targetAudienceFunc.call(me).toString();

  return QString::null;
}

QImage ScriptEngine::icon(const QString &id)
{
  if (!iconFunc.isNull())
    return QImage::fromData(QByteArray::fromBase64(iconFunc.call(me, QScriptValueList() << id).toString().toAscii()));

  return QImage();
}

QList<MediaServer::Item> ScriptEngine::listItems(const QString &path)
{
  QList<MediaServer::Item> result;

  if (!listItemsFunc.isNull())
  {
    const QScriptValue items = listItemsFunc.call(me, QScriptValueList() << path);
    for (int i=0; true; i++)
    {
      const QScriptValue item = items.property(i);
      if (item.isArray())
      {
        const QString streamId = item.property(0).toString();
        const QString name = item.property(1).toString();
        const QString type = item.property(2).toString();

        MediaServer::Item item;
        item.direct = true;

        if (type.compare("Video", Qt::CaseInsensitive))
          item.type = SUPnPContentDirectory::Item::Type_Video;
        else if (type.compare("Audio", Qt::CaseInsensitive))
          item.type = SUPnPContentDirectory::Item::Type_Audio;
        else if (type.compare("Image", Qt::CaseInsensitive))
          item.type = SUPnPContentDirectory::Item::Type_Image;
        else
          item.type = SUPnPContentDirectory::Item::Type_None;

        item.url = streamId;
        item.iconUrl = streamId + "-thumb.png";

        item.title = name;

        result += item;
      }
      else
        break;
    }
  }

  return result;
}

QString ScriptEngine::streamLocation(const QString &id)
{
  if (!streamLocationFunc.isNull())
    return streamLocationFunc.call(me, QScriptValueList() << id).toString();

  return QString::null;
}

void ScriptEngine::exceptionHandler(const QScriptValue &exception)
{
  qDebug() << "Script exception:" << exception.toString();
}

} } // End of namespaces
