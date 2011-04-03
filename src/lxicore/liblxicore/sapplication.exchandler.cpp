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

#define LIBCWD_THREAD_SAFE
#define CWDEBUG

#if defined(Q_OS_UNIX)
#include <bfd.h>
#elif defined(Q_OS_WIN)
#include <bfd.h>
#include <windows.h>
#include <imagehlp.h>
#endif
#include <cxxabi.h>

namespace LXiCore {

class SApplicationLog
{
public:
  static void                   startLogMsg(const char *type) __attribute__((nonnull));
  static void                   endLogMsg(void);
  static void                   writeLog(const char *msg) __attribute__((nonnull));
  static int                    printfLog(const char *format, ...) __attribute__((nonnull));
};

class SApplicationExcHandler
{
public:
  struct Module
  {
#ifdef Q_OS_WIN
    HMODULE                       handle;
    HANDLE                        process;
#endif
    bfd                         * abfd;
    asymbol                    ** syms;
    unsigned                      symcount;
  };

  struct Symbol
  {
    char                        name[256];
    char                        fileName[256];
    unsigned                    lineNumber;
  };

public:
  static void                   initialize(void);

  static void                   logStackFrame(void *instructionPointer, void *stackPointer, void *framePointer);
  static void                 * getModuleBase(void *address);
  static Symbol                 getSymbolFromAddress(const Module &module, void *address);
  static bool                   demangleSymbolName(const char *name, char *demangledName, size_t size);

#ifdef Q_OS_WIN
  static LONG WINAPI            topLevelExceptionFilter(PEXCEPTION_POINTERS exceptionInfo);
#endif

public:
  static SApplicationExcHandler * self;
};

void SApplicationExcHandler::initialize(void)
{
#ifdef Q_OS_WIN
  if (!::IsDebuggerPresent())
    ::SetUnhandledExceptionFilter(&SApplicationExcHandler::topLevelExceptionFilter);
#endif
}

void SApplicationExcHandler::logStackFrame(void *instructionPointer, void *stackPointer, void *framePointer)
{
#ifdef Q_OS_WIN
  STACKFRAME stackFrame;
  memset(&stackFrame, 0, sizeof(stackFrame));
  stackFrame.AddrPC.Offset = (DWORD)instructionPointer;
  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrStack.Offset = (DWORD)stackPointer;
  stackFrame.AddrStack.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Offset = (DWORD)framePointer;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrReturn.Mode = AddrModeFlat;

  Module module;
  module.handle = NULL;
  module.process = ::GetCurrentProcess();
  module.abfd = NULL;
  module.syms = NULL;
  module.symcount = 0;

  if (::ReadProcessMemory(module.process,
                          (LPCVOID)(stackFrame.AddrFrame.Offset + sizeof(void *)),
                          &stackFrame.AddrReturn.Offset,
                          sizeof(stackFrame.AddrReturn.Offset),
                          NULL))
  {
    SApplicationLog::writeLog("\nStack trace:\n");

    while (stackFrame.AddrFrame.Offset != 0)
    {
      HMODULE prevModule = module.handle;
      if (stackFrame.AddrPC.Offset)
      if ((module.handle = (HMODULE)getModuleBase((void *)stackFrame.AddrPC.Offset)) != NULL)
      {
        char moduleName[MAX_PATH];
        if (::GetModuleFileNameA(module.handle, moduleName, sizeof(moduleName)))
        {
          SApplicationLog::printfLog("  %s:%08x", moduleName, stackFrame.AddrPC.Offset);

          if (module.handle != prevModule)
          {
            if (module.syms)
            {
              ::GlobalFree(module.syms);
              module.syms = NULL;
              module.symcount = 0;
            }

            if (module.abfd)
              bfd_close(module.abfd);

            if ((module.abfd = bfd_openr(moduleName, NULL)) != NULL)
            if (bfd_check_format(module.abfd, bfd_object))
            if ((bfd_get_file_flags(module.abfd) & HAS_SYMS) != 0)
            {
              // GlobalAlloc is used here because the heap may be corrupted.
              const long storage = bfd_get_symtab_upper_bound(module.abfd);
              if (storage > 0)
              if ((module.syms = (asymbol **)::GlobalAlloc(GMEM_FIXED, storage)) != NULL)
                module.symcount = bfd_canonicalize_symtab(module.abfd, module.syms);
            }
          }

          if (module.abfd && module.syms && module.symcount)
          {
            const Symbol symbol = getSymbolFromAddress(module, (void *)stackFrame.AddrPC.Offset);

            char demangledName[512];
            demangleSymbolName(symbol.name, demangledName, sizeof(demangledName));

            if ((strlen(symbol.fileName) > 0) && (symbol.lineNumber > 0))
              SApplicationLog::printfLog(" %s [%s:%i]\n", demangledName, symbol.fileName, symbol.lineNumber);
            else
              SApplicationLog::printfLog(" %s\n", demangledName);
          }
          else
            SApplicationLog::writeLog("\n");
        }
      }

      stackFrame.AddrPC.Offset = stackFrame.AddrReturn.Offset;
      if (!::ReadProcessMemory(module.process,
                               (LPCVOID)stackFrame.AddrFrame.Offset,
                               &stackFrame.AddrFrame.Offset,
                               sizeof(stackFrame.AddrFrame.Offset),
                               NULL) ||
          !::ReadProcessMemory(module.process,
                               (LPCVOID)(stackFrame.AddrFrame.Offset + sizeof(void *)),
                               &stackFrame.AddrReturn.Offset,
                               sizeof(stackFrame.AddrReturn.Offset),
                               NULL))
      {
        break;
      }

      ::ReadProcessMemory(module.process,
                          (LPCVOID)(stackFrame.AddrFrame.Offset + 2*sizeof(DWORD)),
                          stackFrame.Params,
                          sizeof(stackFrame.Params),
                          NULL);
    }

    if (module.syms)
    {
      ::GlobalFree(module.syms);
      module.syms = NULL;
      module.symcount = 0;
    }

    if (module.abfd)
      bfd_close(module.abfd);
  }
#endif
}

void * SApplicationExcHandler::getModuleBase(void *address)
{
#ifdef Q_OS_WIN
  MEMORY_BASIC_INFORMATION moduleInfo;
  if (::VirtualQuery(address, &moduleInfo, sizeof(moduleInfo)) != 0)
    return moduleInfo.AllocationBase;
  else
#endif
    return NULL;
}

SApplicationExcHandler::Symbol SApplicationExcHandler::getSymbolFromAddress(const Module &module, void *address)
{
  Symbol result;
  result.name[0] = 0;
  result.fileName[0] = 0;
  result.lineNumber = 0;

  struct find_handle
  {
    const Module * module;
    bfd_vma pc;
    const char *filename;
    const char *functionname;
    unsigned int line;
    bool found;

    // Look for an address in a section.  This is called via  bfd_map_over_sections.
    static void find_address_in_section(bfd *abfd, asection *section, PTR data)
    {
      struct find_handle *info = (struct find_handle *) data;
      bfd_vma vma;
      bfd_size_type size;

      if (info->found)
        return;

      if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
        return;

      vma = bfd_get_section_vma(abfd, section);
      size = bfd_get_section_size(section);

      if (info->pc < (vma = bfd_get_section_vma(abfd, section)))
        return;

      if (info->pc >= vma + (size = bfd_get_section_size(section)))
        return;

      info->found = bfd_find_nearest_line(abfd, section, info->module->syms, info->pc - vma, &info->filename, &info->functionname, &info->line);

      long nearest_index = -1;
      for (unsigned i = 0; i < info->module->symcount; i++)
      if (section == info->module->syms[i]->section && (info->pc - vma >= info->module->syms[i]->value))
      if ((nearest_index == -1) || (info->module->syms[i]->value > info->module->syms[nearest_index]->value))
        nearest_index = i;

      if (nearest_index != -1)
      {
        info->found = 1;
        info->functionname = info->module->syms[nearest_index]->name;
      }
    }
  } info;

  info.pc = (bfd_vma)address;
  info.module = &module;
  info.found = FALSE;
  info.line = -1;
  info.functionname = NULL;
  info.filename = NULL;

  if((bfd_get_file_flags(module.abfd) & HAS_SYMS) && (module.symcount > 0))
  {
    bfd_map_over_sections(module.abfd, &find_handle::find_address_in_section, &info);
    if (info.found && info.functionname)
    {
      strncpy(result.name, info.functionname, sizeof(result.name));
      result.name[sizeof(result.name)-1] = 0;

      if (info.filename)
      {
        strncpy(result.fileName, info.filename, sizeof(result.fileName));
        result.fileName[sizeof(result.fileName)-1] = 0;
      }

      result.lineNumber = info.line;
      return result;
    }
  }

#ifdef Q_OS_WIN
  void * const moduleBase = getModuleBase(address);
  if (moduleBase)
  {
    // From the DOS header, find the NT (PE) header
    LONG e_lfanew;
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)moduleBase;
    if (::ReadProcessMemory(module.process, &pDosHdr->e_lfanew, &e_lfanew, sizeof(e_lfanew), NULL))
    {
      IMAGE_NT_HEADERS NtHdr;
      PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((DWORD)moduleBase + (DWORD)e_lfanew);

      if(::ReadProcessMemory(module.process, pNtHdr, &NtHdr, sizeof(IMAGE_NT_HEADERS), NULL))
      {
        PIMAGE_SECTION_HEADER pSection =
            (PIMAGE_SECTION_HEADER)((DWORD)pNtHdr + sizeof(DWORD) +
                                    sizeof(IMAGE_FILE_HEADER) +
                                    NtHdr.FileHeader.SizeOfOptionalHeader);

        // Look for export section
        DWORD dwNearestAddress = 0, dwNearestName;
        for (int i = 0; i < NtHdr.FileHeader.NumberOfSections; i++, pSection++)
        {
          IMAGE_SECTION_HEADER Section;
          if (!::ReadProcessMemory(module.process, pSection, &Section, sizeof(IMAGE_SECTION_HEADER), NULL))
            continue;

          static const BYTE ExportSectionName[IMAGE_SIZEOF_SHORT_NAME] = {'.', 'e', 'd', 'a', 't', 'a', '\0', '\0'};
          PIMAGE_EXPORT_DIRECTORY pExportDir = NULL;
          if(memcmp(Section.Name, ExportSectionName, IMAGE_SIZEOF_SHORT_NAME) == 0)
            pExportDir = (PIMAGE_EXPORT_DIRECTORY) Section.VirtualAddress;
          else if ((NtHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress >= Section.VirtualAddress) && (NtHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress < (Section.VirtualAddress + Section.SizeOfRawData)))
            pExportDir = (PIMAGE_EXPORT_DIRECTORY) NtHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

          if(pExportDir)
          {
            IMAGE_EXPORT_DIRECTORY ExportDir;

            if (!::ReadProcessMemory(module.process, (PVOID)((DWORD)moduleBase + (DWORD)pExportDir), &ExportDir, sizeof(IMAGE_EXPORT_DIRECTORY), NULL))
              continue;

            // GlobalAlloc is used here because the heap may be corrupted.
            DWORD size = ExportDir.NumberOfFunctions * sizeof(PDWORD);
            PDWORD *AddressOfFunctions = (PDWORD*)::GlobalAlloc(GMEM_FIXED, size);

            if (::ReadProcessMemory(module.process, (PVOID)((DWORD)moduleBase + (DWORD)ExportDir.AddressOfFunctions), AddressOfFunctions, size, NULL))
            for (DWORD j = 0; j < ExportDir.NumberOfFunctions; ++j)
            {
              DWORD pFunction = (DWORD)moduleBase + (DWORD)AddressOfFunctions[j];
              //ReadProcessMemory(module.process, (DWORD)moduleBase + (DWORD) (&ExportDir.AddressOfFunctions[j]), &pFunction, sizeof(pFunction), NULL);

              if ((pFunction <= (DWORD)address) && (pFunction > dwNearestAddress))
              {
                dwNearestAddress = pFunction;

                if (!::ReadProcessMemory(module.process, (PVOID)((DWORD)moduleBase + (DWORD)(ExportDir.AddressOfNames + j * 4)), &dwNearestName, sizeof(dwNearestName), NULL))
                  continue;

                dwNearestName = (DWORD)moduleBase + dwNearestName;
              }
            }

            ::GlobalFree(AddressOfFunctions);
          }
        }

        if (dwNearestAddress)
        if (::ReadProcessMemory(module.process, (PVOID)dwNearestName, result.name, sizeof(result.name), NULL))
        {
          result.name[sizeof(result.name)-1] = 0;
          return result;
        }
      }
    }
  }
#endif

