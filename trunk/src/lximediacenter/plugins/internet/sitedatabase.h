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
  int                           countSites(const QString &audience) const;
  int                           countSites(const QStringList &audiences) const;
  QStringList                   getSites(const QString &audience, unsigned start = 0, unsigned count = 0) const;
  QStringList                   getSites(const QStringList &audiences, unsigned start = 0, unsigned count = 0) const;

  QString                       getScript(const QString &name) const;

protected:
  virtual void                  customEvent(QEvent *);

private:
  void                          readScript(const QString &fileName);
  void                          addScript(const QString &name, const QString &audience, const QString &script);

private:
  class ScriptUpdateEvent;

  static const QEvent::Type     scriptUpdateEventType;
  static SiteDatabase         * self;

  QList< QFuture<void> >        loadFutures;
  QMap<QString, QString>        scripts;
  QMap<QString, QStringList>    audiences;
};

} } // End of namespaces

#endif
