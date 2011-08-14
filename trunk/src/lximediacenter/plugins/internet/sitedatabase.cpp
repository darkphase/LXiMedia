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

#include "sitedatabase.h"
#include "module.h"
#include "scriptengine.h"

namespace LXiMediaCenter {
namespace InternetBackend {

class SiteDatabase::ScriptUpdateEvent : public QEvent
{
public:
  inline ScriptUpdateEvent(const QString &identifier,
                           const QString &targetAudience, const QString &script)
    : QEvent(scriptUpdateEventType), identifier(identifier),
      targetAudience(targetAudience), script(script)
  {
  }

public:
  const QString                 identifier;
  const QString                 targetAudience;
  const QString                 script;
};

const QEvent::Type  SiteDatabase::scriptUpdateEventType = QEvent::Type(QEvent::registerEventType());
SiteDatabase      * SiteDatabase::self = NULL;

SiteDatabase * SiteDatabase::createInstance(void)
{
  if (self)
    return self;
  else
    return self = new SiteDatabase();
}

void SiteDatabase::destroyInstance(void)
{
  delete self;
}

SiteDatabase::SiteDatabase(QObject *parent)
  : QObject(parent)
{
  PluginSettings settings(Module::pluginName);

  Database::Query query;

  // Drop all tables if the database version is outdated.
  static const int databaseVersion = 1;
  //if (settings.value("DatabaseVersion", 0).toInt() != databaseVersion)
  {
    qDebug() << "Internet site database layout changed, recreating tables.";

    QStringList tables;
    query.exec("SELECT name FROM sqlite_master WHERE type='table'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Internet"))
        tables += name;
    }

    QStringList indices;
    query.exec("SELECT name FROM sqlite_master WHERE type='index'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Internet"))
        indices += name;
    }

    query.exec("PRAGMA foreign_keys = OFF");

    Database::transaction();

    foreach (const QString &name, indices)
      query.exec("DROP INDEX IF EXISTS " + name);

    foreach (const QString &name, tables)
      query.exec("DROP TABLE IF EXISTS " + name);

    Database::commit();

    query.exec("PRAGMA foreign_keys = ON");

    //query.exec("VACUUM");
  }

  // Create tables that don't exist
  query.exec("CREATE TABLE IF NOT EXISTS InternetSites ("
             "identifier     TEXT UNIQUE NOT NULL,"
             "name           TEXT NOT NULL,"
             "targetAudience TEXT NOT NULL,"
             "script         TEXT NOT NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS InternetSites_identifier "
             "ON InternetSites(identifier)");

  query.exec("CREATE INDEX IF NOT EXISTS InternetSites_targetAudience "
             "ON InternetSites(targetAudience)");

  settings.setValue("DatabaseVersion", databaseVersion);

  // Update the database
  foreach (const QFileInfo &info, QDir(":/internet/sites/").entryInfoList(QStringList() << "*.js", QDir::Files))
  if (needsUpdate(info.completeBaseName()))
    QtConcurrent::run(this, &SiteDatabase::updateScript, info);
}

SiteDatabase::~SiteDatabase()
{
  QThreadPool::globalInstance()->waitForDone();

  self = NULL;
}

bool SiteDatabase::needsUpdate(const QString &identifierStr) const
{
  const QStringList identifier = identifierStr.split('!');
  if (identifier.count() >= 2)
  {
    Database::Query query;
    query.prepare("SELECT identifier FROM InternetSites "
                  "WHERE identifier LIKE :identifier");
    query.bindValue(0, identifier[0] + "!%");
    query.exec();
    if (query.next())
    {
      const QStringList list = query.value(0).toString().split('!');
      if (list.count() >= 2)
        return list[1] < identifier[1];
    }

    return true;
  }

  return false;
}

void SiteDatabase::update(const QString &identifier, const QString &script)
{
  QtConcurrent::run(this, &SiteDatabase::updateScript, identifier, script);
}

void SiteDatabase::update(const QString &identifierStr, const QString &targetAudience, const QString &script)
{
  const QStringList identifier = identifierStr.split('!');
  if ((identifier.count() >= 2) && !targetAudience.isEmpty() && !script.isEmpty())
  {
    Database::Query query;
    query.prepare("DELETE FROM InternetSites "
                  "WHERE identifier LIKE :identifier");
    query.bindValue(0, identifier[0] + "!%");
    query.exec();

    QString targetAudienceStr;
    foreach (const QString &audience, targetAudience.simplified().split(' '))
      targetAudienceStr += "{" + audience + "} ";

    query.prepare("INSERT OR REPLACE INTO InternetSites "
                  "VALUES (:identifier, :name, :targetAudience, :script)");
    query.bindValue(0, identifierStr.toLower());
    query.bindValue(1, reverseDomain(identifier[0]));
    query.bindValue(2, targetAudienceStr.trimmed());
    query.bindValue(3, script);
    query.exec();
  }
}

void SiteDatabase::remove(const QString &identifierStr)
{
  const QStringList identifier = identifierStr.split('!');
  if (identifier.count() >= 2)
  {
    Database::Query query;
    query.prepare("DELETE FROM InternetSites "
                  "WHERE identifier LIKE :identifier");
    query.bindValue(0, identifier[0] + "!%");
    query.exec();
  }
}

QString SiteDatabase::fullIdentifier(const QString &identifierStr) const
{
  const QStringList identifier = identifierStr.split('!');
  if (!identifier.isEmpty())
  {
    Database::Query query;
    query.prepare("SELECT identifier FROM InternetSites "
                  "WHERE identifier LIKE :identifier");
    query.bindValue(0, identifier[0] + "!%");
    query.exec();
    if (query.next())
      return query.value(0).toString();
  }

  return QString::null;
}

