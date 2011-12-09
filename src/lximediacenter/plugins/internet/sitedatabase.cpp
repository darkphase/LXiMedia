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
  inline ScriptUpdateEvent(const QString &host, const QString &name, const QString &audience)
    : QEvent(scriptUpdateEventType), host(host), name(name), audience(audience)
  {
    this->host.squeeze();
    this->name.squeeze();
    this->audience.squeeze();
  }

public:
  QString host, name, audience;
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
  : QObject(parent),
    localScriptDir(localScriptDirPath()),
    globalScriptDir(":/internet/sites/")
{
  if (!localScriptDir.exists())
    localScriptDir.mkpath(localScriptDir.absolutePath());

  QSet<QString> hosts;

  foreach (const QString &fileName, localScriptDir.entryList(QStringList("*.js"), QDir::Files))
  {
    QFile file(localScriptDir.absoluteFilePath(fileName));
    if (file.open(QFile::ReadOnly))
    {
      const QString host = QFileInfo(fileName).completeBaseName();
      if (!hosts.contains(host))
      {
        hosts.insert(host);

        loadFutures += QtConcurrent::run(
            this, &SiteDatabase::readScript,
            host, QString::fromUtf8(file.readAll()));
      }
    }
  }

  foreach (const QString &fileName, globalScriptDir.entryList(QStringList("*.js"), QDir::Files))
  {
    QFile file(globalScriptDir.absoluteFilePath(fileName));
    if (file.open(QFile::ReadOnly))
    {
      const QString host = QFileInfo(fileName).completeBaseName();
      if (!hosts.contains(host))
      {
        hosts.insert(host);

        loadFutures += QtConcurrent::run(
            this, &SiteDatabase::readScript,
            QFileInfo(fileName).completeBaseName(),
            QString::fromUtf8(file.readAll()));
      }
    }
  }
}

SiteDatabase::~SiteDatabase()
{
  while (!loadFutures.isEmpty())
    loadFutures.takeFirst().waitForFinished();

  self = NULL;
}

QStringList SiteDatabase::allAudiences(void) const
{
  return audiences.keys();
}

int SiteDatabase::countSites(const QString &audience) const
{
  return countSites(QStringList() << audience);
}

int SiteDatabase::countSites(const QStringList &audiences) const
{
  int result = 0;

  while (!loadFutures.isEmpty())
    loadFutures.takeFirst().waitForFinished();

  foreach (const QString &audience, audiences)
  {
    QMap<QString, QSet<QString> >::ConstIterator i = this->audiences.find(audience);
    if (i != this->audiences.end())
      result += i->count();
  }

  return result;
}

QStringList SiteDatabase::getSites(const QString &audience, unsigned start, unsigned count) const
{
  return getSites(QStringList() << audience, start, count);
}

QStringList SiteDatabase::getSites(const QStringList &audiences, unsigned start, unsigned count) const
{
  while (!loadFutures.isEmpty())
    loadFutures.takeFirst().waitForFinished();

  QMap<QString, QString> sortedItems;
  foreach (const QString &audience, audiences)
  {
    QMap<QString, QSet<QString> >::ConstIterator i = this->audiences.find(audience);
    if (i != this->audiences.end())
    foreach (const QString &host, *i)
      sortedItems.insert(getName(host), host);
  }

  const bool returnAll = count == 0;
  QStringList result;

  const QStringList sortedValues = sortedItems.values();
  for (unsigned i=start, n=0; (int(i)<sortedValues.count()) && (returnAll || (n<count)); i++, n++)
    result.append(sortedValues[i]);

  return result;
}

QString SiteDatabase::getScript(const QString &host) const
{
  QFile localFile(localScriptDir.absoluteFilePath(host + ".js"));
  if (localFile.open(QFile::ReadOnly))
    return QString::fromUtf8(localFile.readAll());

  QFile globalFile(globalScriptDir.absoluteFilePath(host + ".js"));
  if (globalFile.open(QFile::ReadOnly))
    return QString::fromUtf8(globalFile.readAll());

  return QString::null;
}

