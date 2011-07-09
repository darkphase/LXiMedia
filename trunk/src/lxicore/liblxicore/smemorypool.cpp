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

#include "smemorypool.h"
#include "sapplication.h"
#if defined(Q_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

#ifndef QT_NO_DEBUG
#define USE_GUARD_PAGES
#endif

namespace LXiCore {

struct SMemoryPool::Init : SApplication::Initializer
{
  virtual void startup(void)
  {
#if defined(Q_OS_LINUX)
    pageSize = int(::sysconf(_SC_PAGESIZE));
    zeroDev = ::open("/dev/zero", O_RDWR);
#elif defined(Q_OS_WIN)
    ::SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    pageSize = info.dwPageSize;
#else
    pageSize = 64;
#endif

    if (pageSize <= 0)
      pageSize = 8192;

    // Ensure static initializers are called.
    QMutexLocker l(mutex());
    freePool().clear();
    freeQueue().clear();
    allocPool().clear();
  }

  virtual void shutdown(void)
  {
    QMutexLocker l(mutex());

    if (!allocPool().isEmpty())
    {
      qWarning() << "SMemoryPool: Not all memory was freed; leaked" << allocPool().count() << "buffers.";
      foreach (const Block &block, freePool().values())
        freePages(block.addr, block.size);
    }

    foreach (const Block &block, freePool().values())
      freePages(block.addr, block.size);

    freePool().clear();
    freeQueue().clear();
    allocPool().clear();

#if defined(Q_OS_LINUX)
    ::close(zeroDev);
    zeroDev = 0;
#endif
    pageSize = 0;
  }
};

SMemoryPool::Init SMemoryPool::init;
int               SMemoryPool::pageSize = 0;
int               SMemoryPool::maxFreeCount = 64;
#if defined(Q_OS_LINUX)
  int             SMemoryPool::zeroDev = 0;
#endif

/*! Returns the size of the specified pool.
 */
size_t SMemoryPool::poolSize(Pool pool)
{
  QMutexLocker l(mutex());

  size_t result = 0;

  if (pool == Pool_Free)
  {
    foreach (const Block &block, freePool())
      result += block.size;
  }
  else if (pool == Pool_Alloc)
  {
    foreach (const Block &block, allocPool())
      result += block.size;
  }

  return result;
}

/*! Aligns the specified size to a multiple that can be allocated by the pool.
 */
size_t SMemoryPool::align(size_t size)
{
  Q_ASSERT(pageSize);

  return ((size + (pageSize - 1)) / pageSize) * pageSize;
}

/*! Returns a buffer of the specified size, either from the pool or newly
    allocated. The specified size needs to be aligned using align().

    \note Memory is not reset to zero.
 */
void * SMemoryPool::alloc(size_t size)
{
  size = align(size);
  if (size > 0)
  {
    QMutexLocker l(mutex());

    QMultiMap<size_t, Block>::Iterator i = freePool().lowerBound(size);
    if ((i != freePool().end()) && (i.key() <= (size + (size / 2))))
    {
      void * const result = i->addr;

      for (QList<size_t>::Iterator j=freeQueue().begin(); j!=freeQueue().end(); j++)
      if (*j == i.key())
      {
        freeQueue().erase(j);
        break;
      }

      allocPool().insert(result, *i);
      freePool().erase(i);

      Q_ASSERT(freePool().count() == freeQueue().count());
      return result;
    }
    else // Allocate new
    {
      void * const result = allocPages(size);
      allocPool().insert(result, Block(result, size));
      return result;
    }
  }

  return NULL;
}

/*! Returns the specified buffer back into the pool. All memory in the pool is
    freed if this was the last allocated buffer.
 */
void SMemoryPool::free(void *ptr)
{
  if (ptr)
  {
    QMutexLocker l(mutex());

    QHash<void *, Block>::Iterator i = allocPool().find(ptr);
    if (i != allocPool().end())
    {
      freePool().insert(i->size, *i);
      freeQueue().prepend(i->size);
      allocPool().erase(i);
    }
    else
      qFatal("SMemoryPool: Could not free buffer %p", ptr);

    // Flush all free buffers if there are no more buffers in use.
    if (allocPool().isEmpty())
    {
      size_t size = 0;
      foreach (const Block &block, freePool().values())
      {
        size += block.size;
        freePages(block.addr, block.size);
      }

      freePool().clear();
      freeQueue().clear();

      //qDebug() << "SMemoryPool: flushed free pool of" << (size / 1024) << "KiB";
    }

    while (freeQueue().count() > maxFreeCount)
    {
      QMultiMap<size_t, Block>::Iterator i = freePool().find(freeQueue().takeLast());
      if (i != freePool().end())
      {
        freePages(i->addr, i->size);
        freePool().erase(i);
      }
    }

    Q_ASSERT(freePool().count() == freeQueue().count());
  }
}

void * SMemoryPool::allocPages(size_t size)
{
  Q_ASSERT(pageSize);
  Q_ASSERT((size % pageSize) == 0);

#if defined(Q_OS_LINUX)
# ifndef USE_GUARD_PAGES
  void * const result =
      ::mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, zeroDev, 0);
# else
  quint8 * const result = reinterpret_cast<quint8 *>(
      ::mmap(0, size + (pageSize * 2), PROT_READ | PROT_WRITE, MAP_PRIVATE, zeroDev, 0));
# endif

