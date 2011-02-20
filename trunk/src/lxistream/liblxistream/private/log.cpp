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

#include "log.h"
#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <iostream>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#include "exceptionhandler.h"
#include "sapplication.h"

// Prevents dependency with zlibg-dev (as Qt already provides zlib).
extern "C"
{
  typedef void * gzFile;
  gzFile gzopen(const char *path, const char *mode);
  int gzwrite (gzFile file, void *buf, unsigned len);
  int gzclose(gzFile file);
}

namespace LXiStream {
namespace Private {


QtMsgHandler                Log::defaultMsgHandler = NULL;
bool                        Log::useFile = false;
bool                        Log::useStderr = false;
qint64                      Log::pid = 0;

void Log::initialize(const QString &preferredLogDir)
{
  pid = QCoreApplication::applicationPid();

  if ((SApplication::instance()->initializeFlags() & SApplication::Initialize_LogToFile) == SApplication::Initialize_LogToFile)
  {
    useFile = true;
    if (!preferredLogDir.isEmpty())
      logDir(preferredLogDir);

    foreach (const QFileInfo &file, QDir(logDir()).entryInfoList(QStringList() << "*.log"))
    if (checkLogFile(file.absoluteFilePath()))
      errorLogFileList() += file.absoluteFilePath();

    const QFileInfo appInfo(QCoreApplication::applicationFilePath());
    const QString msg = "Opened log file for " + QCoreApplication::applicationName() +
                        "(image: " + appInfo.fileName() + " pid: " + QString::number(pid) +
                        ") version " + QCoreApplication::applicationVersion() +
                        "(" +appInfo.lastModified().toString(Qt::ISODate) + ")";

    startLogMsg("INF");
    write(msg.toUtf8());
    endLogMsg();
  }

  if (defaultMsgHandler == NULL)
  {
    defaultMsgHandler = qInstallMsgHandler(&Log::logMessage);
    if (defaultMsgHandler == &Log::logMessage)
      defaultMsgHandler = NULL;
  }

  if ((SApplication::instance()->initializeFlags() & SApplication::Initialize_LogToConsole) == SApplication::Initialize_LogToConsole)
    useStderr = defaultMsgHandler == NULL;

#ifdef Q_OS_UNIX
  useStderr &= ::write(STDERR_FILENO, "", 0) >= 0;
#endif
}

QStringList Log::errorLogFiles(void)
{
  QStringList result;

  foreach (const QString &file, errorLogFileList())
  if (QFile::exists(file))
    result += file;

  return result;
}

QStringList Log::allLogFiles(void)
{
  QStringList result;

  foreach (const QFileInfo &file, QDir(logDir()).entryInfoList(QStringList() << "*.log"))
    result += file.absoluteFilePath();

  return result;
}

void Log::redirectLogMsg(const char *msg)
{
  QMutexLocker l(&logMutex());

  write(msg);
  endLogMsg();
}

/*! Checks the log file. If no exceptions or critical messages are found; the
    file is deleted when it is 7 days old.

    \returns true if it contains an exceptions or critical message.
 */
bool Log::checkLogFile(const QString &fileName)
{
  QFileInfo info(fileName);
  QFile file(fileName);
  if (file.open(QFile::ReadOnly))
  {
    QByteArray msg;
    for (QByteArray line=file.readLine(); !line.isEmpty(); line=file.readLine())
    {
      msg += line;

      if (msg.endsWith("\t\n"))
      {
        QList<QByteArray> msgSplit = msg.split('\t');

        if (msgSplit.count() >= 3)
        if ((msgSplit[1] == "EXC") || (msgSplit[1] == "CRT"))
          return true;

        msg.clear();
      }
    }

    if (info.created().daysTo(QDateTime::currentDateTime()) >= 7)
      file.remove();
    else
      file.close();
  }

  return false;
}

void Log::logMessage(QtMsgType type, const char *msg)
{
  QMutexLocker l(&logMutex());

  switch (type)
  {
  case QtDebugMsg:    startLogMsg("DBG"); break;
  case QtWarningMsg:  startLogMsg("WRN"); break;
  case QtCriticalMsg: startLogMsg("CRT"); break;
  case QtFatalMsg:    startLogMsg("FTL"); break;
  }

  write(msg);
  if (type != QtDebugMsg)
    ExceptionHandler::logStackTrace(ExceptionHandler::currentStackFrame());
  endLogMsg();

  if (defaultMsgHandler)
    defaultMsgHandler(type, msg);

  // In debug mode, all messages except a debug message crash the application so
  // that it can be debugged easily. In release mode only fatal messages crash
  // the application.
  if (type == QtFatalMsg)
    *((int *)0) = 0; // Deliberate crash
}

void Log::startLogMsg(const char *type)
{
  const time_t t = time(NULL);
  struct tm * const b = localtime(&t);

  // Determine thread ID
  static QMap<QThread *, unsigned> threads;
  QThread * const ct = QThread::currentThread();
  QMap<QThread *, unsigned>::Iterator i = threads.find(ct);
  if (i == threads.end())
    i = threads.insert(ct, threads.count() + 1);

  char buffer[128];
  sprintf(buffer, "%4i-%02i-%02iT%02i:%02i:%02i\t%s\t%u:%u\t",
          b->tm_year + 1900, b->tm_mon + 1, b->tm_mday,
          b->tm_hour, b->tm_min, b->tm_sec,
          type, unsigned(pid), *i);

  write(buffer);
}

void Log::endLogMsg(void)
{
  if (useFile)
  {
    fwrite("\t\n", 2, 1, logFile());
    fflush(logFile());
  }

  if (useStderr)
    std::cerr << std::endl;
}

void Log::write(const char *msg)
{
  const size_t len = strlen(msg);
  if (useFile)
    fwrite(msg, len, 1, logFile());

  if (useStderr)
    std::cerr << msg;
}

int Log::printf(const char *format, ...)
{
  char buffer[65536];

  va_list argptr;
  va_start(argptr, format);
  int result = vsprintf(buffer, format, argptr);
  va_end(argptr);

  write(buffer);

  return result;
}

QString & Log::logDir(const QString &preferredLogDir)
{
  static QString logDir = QString::null;

  if (__builtin_expect(!preferredLogDir.isEmpty(), false))
  {
    const QFileInfo prefDir(preferredLogDir);
    if (prefDir.isDir() && prefDir.isWritable())
      logDir = preferredLogDir;
  }

  if (__builtin_expect(logDir.isEmpty(), false))
  {
    const QString name = QFileInfo(QCoreApplication::applicationFilePath()).baseName();

#if defined(Q_OS_UNIX)
    const QFileInfo baseDir("/var/log");
#elif defined(Q_OS_WIN)
    const QFileInfo baseDir(QDir::homePath() + "/Local Settings/Application Data");
#endif

#if defined(Q_OS_UNIX) || defined(Q_OS_WIN)
    if (baseDir.isDir() && baseDir.isWritable())
    {
      QFileInfo fullDir(baseDir.absoluteFilePath() + "/" + name);
      if (!fullDir.exists())
      {
        if (QDir(baseDir.absoluteFilePath()).mkdir(name))
          return logDir = fullDir.absoluteFilePath();
      }
      else if (fullDir.isDir() && fullDir.isWritable())
        return logDir = fullDir.absoluteFilePath();
    }
#endif

    return logDir = QDir::tempPath();
  }

  return logDir;
}

QStringList & Log::errorLogFileList(void)
{
  static QStringList l;

  return l;
}

const QString & Log::logFileName(void)
{
  static QString fileName = QString::null;
  if (__builtin_expect(fileName.isEmpty(), false))
  {
    const QString baseName = logDir() + "/" +
        QFileInfo(QCoreApplication::applicationFilePath()).baseName() + "-" +
        QDateTime::currentDateTime().toString("yyyyMMddThhmmss");

    fileName = baseName + ".log";
    for (int i=1; QFile::exists(fileName); i++)
      fileName = baseName + "-" + QString::number(i) + ".log";
  }

  return fileName;
}

FILE * Log::logFile(void)
{
  static FILE * fd = NULL;
  if (__builtin_expect(fd == NULL, false))
    fd = fopen(logFileName().toUtf8(), "wb");

  return fd;
}

QMutex & Log::logMutex(void)
{
  static QMutex m(QMutex::Recursive);

  return m;
}

QNetworkAccessManager & Log::networkAccessManager(void)
{
  static QNetworkAccessManager * manager = NULL;
  static LogNetworkCleaner * cleaner = NULL;
  if (__builtin_expect(manager == NULL, false))
  {
    manager = new QNetworkAccessManager();
    cleaner = new LogNetworkCleaner();

    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)),
                     cleaner, SLOT(finished(QNetworkReply *)));
  }

  return *manager;
}

void LogNetworkCleaner::finished(QNetworkReply *reply)
{
  reply->deleteLater();
}


} } // End of namespaces
