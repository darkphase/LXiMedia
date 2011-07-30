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

class MediaPlayerServerDir;

class ScriptEngine : public QObject
{
Q_OBJECT
public:
  explicit                      ScriptEngine(const QString &script, QObject *parent = NULL);

  QString                       version(void);
  bool                          isCompatible(void);
  QString                       friendlyName(void);
  QString                       targetAudience(void);

  QImage                        icon(const QString &id);
  QList<MediaServer::Item>      listItems(const QString &path);

private slots:
  void                          exceptionHandler(const QScriptValue &exception);

private:
  QScriptEngine                 engine;
  QScriptValue                  global;
  QScriptValue                  me;

  QScriptValue                  versionFunc;
  QScriptValue                  friendlyNameFunc;
  QScriptValue                  targetAudienceFunc;

  QScriptValue                  iconFunc;
  QScriptValue                  listItemsFunc;
  QScriptValue                  streamLocationFunc;
};

} } // End of namespaces

#endif
