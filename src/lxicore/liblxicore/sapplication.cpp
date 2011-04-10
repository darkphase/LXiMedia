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

#include "sapplication.h"
#include "smodule.h"
#if defined(Q_OS_WIN)
#include <windows.h>
#endif
#include <cstdio>

namespace LXiCore {

struct SApplication::Data
{
  QString                       logDir;
  QStringList                   moduleFilter;
  QList< QPair<QPluginLoader *, SModule *> > modules;
};

SApplication::Initializer * SApplication::initializers = NULL;
SApplication              * SApplication::self = NULL;

/*! Initializes all LXiMedia components.

    \param logDir           The directory where to store log files, no log files
                            are written if this is an empty string.
    \param parent           The parent QObject.
 */
SApplication::SApplication(const QString &logDir, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  if (self != NULL)
    qFatal("Only one instance of the SApplication class is allowed.");

  d->logDir = logDir;

  self = this;

  for (Initializer *i = initializers; i; i = i->next)
    i->startup();

  if (!d->moduleFilter.isEmpty())
  {
    // And now load the plugins
    QStringList paths;
#ifndef Q_OS_WIN
    paths += QCoreApplication::applicationDirPath() + "/lximedia/";
    paths += "/usr/lib/lximedia/";
    paths += "/usr/local/lib/lximedia/";
#else
    const QByteArray myDll = "LXiCore" + QByteArray(version()).split('.').first() + ".dll";
    HMODULE myModule = ::GetModuleHandleA(myDll.data());
    char fileName[MAX_PATH];
    if (::GetModuleFileNameA(myModule, fileName, MAX_PATH) > 0)
    {
      QByteArray path = fileName;
      path = path.left(path.lastIndexOf('\\') + 1);
      paths += path + "lximedia\\";
    }
    else
      qCritical("Failed to locate %s", myDll.data());
#endif

    QSet<QString> modules;
    foreach (QDir dir, paths)
#ifndef Q_OS_WIN
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::Files))
#else
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QStringList() << "*.dll", QDir::Files))
#endif
    if (!modules.contains(fileInfo.fileName()))
    {
      const QString filterName = fileInfo.fileName().split('_').first();

      bool load = false;
      foreach (const QString &filter, d->moduleFilter)
      if (filterName.contains(filter, Qt::CaseInsensitive))
      {
        load = true;
        break;
      }

      if (load)
      {
        modules.insert(fileInfo.fileName());

        QPluginLoader * const loader = new QPluginLoader(fileInfo.absoluteFilePath());
        SModule * const module = qobject_cast<SModule *>(loader->instance());
        if (module)
        {
          if (loadModule(module, loader))
          {
            qDebug() << "Loaded" << fileInfo.absoluteFilePath();
            continue;
          }
        }
        else
        {
          qWarning() << "Could not load" << fileInfo.absoluteFilePath();

          const QString error = loader->errorString();
          if (!error.isEmpty())
            qDebug() << error;
        }

        loader->unload();
        delete loader;
      }
    }
  }
}

/*! \fn void SApplication::shutdown(void)
    This waits for all graphs to finish and cleans up any terminals allocated by
    LXiStream.

    \note Do not invoke initialize() again after shutting down.
 */
SApplication::~SApplication(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

  for (Initializer *i = initializers; i; i = i->next)
    i->shutdown();

  foreach (SFactory *factory, factories())
    factory->clear();

  while (!d->modules.isEmpty())
  {
    QPair<QPluginLoader *, SModule *> i = d->modules.takeLast();

    i.second->unload();
    if (i.first)
    {
      i.first->unload();
      delete i.first;
    }
    else
      delete i.second;
  }

  self = NULL;

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

/*! Returns a name string for LXiMedia.
 */
const char * SApplication::name(void)
{
  return "LXiMedia";
}

/*! Returns the version identifier for the active build of LXiStream.
 */
const char * SApplication::version(void)
{
  return
#include "_version.h"
      ;
}

SApplication  * SApplication::instance(void)
{
  return self;
}

void SApplication::addModuleFilter(const QString &filter)
{
  d->moduleFilter.append(filter);
}

bool SApplication::loadModule(SModule *module, QPluginLoader *loader)
{
  Q_ASSERT(QThread::currentThread() == thread());

  if (module->registerClasses())
  {
    d->modules.append(qMakePair(loader, module));
    return true;
  }
  else
    return false;
}

/*! Returns the about text with minimal XHTML markup.
 */
QByteArray SApplication::about(void) const
{
  QByteArray text =
      " <h1>" + QByteArray(name()) + "</h1>\n"
      " Version: " + QByteArray(version()) + "<br />\n"
      " Website: <a href=\"http://lximedia.sourceforge.net/\">lximedia.sourceforge.net</a><br />\n"
      " <br />\n"
      " <b>Copyright &copy; 2009-2010 A.J. Admiraal. All rights reserved.</b><br />\n"
      " <br />\n"
      " This program is free software; you can redistribute it and/or modify it\n"
      " under the terms of the GNU General Public License version 2 as published\n"
      " by the Free Software Foundation.<br />\n"
      " <br />\n"
      " This program is distributed in the hope that it will be useful, but WITHOUT\n"
      " ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
      " FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br />\n"
      " <br />\n";

  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
    text += i->second->about();

  return text;
}

const QString & SApplication::logDir(void) const
{
  return d->logDir;
}

SApplication * SApplication::createForQTest(QObject *parent)
{
  return new SApplication(parent);
}

SApplication::SApplication(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  if (self != NULL)
    qFatal("Only one instance of the SApplication class is allowed.");

  d->logDir = "::";

  self = this;

  for (Initializer *i = initializers; i; i = i->next)
    i->startup();
}

QList<SFactory *> & SApplication::factories(void)
{
  Q_ASSERT((self == NULL) || (QThread::currentThread() == self->thread()));

  static QList<SFactory *> l;

  return l;
}


SApplication::Initializer::Initializer(void)
  : next(NULL)
{
  if (initializers == NULL)
  {
    initializers = this;
  }
  else for (Initializer *i = initializers; i; i = i->next)
  if (i->next == NULL)
  {
    i->next = this;
    break;
  }
}

SApplication::Initializer::~Initializer()
{
  while (initializers == this)
    initializers = initializers->next;

  for (Initializer *i = initializers; i; i = i->next)
  if (i->next == this)
    i->next = i->next->next;
}

} // End of namespace


