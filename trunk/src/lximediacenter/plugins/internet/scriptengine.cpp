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
  if (!script.isEmpty())
  {
    connect(&engine, SIGNAL(signalHandlerException(QScriptValue)), SLOT(exceptionHandler(QScriptValue)));

    engine.evaluate(script);
    global = engine.globalObject();

    versionFunc = global.property("version");
    nameFunc = global.property("name");
    audienceFunc = global.property("audience");

    iconFunc = global.property("icon");
    listItemsFunc = global.property("listItems");
    streamLocationFunc = global.property("streamLocation");
  }
}

bool ScriptEngine::isValid(void) const
{
  return
      !versionFunc.isNull() &&
      !nameFunc.isNull() &&
      !audienceFunc.isNull();
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

QString ScriptEngine::name(void)
{
  if (!nameFunc.isNull())
    return nameFunc.call(me).toString();

  return QString::null;
}

QString ScriptEngine::audience(void)
{
  if (!audienceFunc.isNull())
    return audienceFunc.call(me).toString();

  return QString::null;
}

QImage ScriptEngine::icon(const QString &name)
{
  QMap<QString, QByteArray>::Iterator icon = iconCache.find(name);
  if ((icon == iconCache.end()) && !iconFunc.isNull())
    icon = iconCache.insert(name, QByteArray::fromBase64(iconFunc.call(me, QScriptValueList() << name).toString().toAscii()));

  if (icon != iconCache.end())
    return QImage::fromData(*icon);

  return QImage();
}

QList<ScriptEngine::Item> ScriptEngine::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<ScriptEngine::Item> result;

  QMap<QString, QList<Item> >::Iterator items = itemCache.find(path);
  if ((items == itemCache.end()) && !listItemsFunc.isNull())
  {
    items = itemCache.insert(path, QList<Item>());

    const QScriptValue fresult = listItemsFunc.call(me, QScriptValueList() << path);
    for (int i=0; true; i++)
    {
      const QScriptValue fresultitem = fresult.property(i);
      if (fresultitem .isArray())
      {
        Item item;
        item.name = fresultitem .property(0).toString();

        const QString type = fresultitem .property(1).toString();
        if (type.compare("Video", Qt::CaseInsensitive) == 0)
          item.type = MediaServer::Item::Type_Video;
        else if (type.compare("Audio", Qt::CaseInsensitive) == 0)
          item.type = MediaServer::Item::Type_Audio;
        else if (type.compare("Image", Qt::CaseInsensitive) == 0)
          item.type = MediaServer::Item::Type_Image;
        else
          item.type = MediaServer::Item::Type_None;

        items->append(item);
      }
      else
        break;
    }
  }

  if (items != itemCache.end())
  {
    const bool returnAll = count == 0;
    for (unsigned i=start, n=0; (int(i)<items->count()) && (returnAll || (n<count)); i++, n++)
      result += (*items)[i];
  }

  return result;
}

QString ScriptEngine::streamLocation(const QString &name)
{
  if (!streamLocationFunc.isNull())
    return streamLocationFunc.call(me, QScriptValueList() << name).toString();

  return QString::null;
}

void ScriptEngine::exceptionHandler(const QScriptValue &exception)
{
  qDebug() << "Script exception:" << exception.toString();
}

} } // End of namespaces
