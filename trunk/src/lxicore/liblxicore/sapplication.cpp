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

#include "sapplication.h"
#include "sfactory.h"
#include "smodule.h"
#if defined(Q_OS_LINUX)
# include <unistd.h>
# include <sys/syscall.h>
#elif defined(Q_OS_WIN)
# include <windows.h>
#endif
#include <iostream>

namespace LXiCore {

struct SApplication::Data
{
  QStringList                   moduleFilter;
  QList< QPair<QPluginLoader *, SModule *> > modules;

  QMutex                        logMutex;
  QtMsgHandler                  defaultMsgHandler;
  QFile                         logFile;

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

SApplication::SApplication(bool useLogFile, const QStringList &skipModules, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  if (self != NULL)
    qFatal("Only one instance of the SApplication class is allowed.");

  if (useLogFile)
  {
#ifdef Q_OS_UNIX
    d->logFile.setFileName("/var/log/" + QFileInfo(qApp->applicationFilePath()).baseName() + ".log");
    if (!d->logFile.open(QFile::ReadWrite))
#endif
    {
      d->logFile.setFileName(QDir::temp().absoluteFilePath(tempFileBase() + "log"));
      d->logFile.open(QFile::ReadWrite);
    }
  }

  d->profileFile = NULL;
  d->profileWidth = 0;

  self = this;

  d->defaultMsgHandler = qInstallMsgHandler(&SApplication::logMessage);

  // Repeat this while the module filter is expanded.
  QSet<QString> modules;
  for (bool repeat = true; repeat; )
  {
    repeat = false;

    for (Initializer *i = initializers; i; i = i->next)
    if (!i->inilialized)
    {
      i->startup();
      i->inilialized = true;
    }

    // And now load the plugins
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
      if (filterName.endsWith(filter, Qt::CaseInsensitive))
      {
        load = true;
        break;
      }

      foreach (const QString &skip, skipModules)
      if (filterName.endsWith(skip, Qt::CaseInsensitive))
      {
        load = false;
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
            repeat = true;
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

SApplication::~SApplication(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

  disableProfiling();

  for (Initializer *i = initializers; i; i = i->next)
  if (i->inilialized)
  {
    i->shutdown();
    i->inilialized = false;
  }

  foreach (SFactory *factory, factories())
    factory->clear();

  while (!d->modules.isEmpty())
  {
    QPair<QPluginLoader *, SModule *> i = d->modules.takeLast();

    i.second->unload();
    // Deliberately not unloading/deleting QPluginLoader as it causes a crash.
    if (i.first == NULL)
      delete i.second;
  }

  self = NULL;

  // Remove log file on successful exit.
  if (d->logFile.isOpen())
    d->logFile.remove();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

const char * SApplication::name(void)
{
  return "LXiMedia";
}

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

QString SApplication::tempFileBase(void)
{
  return
      QFileInfo(qApp->applicationFilePath()).baseName() + '-' +
      QString::number(qApp->applicationPid()) + '.';
}

QStringList SApplication::pluginPaths(void)
{
  QStringList result;

#if defined(Q_OS_UNIX)
  const QByteArray majorVersion = QByteArray(version()).split('.').first();

  if (qApp->applicationFilePath().startsWith("/usr/"))
    qApp->addLibraryPath("/usr/lib");

  if (qApp->applicationFilePath().startsWith("/usr/local/"))
    qApp->addLibraryPath("/usr/local/lib");

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

void SApplication::addModuleFilter(const QString &filter)
{
  d->moduleFilter.append(filter);
}

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

    qDebug() << "Failed to load" << dir.absoluteFilePath(filename) << loader->errorString();

    loader->unload();
    delete loader;
  }

  return false;
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

QMap<QString, SModule *> SApplication::modules(void) const
{
  QMap<QString, SModule *> result;
  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
  if (i->first && i->second)
    result.insert(i->first->fileName(), i->second);

  return result;
}

QByteArray SApplication::about(void) const
{
  QByteArray text =
      " <p>Version: " + QByteArray(version()) + "</p>\n"
      " <p>Website: <a href=\"http://lximedia.sourceforge.net/\">lximedia.sourceforge.net</a></p>\n"
      " <p><b>Copyright &copy; 2009-2012  A.J. Admiraal.</b></p>\n"
      " <p>This program is free software: you can redistribute it and/or modify\n"
      " it under the terms of the GNU General Public License version 3 as\n"
      " published by the Free Software Foundation.</p>\n"
      " <p>This program is distributed in the hope that it will be useful,\n"
      " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
      " GNU General Public License for more details.</p>\n"
      " <p>You should have received a copy of the GNU General Public License\n"
      " along with this program. If not, see &lt;http://www.gnu.org/licenses/&gt;</p>\n";

  QByteArray plugins;
  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
  if (i->first && i->second)
    plugins += "<li title=\"" + i->first->fileName().toUtf8() + "\">" + i->second->about() + "</li>\n";

  if (!plugins.isEmpty())
    text += "<h2>Plugins loaded</h2>\n<ul>\n" + plugins + "</ul>\n";

  text +=
      " <h2>Third-party libraries used</h2>\n"
      " <h3>Qt</h3>\n"
      " <p>Versions: " + QByteArray(qVersion()) + " (linked), " QT_VERSION_STR " (built)</p>\n"
      " <p>Website: <a href=\"http://qt.nokia.com/\">qt.nokia.com</a></p>\n"
      " <p><b>Copyright &copy; 2011 Nokia Corporation and/or its subsidiary(-ies).</b></p>\n"
      " <p>Used under the terms of the GNU General Public License version 3.0\n"
      " as published by the Free Software Foundation.</p>\n";

  QSet<QByteArray> allLicenses;
  for (QList< QPair<QPluginLoader *, SModule *> >::Iterator i=d->modules.begin(); i!=d->modules.end(); i++)
  if (i->second)
  {
    const QByteArray licenses = i->second->licenses();
    if (!allLicenses.contains(licenses))
    {
      text += licenses;
      allLicenses.insert(licenses);
    }
  }

  return text;
}

QByteArray SApplication::log(void)
{
  QMutexLocker l(&d->logMutex);

  if (d->logFile.isOpen())
  if (d->logFile.seek(0))
    return d->logFile.readAll();

  return QByteArray();
}

void SApplication::logLine(const QByteArray &line)
{
  QMutexLocker l(&d->logMutex);

  if (d->logFile.isOpen())
  {
    d->logFile.write(line + "\t\n");
    d->logFile.flush();
  }

  std::cerr << line.data() << std::endl;
}

void SApplication::logMessage(QtMsgType type, const char *msg)
{
  QByteArray message = QDateTime::currentDateTime().toString(Qt::ISODate).toAscii();

  switch (type)
  {
  case QtDebugMsg:    message += "\tDBG"; break;
  case QtWarningMsg:  message += "\tWRN"; break;
  case QtCriticalMsg: message += "\tCRT"; break;
  case QtFatalMsg:    message += "\tFTL"; break;
  }

  message +=
      '\t' + QByteArray::number(QCoreApplication::applicationPid())
#if defined(Q_OS_LINUX)
      + ':' + QByteArray::number(qint64(::syscall(SYS_gettid)))
#elif defined(Q_OS_WIN)
      + ':' + QByteArray::number(quint32(::GetCurrentThreadId()));
#endif
      ;

  message += '\t';
  message += msg;

  self->logLine(message);
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

  qDebug() << "Enabling profiling to file" << fileName;

  // Reserve space for SVG header
  d->profileFile->write("<svg>" + QByteArray(250, ' ') + '\n');

  d->profileTimer.start();
  d->profileWidth = 0;
  profileTask(-1, -1, "Start", TaskType_Blocked);

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

    qDebug() << "Disabled profiling";
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

void SApplication::profileTask(int startTime, int stopTime, const QByteArray &taskName, TaskType taskType)
{
  static const char * const taskFill[4]   = { "C0C0C0", "E0FFE0", "FFE0E0", "E0E0FF" };
  static const char * const taskStroke[4] = { "404040", "008000", "800000", "000080" };

  if (d->profileFile)
  {
    QMutexLocker l(&d->profileMutex);

    if (d->profileFile)
    {
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

      if (taskWidth >= d->profileMinTaskWidth)
      {
        d->profileFile->write(
            "<rect x=\"" + QByteArray::number(taskStart) + "\" "
                  "y=\"" + QByteArray::number(*threadId * d->profileLineHeight) + "\" "
                  "width=\"" + QByteArray::number(taskWidth) + "\" "
                  "height=\"" + QByteArray::number(d->profileLineHeight) + "\" "
                  "style=\"fill:#" + QByteArray(taskFill[taskType]) +
                         ";stroke:#" + QByteArray(taskStroke[taskType]) + ";stroke-width:1\" />\n");

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
//      else
//      {
//        d->profileFile->write(
//            "<!-- " + taskName + " " + QByteArray::number(duration) + " ms -->\n");
//      }

      d->profileFile->flush();

      if (d->profileFile->size() >= 4194304)
        disableProfiling();
    }
  }
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

  d->profileFile = NULL;
  d->profileWidth = 0;

  self = this;

  for (Initializer *i = initializers; i; i = i->next)
  if (!i->inilialized)
  {
    i->startup();
    i->inilialized = true;
  }
}

QList<SFactory *> & SApplication::factories(void)
{
  Q_ASSERT((self == NULL) || (QThread::currentThread() == self->thread()));

  static QList<SFactory *> l;

  return l;
}


SApplication::Initializer::Initializer(void)
  : inilialized(false),
    next(NULL)
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


SApplication::Profiler::Profiler(const char *taskName, TaskType taskType)
  : taskName(taskName),
    waitName(NULL),
    taskType(taskType),
    startTime(self->profileTimeStamp())
{
  if (self->d->profileFile)
    self->profileTask(startTime, startTime, QByteArray(taskName) + "_start", taskType);
}

SApplication::Profiler::Profiler(const char *taskName, const char *waitName, TaskType taskType)
  : taskName(taskName),
    waitName(waitName),
    taskType(taskType),
    startTime(self->profileTimeStamp())
{
  if (self->d->profileFile)
    self->profileTask(startTime, startTime, QByteArray(taskName) + "/" + waitName + "_start", taskType);
}

SApplication::Profiler::~Profiler()
{
  if (self->d->profileFile)
  {
    if (waitName == NULL)
      self->profileTask(startTime, self->profileTimeStamp(), taskName, taskType);
    else
      self->profileTask(startTime, self->profileTimeStamp(), QByteArray(taskName) + "/" + waitName, taskType);
  }
}

} // End of namespace
