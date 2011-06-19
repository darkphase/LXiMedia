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
#include "sfactory.h"
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

  QMutex                        profileMutex;
  QFile                       * profileFile;
  QMap<QThread *, int>          profileThreadMap;
  QTime                         profileTimer;
  int                           profileWidth;
  static const int              profileLineHeight = 20;
  static const int              profileSecWidth = 20000;
  static const int              profileMinTaskWidth = 20;
};

SApplication::Initializer * SApplication::initializers = NULL;
SApplication              * SApplication::self = NULL;

/*! Initializes all LXiMedia components.

    \note Only one active instance is allowed at any time.

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
  d->profileFile = NULL;
  d->profileWidth = 0;

  self = this;

  for (Initializer *i = initializers; i; i = i->next)
    i->startup();

  if (!d->moduleFilter.isEmpty())
  {
    // And now load the plugins
    QSet<QString> modules;
    foreach (QDir dir, pluginPaths())
#if defined(Q_OS_UNIX)
# if defined(Q_OS_MACX)
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QStringList() << "*.dylib", QDir::Files))
# else
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QStringList() << "*.so", QDir::Files))
# endif
#elif defined(Q_OS_WIN)
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

/*! Uninitializes all LXiMedia components.

    \note Only one active instance is allowed at any time.
 */
SApplication::~SApplication(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

  disableProfiling();

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

/*! Returns a pointer to the active SApplication instance or NULL if none
    exists.

    \note Only one active instance is allowed at any time.
 */
SApplication  * SApplication::instance(void)
{
  return self;
}

/*! Returns the paths that are searched for plugins.
 */
QStringList SApplication::pluginPaths(void)
{
  QStringList result;

#if defined(Q_OS_UNIX)
  const QByteArray majorVersion = QByteArray(version()).split('.').first();

  foreach (const QString &path, qApp->libraryPaths())
    result += path + "/lximedia" + majorVersion + "/";
#elif defined(Q_OS_WIN)
  const QByteArray myDll = "LXiCore.dll";
  HMODULE myModule = ::GetModuleHandleA(myDll.data());
  char fileName[MAX_PATH];
  if (::GetModuleFileNameA(myModule, fileName, MAX_PATH) > 0)
  {
    QByteArray path = fileName;
    path = path.left(path.lastIndexOf('\\') + 1);
    result += path + "lximedia\\";
  }
  else
    qCritical("Failed to locate %s", myDll.data());
#else
# error Not implemented
#endif

  return result;
}

/*! Adds a module filter string whoich is use to determine which of the plugins
    in the plugin directory is actually loaded.

    \note This method should rarely be useful.
 */
void SApplication::addModuleFilter(const QString &filter)
{
  d->moduleFilter.append(filter);
}

/*! Loads a module.

    \note This method should rarely be useful.
 */
bool SApplication::loadModule(const QString &name)
{
  Q_ASSERT(QThread::currentThread() == thread());

  const QString filename =
#if defined(Q_OS_UNIX)
# if defined(Q_OS_MACX)
      "lib" + name + ".dylib";
# else
      "lib" + name + ".so";
# endif
#elif defined(Q_OS_WIN)
      name + ".dll";
#endif

  foreach (QDir dir, pluginPaths())
  if (dir.exists(filename))
  {
    QPluginLoader * const loader = new QPluginLoader(dir.absoluteFilePath(filename));
    SModule * const module = qobject_cast<SModule *>(loader->instance());
    if (module)
    if (loadModule(module, loader))
      return true;

    loader->unload();
    delete loader;
  }

  return false;
}

/*! Loads a module.

    \note This method should rarely be useful.
 */
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
      "<h1>About " + QByteArray(name()) + "</h1>\n"
      "Version: " + QByteArray(version()) + "<br />\n"
      "Website: <a href=\"http://lximedia.sourceforge.net/\">lximedia.sourceforge.net</a><br />\n"
      "<br />\n"
      "<b>Copyright &copy; 2009-2011 A.J. Admiraal. All rights reserved.</b><br />\n"
      "<br />\n"
      "This program is free software; you can redistribute it and/or modify it\n"
      "under the terms of the GNU General Public License version 2 as published\n"
      "by the Free Software Foundation.<br />\n"
      "<br />\n"
      "This program is distributed in the hope that it will be useful, but WITHOUT\n"
      "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
      "FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n";

  QByteArray plugins;
  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
  if (i->first && i->second)
    plugins += "<li title=\"" + i->first->fileName().toUtf8() + "\">" + i->second->about() + "</li>\n";

  if (!plugins.isEmpty())
    text += "<h2>Plugins loaded</h2>\n<ul>\n" + plugins + "</ul>\n";

  text +=
      "<h2>Third-party libraries used</h2>\n"
      "<h3>Qt</h3>\n"
      "Versions: " + QByteArray(qVersion()) + " (linked), " QT_VERSION_STR " (built)<br />\n"
      "Website: <a href=\"http://qt.nokia.com/\">qt.nokia.com</a><br />\n"
      "<br />\n"
      "<b>Copyright &copy; 2011 Nokia Corporation and/or its subsidiary(-ies).</b><br />\n"
      "<br />\n"
      "Used under the terms of the GNU Lesser General Public License version 2.1\n"
      "as published by the Free Software Foundation.\n";

  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
  if (i->second)
    text += i->second->licenses();

  return text;
}

