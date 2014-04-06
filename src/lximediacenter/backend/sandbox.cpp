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

#include <LXiCore>
#include <LXiStream>
#include <LXiStreamGui>
#include <QtConcurrent>
#include <cstdlib>
#include <iostream>
#if defined(Q_OS_UNIX)
# include <unistd.h>
# if defined(Q_OS_LINUX)
#  include <sys/syscall.h>
# endif
#elif defined(Q_OS_WIN)
# include <windows.h>
# ifndef PROCESS_MODE_BACKGROUND_BEGIN
#  define PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
# endif
#endif

#if defined(Q_OS_WIN)
// Entry point where control comes on an unhandled exception, so crashing
// sandboxes can terminate cleanly.
static LONG WINAPI topLevelExceptionFilter(PEXCEPTION_POINTERS exceptionInfo)
{
  PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;

  const char *excName = NULL;
  switch(exceptionRecord->ExceptionCode)
  {
  case EXCEPTION_ACCESS_VIOLATION:          excName = "Access Violation";          break;
  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:     excName = "Array Bound Exceeded";      break;
  case EXCEPTION_BREAKPOINT:                excName = "Breakpoint";                break;
  case EXCEPTION_DATATYPE_MISALIGNMENT:     excName = "Datatype Misalignment";     break;
  case EXCEPTION_FLT_DENORMAL_OPERAND:      excName = "Float Denormal Operand";    break;
  case EXCEPTION_FLT_DIVIDE_BY_ZERO:        excName = "Float Divide By Zero";      break;
  case EXCEPTION_FLT_INEXACT_RESULT:        excName = "Float Inexact Result";      break;
  case EXCEPTION_FLT_INVALID_OPERATION:     excName = "Float Invalid Operation";   break;
  case EXCEPTION_FLT_OVERFLOW:              excName = "Float Overflow";            break;
  case EXCEPTION_FLT_STACK_CHECK:           excName = "Float Stack Check";         break;
  case EXCEPTION_FLT_UNDERFLOW:             excName = "Float Underflow";           break;
  case EXCEPTION_GUARD_PAGE:                excName = "Guard Page";                break;
  case EXCEPTION_ILLEGAL_INSTRUCTION:       excName = "Illegal Instruction";       break;
  case EXCEPTION_IN_PAGE_ERROR:             excName = "In Page Error";             break;
  case EXCEPTION_INT_DIVIDE_BY_ZERO:        excName = "Integer Divide By Zero";    break;
  case EXCEPTION_INT_OVERFLOW:              excName = "Integer Overflow";          break;
  case EXCEPTION_INVALID_DISPOSITION:       excName = "Invalid Disposition";       break;
  case EXCEPTION_INVALID_HANDLE:            excName = "Invalid Handle";            break;
  case EXCEPTION_NONCONTINUABLE_EXCEPTION:  excName = "Noncontinuable Exception";  break;
  case EXCEPTION_PRIV_INSTRUCTION:          excName = "Privileged Instruction";    break;
  case EXCEPTION_SINGLE_STEP:               excName = "Single Step";               break;
  case EXCEPTION_STACK_OVERFLOW:            excName = "Stack Overflow";            break;
  case DBG_CONTROL_C:                       excName = "Control+C";                 break;
  case DBG_CONTROL_BREAK:                   excName = "Control+Break";             break;
  case DBG_TERMINATE_THREAD:                excName = "Terminate Thread";          break;
  case DBG_TERMINATE_PROCESS:               excName = "Terminate Process";         break;
  case RPC_S_UNKNOWN_IF:                    excName = "Unknown Interface";         break;
  case RPC_S_SERVER_UNAVAILABLE:            excName = "Server Unavailable";        break;
  default:                                  excName = "";                          break;
  }

  std::cerr
      << "Caught exception " << excName << "(" << ((void *)exceptionRecord->ExceptionCode)
      << ") at location " << exceptionRecord->ExceptionAddress << std::endl;

  ::TerminateProcess(::GetCurrentProcess(), 1);
  ::Sleep(1000);

  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

static void setprio(const char *cmd)
{
  if (strcmp(cmd, "low") == 0)
  {
#if defined(Q_OS_UNIX)
    ::nice(15);
# if defined(Q_OS_LINUX)
    ::syscall(SYS_ioprio_set, 1, ::getpid(), 0x6007);
# endif
#elif defined(Q_OS_WIN)
    ::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    ::SetPriorityClass(::GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
#endif
  }
  else if (strcmp(cmd, "high") == 0)
  {
#if defined(Q_OS_UNIX)
    ::nice(-5);
# if defined(Q_OS_LINUX)
    ::syscall(SYS_ioprio_set, 1, ::getpid(), 0x4002);
# endif
#elif defined(Q_OS_WIN)
    ::SetPriorityClass(::GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
#endif
  }
}

static void probe(const char *cmd)
{
  int mode = -1;
  if      (strncmp(cmd, "format " , 7) == 0) { mode = 0; cmd += 7; }
  else if (strncmp(cmd, "content ", 8) == 0) { mode = 1; cmd += 8; }

  if (mode >= 0)
  {
    QList< QFuture<SMediaInfo> > futures;
    while (cmd && *cmd)
    {
      const char * const next = strchr(cmd, ' ');
      const QByteArray url(cmd, next ? int(next - cmd) : -1);
      if (!url.isEmpty())
      {
        struct T
        {
          static SMediaInfo probe(int mode, QByteArray url)
          {
            SMediaInfo info(QUrl::fromEncoded(url));
            if (!info.isNull())
            {
              if (mode == 0)
                info.probeFormat();
              else
                info.probeContent();
            }

            return info;
          }
        };

        futures += QtConcurrent::run(&T::probe, mode, url);
      }

      cmd = next ? (next + 1) : NULL;
    }

    QByteArray data;
    {
      QXmlStreamWriter writer(&data);
      writer.setAutoFormatting(false);
      writer.writeStartElement("nodes");

      foreach (const QFuture<SMediaInfo> &future, futures)
        future.result().serialize(writer);

      writer.writeEndElement();
    }

    std::cout << data.data() << std::endl;
    return;
  }

  std::cout << "#FAILED" << std::endl;
}

static void thumb(const char *cmd)
{
  const char * const path = strchr(cmd, ' ');
  if (path)
  {
    SMediaInfo fileNode(QUrl::fromEncoded(path + 1));
    if (!fileNode.isNull())
    {
      const QSize size = SSize::fromString(QByteArray(cmd, path - cmd)).size();
      const SVideoBuffer thumbnail = fileNode.readThumbnail(size);
      if (!thumbnail.isNull())
      {
        QBuffer buffer;
        if (buffer.open(QBuffer::WriteOnly))
        if (SImage(thumbnail, true).save(&buffer, "png"))
        {
          std::cout << buffer.data().toBase64().data() << std::endl;
          return;
        }
      }
    }
  }

  std::cout << "#FAILED" << std::endl;
}

int sandbox()
{
  static const int timeout = 30000;

#if defined(Q_OS_WIN)
  if (!::IsDebuggerPresent())
    ::SetUnhandledExceptionFilter(&topLevelExceptionFilter);
#endif

  // Watchdog thread prevents processes hanging around when parent has crashed.
  struct T : QThread
  {
    QAtomicInt active;
    QSemaphore sem;
    virtual void run()
    {
      while (active.deref())
      if (sem.tryAcquire(1, timeout))
        return;

      exit(1);
    }
  } t;

  t.active = 2;
  t.start();

  char line[65536]; // Keep length in sync with maxlen in mediadatabase.cpp
  while (std::cin.getline(&(line[0]), sizeof(line)))
  {
    t.active = 2;

    if      (strncmp(&(line[0]), "probe "   , 6 ) == 0) probe(&(line[6]));
    else if (strncmp(&(line[0]), "thumb "   , 6 ) == 0) thumb(&(line[6]));
    else if (strncmp(&(line[0]), "setprio " , 8 ) == 0) setprio(&(line[8]));
    else if ( strcmp(&(line[0]), "quit"         ) == 0) break;

    t.active = 2;
  }

  t.sem.release();
  t.wait();

  return 0;
}
