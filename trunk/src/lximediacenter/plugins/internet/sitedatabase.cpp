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
  inline ScriptUpdateEvent(const QString &name, const QString &audience, const QString &script)
    : QEvent(scriptUpdateEventType), name(name), audience(audience), script(script)
  {
  }

public:
  const QString                 name;
  const QString                 audience;
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
  QDir dir(":/internet/sites/");
  foreach (const QString &fileName, dir.entryList(QDir::Files))
    loadFutures += QtConcurrent::run(this, &SiteDatabase::readScript, dir.absoluteFilePath(fileName));
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

  foreach (const QString &audience, audiences)
  {
    QMap<QString, QStringList>::ConstIterator i = this->audiences.find(audience);
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
  const bool returnAll = count == 0;
  QStringList result;

  unsigned n = 0;
  foreach (const QString &audience, audiences)
  {
    QMap<QString, QStringList>::ConstIterator i = this->audiences.find(audience);
    if (i != this->audiences.end())
    {
      if (i->count() > int(start))
      {
        for (unsigned j=start; (int(j)<i->count()) && (returnAll || (n<count)); j++, n++)
          result.append((*i)[j]);

        start = 0;
      }
      else
        start -= i->count();
    }
  }

  return result;
}

QString SiteDatabase::getScript(const QString &name) const
{
  QMap<QString, QString>::ConstIterator i = scripts.find(name);
  if (i != scripts.end())
    return *i;

  return QString::null;
}

void SiteDatabase::customEvent(QEvent *e)
{
  if (e->type() == scriptUpdateEventType)
  {
    const ScriptUpdateEvent * const event = static_cast<const ScriptUpdateEvent *>(e);

    addScript(event->name, event->audience, event->script);
  }
  else
    QObject::customEvent(e);
}

void SiteDatabase::readScript(const QString &fileName)
{
  QFile file(fileName);
  if (file.open(QFile::ReadOnly))
  {
    const QString script = QString::fromUtf8(file.readAll());

    const QScriptSyntaxCheckResult syntaxCheckResult = QScriptEngine::checkSyntax(script);
    if (syntaxCheckResult.state() == QScriptSyntaxCheckResult::Valid)
    {
      ScriptEngine engine(script);
      if (engine.isCompatible())
      {
        qApp->postEvent(this, new ScriptUpdateEvent(
            engine.name(),
            engine.audience(),
            script));
      }
    }
    else
    {
      qWarning()
          << fileName
          << ":" << syntaxCheckResult.errorLineNumber() << ":" << syntaxCheckResult.errorColumnNumber()
          << syntaxCheckResult.errorMessage();
    }
  }
}

void SiteDatabase::addScript(const QString &name, const QString &audience, const QString &script)
{
  if (!name.isEmpty() && !audience.isEmpty() && !script.isEmpty())
  {
    scripts.insert(name, script);
    audiences[audience].append(name);
  }
}

} } // End of namespaces