/*! Returns the directory containing the log files or an emty string of not
    applicable.
 */
const QString & SApplication::logDir(void) const
{
  return d->logDir;
}

bool SApplication::enableProfiling(const QString &fileName)
{
  disableProfiling();

  d->profileFile = new QFile(fileName);
  if (!d->profileFile->open(QFile::WriteOnly))
  {
    delete d->profileFile;
    d->profileFile = NULL;
    return false;
  }

  // Reserve space for SVG header
  d->profileFile->write("<svg>" + QByteArray(250, ' ') + '\n');

  d->profileTimer.start();
  d->profileWidth = 0;
  profileTask(-1, -1, "Start");

  return true;
}

void SApplication::disableProfiling(void)
{
  if (d->profileFile)
  {
    // Write SVG trailer
    d->profileFile->write("</svg>\n");

    // Write SVG header
    d->profileFile->seek(0);
    d->profileFile->write(
        "<!-- Trace file created by LXiStream -->\n"
        "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
             "xmlns=\"http://www.w3.org/2000/svg\" "
             "version=\"1.1\" id=\"svg2\" "
             "width=\"" + QByteArray::number(d->profileWidth + (d->profileSecWidth / 10)) + "\" "
             "height=\"" + QByteArray::number(d->profileThreadMap.count() * d->profileLineHeight) + "\">");

    delete d->profileFile;
    d->profileFile = NULL;

    d->profileThreadMap.clear();
  }
}

int SApplication::profileTimeStamp(void)
{
  if (d->profileFile)
  {
    QMutexLocker l(&d->profileMutex);

    return d->profileTimer.elapsed();
  }
  else
    return 0;
}

void SApplication::profileTask(int startTime, int stopTime, const QByteArray &taskName)
{
  if (d->profileFile)
  {
    QMutexLocker l(&d->profileMutex);

    QMap<QThread *, int>::Iterator threadId = d->profileThreadMap.find(QThread::currentThread());
    if (threadId == d->profileThreadMap.end())
    {
      threadId = d->profileThreadMap.insert(QThread::currentThread(), d->profileThreadMap.count());

      d->profileFile->write(
          "<text x=\"0\" "
                "y=\"" + QByteArray::number((*threadId * d->profileLineHeight) + 10) + "\" "
                "style=\"font-size:8px\">"
            "Thread " + QByteArray::number(*threadId) + "</text>\n");
    }

    const int duration = stopTime - startTime;
    const int taskStart = (startTime * d->profileSecWidth / 1000) + 40;
    const int taskWidth = qMax(1, duration * d->profileSecWidth / 1000);

    d->profileWidth = qMax(d->profileWidth, taskStart + taskWidth);

    d->profileFile->write(
        "<rect x=\"" + QByteArray::number(taskStart) + "\" "
              "y=\"" + QByteArray::number(*threadId * d->profileLineHeight) + "\" "
              "width=\"" + QByteArray::number(taskWidth) + "\" "
              "height=\"" + QByteArray::number(d->profileLineHeight) + "\" "
              "style=\"fill:#E0E0FF;stroke:#000000;stroke-width:1\" />\n");

    if (taskWidth >= d->profileMinTaskWidth)
    {
      QByteArray xmlTaskName = taskName;
      const int par = xmlTaskName.indexOf('(');
      if (par > 0)
      {
        xmlTaskName = xmlTaskName.left(par);

        const int spc = xmlTaskName.lastIndexOf(' ');
        if (spc >= 0)
          xmlTaskName = xmlTaskName.mid(spc + 1);

        int sep = -1;
        for (int i = xmlTaskName.indexOf("::"); i >= 0; )
        {
          const int ns = xmlTaskName.indexOf("::", i + 2);
          if (ns > i)
          {
            sep = i;
            i = ns;
          }
          else
            break;
        }

        if (sep >= 0)
          xmlTaskName = xmlTaskName.mid(sep + 2);
      }

      xmlTaskName.replace("&", "&amp;");
      xmlTaskName.replace("<", "&lt;");

      d->profileFile->write(
          "<text x=\"" + QByteArray::number(taskStart) + "\" "
                "y=\"" + QByteArray::number((*threadId * d->profileLineHeight) + d->profileLineHeight - 7) + "\" "
                "style=\"font-size:6px\">" + QByteArray::number(duration) + " ms</text>\n"
          "<text x=\"" + QByteArray::number(taskStart) + "\" "
                "y=\"" + QByteArray::number((*threadId * d->profileLineHeight) + d->profileLineHeight - 1) + "\" "
                "style=\"font-size:6px\">" + xmlTaskName + "</text>\n");
    }

    d->profileFile->flush();
  }
}

/*! Creates a new SApplication instance for use within a QTest environment. No
    modules will be loaded automatically and they have to be loaded using
    loadModule().
 */
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
  d->profileFile = NULL;
  d->profileWidth = 0;

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


SApplication::Profiler::Profiler(const QByteArray &taskName)
  : taskName(taskName),
    startTime(self->profileTimeStamp())
{
}

SApplication::Profiler::~Profiler()
{
  self->profileTask(startTime, self->profileTimeStamp(), taskName);
}

} // End of namespace
