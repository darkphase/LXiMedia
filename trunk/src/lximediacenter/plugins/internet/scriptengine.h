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

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QtScript>
#include <LXiStream>
#include <LXiMediaCenter>

namespace LXiMediaCenter {
namespace InternetBackend {

class ScriptEngine : public QObject
{
Q_OBJECT
public:
  struct Item
  {
    QString                     name;
    MediaServer::Item::Type     type;
  };

public:
  explicit                      ScriptEngine(const QString &script, QObject *parent = NULL);

  bool                          isValid(void) const;

  QString                       version(void);
  bool                          isCompatible(void);
  QString                       name(void);
  QString                       audience(void);

  QImage                        icon(const QString &name);
  QList<Item>                   listItems(const QString &path, int start, int &count);
  QString                       streamLocation(const QString &name);

private slots:
  void                          exceptionHandler(const QScriptValue &exception);

private:
  QScriptEngine                 engine;
  QScriptValue                  global;
  QScriptValue                  me;

  QScriptValue                  versionFunc;
  QScriptValue                  nameFunc;
  QScriptValue                  audienceFunc;

  QScriptValue                  iconFunc;
  QScriptValue                  listItemsFunc;
  QScriptValue                  streamLocationFunc;

  QMap<QString, QList<Item> >   itemCache;
  QMap<QString, QByteArray>     iconCache;
};

} } // End of namespaces

#endif