  return result;
}

bool SApplicationExcHandler::demangleSymbolName(const char *name, char *demangledName, size_t size)
{
  const char *clean = name;
  while ((clean[0] == '_') && (clean[1] == '_'))
      clean++;

  if (abi::__cxa_demangle(clean, demangledName, &size, NULL) == NULL)
  {
    strncpy(demangledName, name, size);
    demangledName[size-1] = 0;
    return false;
  }
  else
    return true;
}

#ifdef Q_OS_WIN
// Entry point where control comes on an unhandled exception
LONG WINAPI SApplicationExcHandler::topLevelExceptionFilter(PEXCEPTION_POINTERS exceptionInfo)
{
  static bool recursed = false;
  if (!recursed)
  {
    recursed = true;

    UINT oldErrorMode =
        ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

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

    char applicationName[MAX_PATH] = "Application";
    ::GetModuleFileNameA(NULL, applicationName, sizeof(applicationName));

    char moduleName[MAX_PATH] = "";
    void * const moduleBase = getModuleBase(exceptionRecord->ExceptionAddress);
    if (moduleBase)
      ::GetModuleFileNameA((HINSTANCE)moduleBase, moduleName, sizeof(moduleName));

    SApplicationLog::startLogMsg("EXC");
    SApplicationLog::printfLog(
        "%s caused exception %s(%08x) at location %08x %s",
        moduleName,
        excName,
        exceptionRecord->ExceptionCode,
        exceptionRecord->ExceptionAddress,
        moduleName);

    // If the exception was an access violation, print out some additional information, to the error log and the debugger.
    if ((exceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) &&
        (exceptionRecord->NumberParameters >= 2))
    {
      SApplicationLog::printfLog(
          " while %s %08x",
          exceptionRecord->ExceptionInformation[0] ? "writing to" : "reading from",
          exceptionRecord->ExceptionInformation[1]);
    }

    PCONTEXT context = exceptionInfo->ContextRecord;

    // Show the registers
    SApplicationLog::writeLog("\nRegisters:\n");
    if (context->ContextFlags & CONTEXT_INTEGER)
    {
      SApplicationLog::printfLog(
          "  eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
          context->Eax, context->Ebx, context->Ecx,
          context->Edx, context->Esi, context->Edi);
    }

    if (context->ContextFlags & CONTEXT_CONTROL)
    {
      SApplicationLog::printfLog(
          "  eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx %s %s %s %s %s %s %s %s %s %s\n",
          context->Eip, context->Esp, context->Ebp,
          (context->EFlags >> 12) & 3,	//  IOPL level value
          context->EFlags & 0x00100000 ? "vip" : "   ",	//  VIP (virtual interrupt pending)
          context->EFlags & 0x00080000 ? "vif" : "   ",	//  VIF (virtual interrupt flag)
          context->EFlags & 0x00000800 ? "ov" : "nv",	//  VIF (virtual interrupt flag)
          context->EFlags & 0x00000400 ? "dn" : "up",	//  OF (overflow flag)
          context->EFlags & 0x00000200 ? "ei" : "di",	//  IF (interrupt enable flag)
          context->EFlags & 0x00000080 ? "ng" : "pl",	//  SF (sign flag)
          context->EFlags & 0x00000040 ? "zr" : "nz",	//  ZF (zero flag)
          context->EFlags & 0x00000010 ? "ac" : "na",	//  AF (aux carry flag)
          context->EFlags & 0x00000004 ? "po" : "pe",	//  PF (parity flag)
          context->EFlags & 0x00000001 ? "cy" : "nc");	//  CF (carry flag)
    }

    if (context->ContextFlags & CONTEXT_SEGMENTS)
    {
      SApplicationLog::printfLog(
          "  cs=%04lx  ss=%04lx  ds=%04lx  es=%04lx  fs=%04lx  gs=%04lx\n",
          context->SegCs, context->SegSs, context->SegDs,
          context->SegEs, context->SegFs, context->SegGs);
    }

    logStackFrame((void *)context->Eip, (void *)context->Esp, (void *)context->Ebp);

    SApplicationLog::endLogMsg();

    ::SetErrorMode(oldErrorMode);
  }

  ::TerminateProcess(::GetCurrentProcess(), 1);
  ::Sleep(1000);

  return EXCEPTION_CONTINUE_SEARCH;
}
#endif


void SApplication::installExcpetionHandler(void)
{
  SApplicationExcHandler::initialize();
}

void SApplication::logStackFrame(const StackFrame &stackFrame)
{
  return SApplicationExcHandler::logStackFrame(NULL, stackFrame.stackPointer, stackFrame.framePointer);
}

} // End of namespace
