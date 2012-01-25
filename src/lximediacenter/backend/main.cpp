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
#include <iostream>
#if defined(Q_OS_WIN)
# include <windows.h>
#endif

#include "backend.h"
#include "sandbox.h"

void configApp(void)
{
  qApp->setOrganizationName("LeX-Interactive");
  qApp->setOrganizationDomain("lximedia.sf.net");
  qApp->setApplicationName("LXiMediaCenter");
  qApp->setApplicationVersion(
#include "_version.h"
      );

#if defined(Q_OS_MACX)
  QDir appDir(qApp->applicationDirPath());
  if (appDir.dirName() == "MacOS")
  {
    appDir.cdUp();
    appDir.cd("PlugIns");
    qApp->setLibraryPaths(QStringList() << appDir.absolutePath());
  }
#elif defined(Q_OS_WIN)
  // Do not use the registry on Windows.
  QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
}

#if (defined(Q_OS_LINUX) || defined(Q_OS_WIN))
class Daemon : public SDaemon
{
public:
  inline Daemon(void)
    : SDaemon(name)
  {
  }

  virtual int run(int &argc, char *argv[])
  {
    QCoreApplication app(argc, argv); configApp();
#if !defined(DEBUG_USE_LOCAL_SANDBOX)
    SApplication mediaApp(true, QStringList() << "lxistream" << "lxistreamgui");
#else
    SApplication mediaApp(true);
#endif

    Backend backend;
    backend.start();

    const int exitCode = qApp->exec();

    qDebug() << "Daemon has finished with exit code" << exitCode;
    return exitCode;
  }

  virtual void quit(void)
  {
    qApp->quit();
  }

public:
  static const char name[];
};

#if defined(Q_OS_LINUX)
const char Daemon::name[] = "lximcbackend";
#elif defined(Q_OS_WIN)
const char Daemon::name[] = "LXiMediaCenter Backend";
#endif

#if defined(Q_OS_WIN)
// Entry point where control comes on an unhandled exception, so crashing
// sandboxes can terminate cleanly.
LONG WINAPI topLevelExceptionFilter(PEXCEPTION_POINTERS exceptionInfo)
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

int main(int argc, char *argv[])
{
  if ((argc >= 3) && (strcmp(argv[1], "--sandbox") == 0))
  {
    QCoreApplication app(argc, argv); configApp();
    SApplication mediaApp(false);

#if defined(Q_OS_WIN)
  if (!::IsDebuggerPresent())
    ::SetUnhandledExceptionFilter(&topLevelExceptionFilter);
#endif

    (new Sandbox())->start(argv[2]);

    return qApp->exec();
  }
  else
  {
    Daemon daemon;
    return daemon.main(argc, argv);
  }
}
#elif defined(Q_OS_MACX)
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv); configApp();

  if ((argc >= 3) && (strcmp(argv[1], "--sandbox") == 0))
  {
    SApplication mediaApp(false);

    (new Sandbox())->start(argv[2]);

    return qApp->exec();
  }
  else
  {
#if !defined(DEBUG_USE_LOCAL_SANDBOX)
    SApplication mediaApp(true, QStringList() << "lxistream" << "lxistreamgui");
#else
    SApplication mediaApp(true);
#endif

    int exitCode = 0;
    do
    {
      Backend backend;
      backend.start();

      exitCode = qApp->exec();
    }
    while (exitCode == -1);

    return exitCode;
  }
}
#endif
