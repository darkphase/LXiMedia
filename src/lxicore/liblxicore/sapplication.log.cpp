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
#include <cstdio>
#include <ctime>
#include <iostream>

namespace LXiCore {

class SApplicationLog
{
public:
  struct Init : SApplication::Initializer
  {
    virtual void                startup(void)                                   { self = new SApplicationLog(SApplication::instance()->logDir()); }
    virtual void                shutdown(void)                                  { delete self; self = NULL; }
  };

public:
  explicit                      SApplicationLog(const QString &);
                                ~SApplicationLog();

  static void                   logMessage(QtMsgType, const char *);
  static bool                   checkLogFile(const QString &fileName);
  static void                   startLogMsg(const char *type);
  static void                   endLogMsg(void);
  static void                   writeLog(const char *msg);
  static int                    printfLog(const char *format, ...);

public:
  static Init                   init;
  static SApplicationLog      * self;
  static qint64                 pid;
  static FILE                 * logFile;

  QMutex                        mutex;
  QStringList                   allLogFiles, errorLogFiles;
  QString                       logFileName;
  QtMsgHandler                  defaultMsgHandler;
};

SApplicationLog::Init   SApplicationLog::init;
SApplicationLog       * SApplicationLog::self = NULL;
qint64                  SApplicationLog::pid = 0;
FILE                  * SApplicationLog::logFile = NULL;

SApplicationLog::SApplicationLog(const QString &logDir)
  : mutex(QMutex::Recursive)
{
  pid = QCoreApplication::applicationPid();

  if (logDir != "::")
  {
    defaultMsgHandler = qInstallMsgHandler(&SApplicationLog::logMessage);

    if (!logDir.isEmpty())
    {
      foreach (const QFileInfo &file, QDir(logDir).entryInfoList(QStringList() << "*.log"))
      {
        if (checkLogFile(file.absoluteFilePath()))
          errorLogFiles += file.absoluteFilePath();

        if (QFile::exists(file.absoluteFilePath()))
          allLogFiles += file.absoluteFilePath();
      }

      const QString baseName = logDir + "/" +
          QFileInfo(QCoreApplication::applicationFilePath()).baseName() + "-" +
          QDateTime::currentDateTime().toString("yyyyMMddThhmmss");

      logFileName = baseName + ".log";
      for (int i=1; QFile::exists(logFileName); i++)
        logFileName = baseName + "-" + QString::number(i) + ".log";

      logFile = fopen(logFileName.toUtf8(), "wb");

      const QFileInfo appInfo(QCoreApplication::applicationFilePath());
      const QString msg =
          "Opened log file " + logFileName +
          " for " + QCoreApplication::applicationName() +
          " version " + QCoreApplication::applicationVersion() +
          " (" + appInfo.lastModified().toString(Qt::ISODate) + ")";

      startLogMsg("INF");
      writeLog(msg.toUtf8());
      endLogMsg();
    }
  }
}

/*! \fn void SApplication::shutdown(void)
    This waits for all graphs to finish and cleans up any terminals allocated by
    LXiStream.

    \note Do not invoke initialize() again after shutting down.
 */
SApplicationLog::~SApplicationLog(void)
{
  qInstallMsgHandler(defaultMsgHandler);

  if (logFile)
  {
    fclose(logFile);
    logFile = NULL;
  }
}

/*! Checks the log file. If no exceptions or critical messages are found; the
    file is deleted when it is 7 days old.

    \returns true if it contains an exceptions or critical message.
 */
bool SApplicationLog::checkLogFile(const QString &fileName)
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

void SApplicationLog::logMessage(QtMsgType type, const char *msg)
{
  QMutexLocker l(&self->mutex);

  switch (type)
  {
  case QtDebugMsg:    startLogMsg("DBG"); break;
  case QtWarningMsg:  startLogMsg("WRN"); break;
  case QtCriticalMsg: startLogMsg("CRT"); break;
  case QtFatalMsg:    startLogMsg("FTL"); break;
  }

  writeLog(msg);
  //if (type != QtDebugMsg)
  //  logStackTrace(SApplication::currentStackFrame());
  endLogMsg();

  if (type == QtFatalMsg)
    *((int *)0) = 0; // Deliberate crash
}

void SApplicationLog::startLogMsg(const char *type)
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

  writeLog(buffer);
}

void SApplicationLog::endLogMsg(void)
{
  if (logFile)
  {
    fwrite("\t\n", 2, 1, logFile);
    fflush(logFile);
  }

  std::cerr << std::endl;
}

void SApplicationLog::writeLog(const char *msg)
{
  const size_t len = strlen(msg);
  if (logFile)
    fwrite(msg, len, 1, logFile);

  std::cerr << msg;
}

int SApplicationLog::printfLog(const char *format, ...)
{
  char buffer[65536];

  va_list argptr;
  va_start(argptr, format);
  int result = vsprintf(buffer, format, argptr);
  va_end(argptr);

  writeLog(buffer);

  return result;
}


const QStringList & SApplication::allLogFiles(void) const
{
  return SApplicationLog::self->allLogFiles;
}

const QStringList & SApplication::errorLogFiles(void) const
{
  return SApplicationLog::self->errorLogFiles;
}

const QString & SApplication::activeLogFile(void) const
{
  return SApplicationLog::self->logFileName;
}

void SApplication::logLineToActiveLogFile(const QString &line)
{
  QMutexLocker l(&SApplicationLog::self->mutex);

  SApplicationLog::self->writeLog(line.toUtf8());
  SApplicationLog::self->endLogMsg();
}


SApplication::LogFile::LogFile(const QString &fileName, QObject *parent)
  : QFile(fileName, parent)
{
}

SApplication::LogFile::Message SApplication::LogFile::readMessage(void)
{
  Message message;

  bool firstLine = true;
  for (QByteArray line=readLine(); !line.isEmpty(); line=readLine())
  {
    if (firstLine)
    {
      const QList<QByteArray> columns = line.split('\t');
      if (columns.count() >= 4)
      {
        message.date = QDateTime::fromString(columns[0], Qt::ISODate);
        message.type = columns[1];

        QList<QByteArray> pt = columns[2].split(':');
        if (pt.count() == 2)
        {
          message.pid = pt[0].toUInt();
          message.tid = pt[1].toUInt();
        }
        else
          message.pid = message.tid = 0;

        for (int i=3; i<columns.count(); i++)
          message.headline += columns[i] + '\t';

        message.headline = message.headline.simplified();
      }

      firstLine = false;
    }
    else
      message.message += line;

    if (line.endsWith("\t\n"))
      break;
  }

  return message;
}

} // End of namespace
