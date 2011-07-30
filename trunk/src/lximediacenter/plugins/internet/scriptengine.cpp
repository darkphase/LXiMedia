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
  friendlyNameFunc = global.property("friendlyName");
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

QString ScriptEngine::friendlyName(void)
{
  if (!friendlyNameFunc.isNull())
    return friendlyNameFunc.call(me).toString();

  return QString::null;
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
    const QString itemsStr = listItemsFunc.call(me, QScriptValueList() << path).toString();
    foreach (const QString &itemStr, itemsStr.split(';', QString::SkipEmptyParts))
    {
      const QStringList vars = itemStr.split('|');
      if (vars.count() >= 3)
      {
        MediaServer::Item item;

        if (vars[2] == "Video")
          item.type = SUPnPContentDirectory::Item::Type_Video;
        else if (vars[2] == "Audio")
          item.type = SUPnPContentDirectory::Item::Type_Audio;
        else if (vars[2] == "Image")
          item.type = SUPnPContentDirectory::Item::Type_Image;
        else
          item.type = SUPnPContentDirectory::Item::Type_None;

        item.url = vars[0];
        item.iconUrl = vars[0] + "-thumb.png";

        item.title = vars[1];

        result += item;
      }
    }
  }

  return result;
}

void ScriptEngine::exceptionHandler(const QScriptValue &exception)
{
  qDebug() << "Script exception:" << exception.toString();
}

} } // End of namespaces
