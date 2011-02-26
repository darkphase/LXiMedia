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

#include "private/exceptionhandler.h"
#include "private/log.h"

namespace LXiStream {
namespace SDebug {


const int           Locker::maxLockTime = 60000;
const char * const  Locker::nullType = "null";
const char * const  Locker::type = "Locker";

void Locker::initialize(void)
{
  mutex();
}

void Locker::attemptLock(void *lck, const char *type, const char *file, unsigned line)
{
  QMutexLocker l(&mutex());

  LockMap::Iterator i = lockMap().find(lck);
  if (i == lockMap().end())
    i = lockMap().insert(lck, ThreadMap());

  QThread * const t = QThread::currentThread();
  ThreadMap::Iterator j = i->find(t);
  if (j == i->end())
    j = i->insert(t, QStack<LockDesc>());

  j->push(LockDesc(Private::ExceptionHandler::currentStackFrame(), file, line, type, false));
}

void Locker::gotLock(void *lck)
{
  QMutexLocker l(&mutex());

  LockMap::Iterator i = lockMap().find(lck);
  if (i != lockMap().end())
  {
    ThreadMap::Iterator j = i->find(QThread::currentThread());
    if (j != i->end())
      j->top().locked = true;
  }
}

void Locker::releasedLock(void *lck)
{
  QMutexLocker l(&mutex());

  LockMap::Iterator i = lockMap().find(lck);
  if (i != lockMap().end())
  {
    ThreadMap::Iterator j = i->find(QThread::currentThread());
    if (j != i->end())
    {
      if (!j->isEmpty())
        j->pop();

      if (j->isEmpty())
        i->erase(j);
    }

    if (i->isEmpty())
      lockMap().erase(i);
  }
}

void Locker::logPotentialDeadlock(void *lck, const char *file, unsigned line)
{
  mutex().lock();
  const LockMap copy = lockMap();
  mutex().unlock();

  QMutexLocker l(&Private::Log::logMutex());

  if (lastLog().isNull() || (qAbs(lastLog().elapsed()) > 600000))
  {
    Private::Log::startLogMsg("EXC");
    Private::Log::printf("Potential deadlock detected while aquiring lock %08x at %s:%u", lck, file, line);
    Private::ExceptionHandler::logStackTrace(Private::ExceptionHandler::currentStackFrame());
    Private::Log::write("\n");

    for (LockMap::ConstIterator i=copy.begin(); i!=copy.end(); i++)
    {
      Private::Log::printf("======== Lock %08x is accessed by threads:\n", i.key());

      for (ThreadMap::ConstIterator j=i->begin(); j!=i->end(); j++)
      for (int k=j->count()-1; k>=0; k--)
      {
        Private::Log::printf("Thread %08x %s the lock using a %s at %s:%u.",
                    j.key(),
                    j->at(k).locked ? "has acquired" : "is waiting for",
                    j->at(k).type,
                    j->at(k).file, j->at(k).line);

        Private::ExceptionHandler::logStackTrace(j->at(k).stackFrame);
        Private::Log::write("\n");
      }
    }

    Private::Log::endLogMsg();
  }

  lastLog().start();
}

QMutex & Locker::mutex(void)
{
  static QMutex m;

  return m;
}

Locker::LockMap & Locker::lockMap(void)
{
  static LockMap l;

  return l;
}

QTime & Locker::lastLog(void)
{
  static QTime t;

  return t;
}


LogFile::LogFile(const QString &fileName, QObject *parent)
    : QFile(fileName, parent)
{
}

LogFile::~LogFile()
{
}

LogFile::Message LogFile::readMessage(void)
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

QStringList LogFile::errorLogFiles(void)
{
  return Private::Log::errorLogFiles();
}

QStringList LogFile::allLogFiles(void)
{
  return Private::Log::allLogFiles();
}

QString LogFile::activeLogFile(void)
{
  return Private::Log::logFileName();
}

} } // End of namespaces