bool SiteDatabase::updateScript(const QString &host, const QString &script)
{
  QFile file(localScriptDir.absoluteFilePath(host + ".js"));
  if (file.open(QFile::WriteOnly))
  {
    file.write(script.toUtf8());

    if (readScript(host, script))
    {
      removeScript(host);
      qApp->sendPostedEvents(this, scriptUpdateEventType);

      return true;
    }
  }

  return false;
}

bool SiteDatabase::deleteLocalScript(const QString &host)
{
  foreach (const QString &fileName, localScriptDir.entryList(QStringList() << (host + ".js"), QDir::Files))
  if (localScriptDir.remove(fileName))
  {
    removeScript(host);

    QFile file(globalScriptDir.absoluteFilePath(host + ".js"));
    if (file.open(QFile::ReadOnly))
      readScript(host, QString::fromUtf8(file.readAll()));

    qApp->sendPostedEvents(this, scriptUpdateEventType);

    return true;
  }

  return false;
}

bool SiteDatabase::isLocal(const QString &host) const
{
  return localScriptDir.exists(host + ".js");
}

bool SiteDatabase::isGlobal(const QString &host) const
{
  return globalScriptDir.exists(host + ".js");
}

QString SiteDatabase::getHost(const QString &name) const
{
  QMap<QString, QString>::ConstIterator i = hostByName.find(name);
  if (i != hostByName.end())
    return *i;

  return QString::null;
}

QString SiteDatabase::getName(const QString &host) const
{
  QMap<QString, QString>::ConstIterator i = nameByHost.find(host);
  if (i != nameByHost.end())
    return *i;

  return QString::null;
}

void SiteDatabase::customEvent(QEvent *e)
{
  if (e->type() == scriptUpdateEventType)
  {
    const ScriptUpdateEvent * const event = static_cast<const ScriptUpdateEvent *>(e);

    addScript(event->host, event->name, event->audience);
  }
  else
    QObject::customEvent(e);
}

bool SiteDatabase::readScript(const QString &host, const QString &script)
{
  const QScriptSyntaxCheckResult result = QScriptEngine::checkSyntax(script);
  if (result.state() == QScriptSyntaxCheckResult::Valid)
  {
    ScriptEngine engine(script);
    if (engine.isCompatible())
    {
      qApp->postEvent(this, new ScriptUpdateEvent(host, engine.name(), engine.audience()));

      return true;
    }
  }
  else
    qWarning() << host << ":" << result.errorLineNumber() << ": " << result.errorMessage();

  return false;
}

void SiteDatabase::addScript(const QString &host, const QString &name, const QString &audience)
{
  if (!host.isEmpty() && !name.isEmpty() && !audience.isEmpty())
  {
    QString fullName = name;
    if (names.contains(name))
    {
      fullName += " (" + host + ")";
      fullName.squeeze();

      for (QMap<QString, QString>::Iterator i = hostByName.find(name);
           i != hostByName.end();
           i = hostByName.find(name))
      {
        const QString h = *i;
        const QString n = name + " (" + h + ")";

        nameByHost.remove(*i);
        hostByName.erase(i);

        hostByName.insert(n, h);
        nameByHost.insert(h, n);
      }
    }
    else
      names.insert(name);

    hostByName.insert(fullName, host);
    nameByHost.insert(host, fullName);
    audiences[audience].insert(host);
  }
}

void SiteDatabase::removeScript(const QString &host)
{
  QMap<QString, QString>::Iterator iname = nameByHost.find(host);
  if (iname != nameByHost.end())
  {
    names.remove(*iname);

    QMap<QString, QString>::Iterator ihost = hostByName.find(*iname);
    if (ihost != hostByName.end())
      hostByName.erase(ihost);

    if (iname != nameByHost.end())
      nameByHost.erase(iname);

    for (QMap<QString, QSet<QString> >::Iterator i=audiences.begin(); i!=audiences.end(); )
    {
      i->remove(host);
      if (i->isEmpty())
        i = audiences.erase(i);
      else
        i++;
    }
  }
}

QString SiteDatabase::localScriptDirPath(void)
{
  const QFileInfo settingsFile = QSettings().fileName();

  return
      settingsFile.absolutePath() + "/" +
      settingsFile.completeBaseName() + "." +
      Module::pluginName + ".Sites";
}

} } // End of namespaces
