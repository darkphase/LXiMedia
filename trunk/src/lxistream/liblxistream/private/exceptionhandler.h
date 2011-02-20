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

#ifndef LXDEBUG_EXCEPTIONHANDLER_H
#define LXDEBUG_EXCEPTIONHANDLER_H

#include <QtCore>
#include "sdebug.h"

namespace LXiStream {
namespace Private {


class ExceptionHandler : public SDebug::ExceptionHandler
{
private:
  struct Module;

  struct Symbol
  {
    char                        name[256];
    char                        fileName[256];
    unsigned                    lineNumber;
  };

public:
  static void                   initialize(void);
  static void                   logStackTrace(const StackFrame &);
  inline static StackFrame      currentStackFrame(void);

private:
  static void                   logStackTrace(void *instructionPointer, void *stackPointer, void *framePointer);
  static void                 * getModuleBase(void *address);
  static Symbol                 getSymbolFromAddress(const Module &module, void *address);
  static bool                   demangleSymbolName(const char *name, char *demangledName, size_t size);
};


inline ExceptionHandler::StackFrame ExceptionHandler::currentStackFrame(void)
{
  StackFrame stackFrame;

#if (defined(QT_ARCH_I386) || defined(QT_ARCH_WINDOWS))
  asm volatile("movl %%esp, %0;"
               "movl %%ebp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#elif defined(QT_ARCH_X86_64)
  asm volatile("movq %%rsp, %0;"
               "movq %%rbp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#else
#error "Not implemented"
#endif

  return stackFrame;
}


} } // End of namespaces

#endif
