/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#ifndef SITEDATABASE_H
#define SITEDATABASE_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {
namespace InternetBackend {

class SiteDatabase : public QObject
{
Q_OBJECT
public:
  static SiteDatabase         * createInstance(void);
  static void                   destroyInstance(void);

private:
  explicit                      SiteDatabase(QObject *parent = NULL);
  virtual                       ~SiteDatabase();

public:
  QStringList                   allAudiences(void) const;
  QStringList                   listSites(const QString &audience, int start, int &count) const;
  QStringList                   listSites(const QStringList &audiences, int start, int &count) const;

  QString                       getScript(const QString &host) const;
  bool                          updateScript(const QString &host, const QString &script);
  bool                          deleteLocalScript(const QString &host);
  bool                          isLocal(const QString &host) const;
  bool                          isGlobal(const QString &host) const;

  QString                       getHost(const QString &name) const;
  QString                       getName(const QString &host) const;

protected:
  virtual void                  customEvent(QEvent *);

private:
  bool                          readScript(const QString &host, const QString &script);
  void                          addScript(const QString &host, const QString &name, const QString &audience);
  void                          removeScript(const QString &host);

  static QString                localScriptDirPath(void);

private:
  class ScriptUpdateEvent;

  static const QEvent::Type     scriptUpdateEventType;
  static SiteDatabase         * self;

  QDir                          localScriptDir;
  QDir                          globalScriptDir;
  QSet<QString>                 names;
  QMap<QString, QString>        hostByName;
  QMap<QString, QString>        nameByHost;
  QMap<QString, QSet<QString> > audiences;

  mutable QList< QFuture<bool> > loadFutures;
};

} } // End of namespaces

#endif