  if (result != MAP_FAILED)
  {
# ifndef USE_GUARD_PAGES
    return result;
# else
    ::mprotect(result, pageSize, PROT_NONE);
    ::mprotect(result + pageSize + size, pageSize, PROT_NONE);
    return result + pageSize;
# endif
  }
#elif defined(Q_OS_WIN)
# ifndef USE_GUARD_PAGES
  void * const result =
      ::VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
# else
  quint8 * const result = reinterpret_cast<quint8 *>(
      ::VirtualAlloc(NULL, size + (pageSize * 2), MEM_RESERVE, PAGE_READWRITE));
# endif

  if (result != NULL)
  {
# ifndef USE_GUARD_PAGES
    return result;
# else
    ::VirtualAlloc(result + pageSize, size, MEM_COMMIT, PAGE_READWRITE);
    return result + pageSize;
# endif
  }
#else
  quint8 * const buffer = new quint8[size + sizeof(quint8 *) + pageSize];
  quint8 * const result = (quint8 *)(quintptr(buffer + sizeof(quint8 *) + pageSize) & ~quintptr(pageSize - 1));
  reinterpret_cast<quint8 **>(result)[-1] = buffer;

  return result;
#endif

  qFatal("SMemoryPool: Failed to allocate a block of size %u", unsigned(size));
  return NULL;
}

void SMemoryPool::freePages(void *addr, size_t size)
{
  Q_ASSERT(pageSize);
  Q_ASSERT((size % pageSize) == 0);

#if defined(Q_OS_LINUX)
# ifndef USE_GUARD_PAGES
  ::munmap(addr, size);
# else
  ::munmap(reinterpret_cast<quint8 *>(addr) - pageSize, size + (pageSize * 2));
# endif
#elif defined(Q_OS_WIN)
# ifndef USE_GUARD_PAGES
  ::VirtualFree(addr, 0, MEM_RELEASE);
# else
  ::VirtualFree(reinterpret_cast<quint8 *>(addr) - pageSize, 0, MEM_RELEASE);
# endif
#else
  delete [] reinterpret_cast<quint8 **>(addr)[-1];
#endif
}

QMutex * SMemoryPool::mutex(void)
{
  static QMutex m;

  return &m;
}

QMultiMap<size_t, SMemoryPool::Block> & SMemoryPool::freePool(void)
{
  static QMultiMap<size_t, Block> p;

  return p;
}

QList<size_t> & SMemoryPool::freeQueue(void)
{
  static QList<size_t> q;

  return q;
}

QHash<void *, SMemoryPool::Block> & SMemoryPool::allocPool(void)
{
  static QHash<void *, Block> p;

  return p;
}

} // End of namespace