QString SiteDatabase::script(const QString &identifierStr) const
{
  const QStringList identifier = identifierStr.split('!');

  Database::Query query;
  query.prepare("SELECT script FROM InternetSites "
                "WHERE identifier LIKE :identifier");
  query.bindValue(0, identifier.first() + "!%");
  query.exec();
  if (query.next())
    return query.value(0).toString();

  return QString::null;
}

QStringList SiteDatabase::allTargetAudiences(void) const
{
  QMap<QString, QString> result;

  Database::Query query;
  query.prepare("SELECT DISTINCT targetAudience FROM InternetSites");
  query.exec();
  while (query.next())
  foreach (QString targetAudience, query.value(0).toString().simplified().split(' '))
    result.insert(targetAudience.toUpper(), targetAudience.replace("{", "").replace("}", ""));

  return result.values();
}

int SiteDatabase::countSites(const QString &targetAudience) const
{
  Database::Query query;
  query.prepare("SELECT COUNT(*) FROM InternetSites "
                "WHERE targetAudience LIKE :targetAudience");
  query.bindValue(0, "%{" + targetAudience + "}%");
  query.exec();
  if (query.next())
    return query.value(0).toInt();

  return 0;
}

int SiteDatabase::countSites(const QStringList &targetAudiences) const
{
  if (!targetAudiences.isEmpty())
  {
    QString q = "SELECT COUNT(*) FROM InternetSites";
    for (int i=0; i<targetAudiences.count(); i++)
    {
      q += (i == 0) ? " WHERE (" : " OR ";
      q += "(targetAudience LIKE :targetAudience" + QString::number(i) + ")";
    }

    q += ")";

    Database::Query query;
    query.prepare(q);
    for (int i=0; i<targetAudiences.count(); i++)
      query.bindValue(i, "%{" + targetAudiences[i] + "}%");

    query.exec();
    if (query.next())
      return query.value(0).toInt();
  }

  return 0;
}

QStringList SiteDatabase::getSites(const QString &targetAudience, unsigned start, unsigned count) const
{
  QStringList result;

  QString limit;
  if (count > 0)
  {
    limit += " LIMIT " + QString::number(count);
    if (start > 0)
      limit += " OFFSET " + QString::number(start);
  }

  Database::Query query;
  query.prepare("SELECT identifier, name FROM InternetSites "
                "WHERE targetAudience LIKE :targetAudience "
                "ORDER BY name" + limit);
  query.bindValue(0, "%{" + targetAudience + "}%");
  query.exec();
  while (query.next())
    result += query.value(0).toString().split('!').first();

  return result;
}

QStringList SiteDatabase::getSites(const QStringList &targetAudiences, unsigned start, unsigned count) const
{
  QStringList result;

  if (!targetAudiences.isEmpty())
  {
    QString limit;
    if (count > 0)
    {
      limit += " LIMIT " + QString::number(count);
      if (start > 0)
        limit += " OFFSET " + QString::number(start);
    }

    QString q = "SELECT identifier, name FROM InternetSites";
    for (int i=0; i<targetAudiences.count(); i++)
    {
      q += (i == 0) ? " WHERE (" : " OR ";
      q += "(targetAudience LIKE :targetAudience" + QString::number(i) + ")";
    }

    q += ") ORDER BY name" + limit;

    Database::Query query;
    query.prepare(q);
    for (int i=0; i<targetAudiences.count(); i++)
      query.bindValue(i, "%{" + targetAudiences[i] + "}%");

    query.exec();
    while (query.next())
      result += query.value(0).toString().split('!').first();
  }

  return result;
}

QString SiteDatabase::reverseDomain(const QString &domain)
{
  QString reversed;
  foreach (const QString &part, domain.split('.'))
    reversed.prepend(part + '.');

  if (!reversed.isEmpty())
    return reversed.left(reversed.length() - 1);
  else
    return QString::null;
}

void SiteDatabase::customEvent(QEvent *e)
{
  if (e->type() == scriptUpdateEventType)
  {
    const ScriptUpdateEvent * const event = static_cast<const ScriptUpdateEvent *>(e);

    update(event->identifier, event->targetAudience, event->script);
  }
  else
    QObject::customEvent(e);
}

void SiteDatabase::updateScript(const QFileInfo &info)
{
  QFile file(info.absoluteFilePath());
  if (file.open(QFile::ReadOnly))
    updateScript(info.completeBaseName(), QString::fromUtf8(file.readAll()));
}

void SiteDatabase::updateScript(const QString &identifier, const QString &script)
{
  if (!identifier.isEmpty() && !script.isEmpty())
  {
    const QScriptSyntaxCheckResult syntaxCheckResult = QScriptEngine::checkSyntax(script);
    if (syntaxCheckResult.state() == QScriptSyntaxCheckResult::Valid)
    {
      ScriptEngine engine(script);
      if (engine.isCompatible())
      {
        qApp->postEvent(this, new ScriptUpdateEvent(
            identifier,
            engine.targetAudience(),
            script));
      }
    }
    else
    {
      qWarning()
          << identifier
          << ":" << syntaxCheckResult.errorLineNumber() << ":" << syntaxCheckResult.errorColumnNumber()
          << syntaxCheckResult.errorMessage();
    }
  }
}

} } // End of namespaces
